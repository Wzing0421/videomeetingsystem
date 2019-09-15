#include<logger.h>
#include<parameter.h>
#include<stdio.h>
#include<string.h>
#include<common_defs.h>
#include"agent.h"
#include"monitor_server.h"
#include"epoll_wrapper.h"
using namespace std;
#define EtcStartPos 15000
extern Parameter g_parameter;
extern Logger g_logger;


MonitorServer::MonitorServer(){
    request_seq_ = 0;
}

MonitorServer::~MonitorServer(){
    //delete all pointers in map
    while(! observed_client_.empty()){
        std::map<int, CommonSocket*>::iterator it = observed_client_.begin();
        delete it->second;
        observed_client_.erase(it);
    }
}


bool MonitorServer::StartServer(){
    
    status_sockaddr_.sin_family = AF_INET;
    status_sockaddr_.sin_addr.s_addr = inet_addr(g_parameter.app_client.client_ip);
    status_sockaddr_.sin_port = htons(g_parameter.app_client.client_port);

	GetCpu();
	if (!server_socket_.Create() || !app_socket_.Create()){  //modified by yjb
		return false;
	}
	if (!server_socket_.SetBlocking(false)){
		return false;
	}
	if (!server_socket_.Bind("0.0.0.0", g_parameter.module_port.monitor_server_port)){
		return false;
	}
	if (!app_socket_.Bind("0.0.0.0", g_parameter.module_port.monitor_app_port)){
		return false;
	}
	if (!server_socket_.Listen(10)){
		return false;
	}
	//in linux environment, we use epoll; else use select
#ifdef LINUX
	epoll_.AddEvent(server_socket_.socket_fd(), EPOLLIN);
	epoll_.AddEvent(app_socket_.socket_fd(), EPOLLIN);
#endif

	observed_client_.insert(pair<int, CommonSocket*>(server_socket_.socket_fd(), &server_socket_));
	observed_client_.insert(pair<int, CommonSocket*>(app_socket_.socket_fd(), &app_socket_));
	g_logger.LogToFile("monitor server start successfully!");
	return true;
}

void MonitorServer::WorkLoop(){
	//use epoll(in linux) or select(in windows) to handle all sockets descriptions
	//handle the tcp between monitor_server and other modules, udp between monitor_server and app client
#ifdef LINUX
	while(true){
		int event_num = epoll_.Wait();
		const struct epoll_event* ready_events = epoll_.ReadyEvents();
		//handle events
		for (int i = 0; i < event_num; i++){
			HandleEvent(ready_events[i].data.fd);
		}
	}
#else
	struct timeval tv;
	FD_SET read_set, write_set;
	while (true){
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		for (map<int, CommonSocket*>::iterator it = observed_client_.begin(); it != observed_client_.end(); ++it){
			FD_SET(it->first, &read_set);
		}
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		int ret = select(0, &read_set, &write_set, NULL, &tv);
		if (ret == SOCKET_ERROR){
			continue;
		}
		map<int, CommonSocket*>::iterator it = observed_client_.begin();
		while (it != observed_client_.end()){
			//in case that, old_it is deleted in the HandleEvent function
			map<int, CommonSocket*>::iterator old_it = it++;
			if (FD_ISSET(old_it->first, &read_set)){
				HandleEvent(old_it->first);
			}
		}
	}
#endif
}


void MonitorServer::AddObservedClient(int socket_fd){	
    TcpSocket* client = new TcpSocket;
    client->Create(socket_fd);//build socket by using socket_fd
	client->SetBlocking(false); //set socket in no block mode
#ifdef LINUX
    if (! epoll_.AddEvent(socket_fd, EPOLLIN)){ //add socket fd to epoll events
        g_logger.LogToFile("epoll add event failed");
    }
#endif
    observed_client_.insert(std::pair<int, CommonSocket*>(socket_fd, client));
}

void MonitorServer::DelObservedClient(int socket_fd){
    std::map<int, CommonSocket*>::iterator it = observed_client_.find(socket_fd);
    if (it == observed_client_.end()){
        return;
    }
    delete it->second; //delete pointer
    observed_client_.erase(it);
#ifdef LINUX
    if (! epoll_.DelEvent(socket_fd, EPOLLIN)){ //add socket fd to epoll events
        g_logger.LogToFile("epoll del event failed");
    }
#endif
}

void MonitorServer::HandleEvent(int socket_fd){
	if (socket_fd == server_socket_.socket_fd()){ //listen fd event
        //handle connection event
        int conn_fd;
        struct sockaddr_in client_addr;
        if (! server_socket_.Accept(&conn_fd, &client_addr)){
            g_logger.LogToFile("monitor server accept connection failed");
            return;
        }else{
			cout << "monitor server accept a new connection" << endl;
            g_logger.LogToFile("monitor server accept a new connection");
        }
        //add a observed client to map
        AddObservedClient(conn_fd);
        return;  //handle a  connection event and return
    }else{
        const int kBufSize = 50000, kHeadLen = 4;
        int recved_len;
        char recv_buffer[kBufSize];
        //app socket event
        if (socket_fd == app_socket_.socket_fd()){
            uint32_t from_ip;
            uint16_t from_port;
            recved_len = app_socket_.RecvFrom(recv_buffer, kBufSize, &from_ip, &from_port);
            web_sockaddr_.sin_family = AF_INET;
            web_sockaddr_.sin_addr.s_addr = htonl(from_ip);
            web_sockaddr_.sin_port = htons(from_port);
            ProcessPacketFromAppClient(recved_len, recv_buffer);
        }
        //other modules event
        else{
            //get the observed client that event occurs
            TcpSocket* client = dynamic_cast<TcpSocket*>(observed_client_[socket_fd]);
            int complete_packet_num = client->RecvPacket(recv_buffer, &recved_len);
            if (complete_packet_num < 0){ //receive failed or not receive 1 or more complete packet
                cout << "connection closed!" << endl;
                DelObservedClient(socket_fd);
            }else{
                char* read_pos = recv_buffer;
                for (int i = 0; i < complete_packet_num; i++){//get all complete packets in buffer
                    int packet_len = *(int*)read_pos;
                    read_pos += kHeadLen;
                    ProcessPacketFromOtherModule(client, packet_len, read_pos);
                    read_pos += packet_len;
                }
            }
        }
    }
}

//处理来自后端28所的信令
void MonitorServer::ProcessPacketFromAppClient(int len, char* recv_data){
	//print the signalling packet between monitor server and app client
	for (int i = 0; i < len; i++)
	{
		printf("%u ", recv_data[i]);
	}
	printf("\n");

	char * read_pos = recv_data;
	char packetType = *read_pos;
	read_pos += sizeof(packetType);
	if (packetType != 0x40)
	{
		cout << "error: packet type error" << endl;
		return;
	}
	int nPacketLength = ntohl(*(int *)read_pos);
	read_pos += sizeof(nPacketLength);

	char controlType = *(char *)read_pos;
	cout << "the control type is " << (int)controlType << endl;
	read_pos += sizeof(char);
	switch (controlType)
	{
	case HEARTBEAT:
		SendCpuAndMem();
		break;
	case STATUS:
    {
		int nSequnceNumber = ntohl(*(int*)read_pos);
		read_pos += sizeof(int);
		int nID = ntohl(*(int *)read_pos);
		if (-1 == nID)
			SendTransactionStatus(nSequnceNumber);
		else
			SendTransactionStatus(nSequnceNumber, nID);
    }
	    break;
	case SETIPANDPORT:
    {
		uint32_t newAddress = *((uint32_t*)read_pos);
		read_pos += 4;
		uint16_t newPort = ntohs(*((uint16_t*)read_pos));
		in_addr newAddr;
		newAddr.s_addr = newAddress;
		SetAddressAndPort(newAddr, newPort);
   }
		break;
	default:
		break;
	}
}

//处理来自其他模块的信令
void MonitorServer::ProcessPacketFromOtherModule(TcpSocket* client, int len, char* recv_data){
    char* recv_pos = recv_data;
    uint8_t src_role = *(uint8_t*)recv_pos;
    recv_pos ++;
    uint32_t seq = *(uint32_t*)recv_pos;
    recv_pos += sizeof(uint32_t);
    recv_pos ++; //请求or回复
    uint8_t signal_type = *(uint8_t*)recv_pos;
    recv_pos ++;
    switch(signal_type){
        case REGISTER:
            OnReceiveRegister(client, recv_data, len);
            break;
        case UNREGISTER:
            OnReceiveUnregister(client, recv_data, len);
            break;
        case REPORT_STATE:
            OnReceiveState(client, recv_data, len);
            break;
    }
}


//收到其他模块的注册信令
void MonitorServer::OnReceiveRegister(TcpSocket* client, char* recv_data, int len)
{
    char send_buf[100];
    memcpy(send_buf, recv_data, len);
    char* send_pos = send_buf;
    uint8_t src_role = *send_pos;
    switch(src_role)
    {
        case CELL_SPLITTER:
		printf("Cell Splitter register to monitor server.\n");
            break;
        case MSG_RECEIVER:
		printf("Msg Receiver register to monitor server.\n");
            break;
        case VIDEO_RECEIVER:
		printf("Video Receiver register to monitor server.\n");
            break;
        case FILE_RECEIVER:
		printf("File Receiver register to monitor server.\n");
            break;
        default :
            break;
    }
    //回复给模块，源角色改为 RECV_DRIVER
    *send_pos++ = RECV_DRIVER;
    send_pos += sizeof(uint32_t); //序列号保持不变
    *send_pos++ = 0x01; //表示回复包
    send_pos ++; //信令类型不变
    *(uint32_t*)send_pos = htonl(1); //数据长度为1, 主机字节序
    send_pos += sizeof(uint32_t);
    *send_pos++ = 0x00; //表示成功
    
    client->SendPacket(send_buf, send_pos - send_buf);
}

//收到其他模块的注销信令
void MonitorServer::OnReceiveUnregister(TcpSocket* client, char* recv_data, int len)
{
}

//接收到来自其他模块的业务状态变化信令
bool MonitorServer::OnReceiveState(TcpSocket* client, char * recv_data, int len)
{
    char* pos = recv_data;
    uint8_t src_role = *pos++;
    pos += sizeof(uint32_t);
    pos ++; //请求or回复
    uint8_t signal_type = *pos ++;
    assert(signal_type == REPORT_STATE);
    uint32_t data_len = ntohl(*(uint32_t*)pos);
    pos += sizeof(uint32_t);
    uint32_t transaction_id = ntohl(*(uint32_t*)pos);
    pos += sizeof(uint32_t);
    uint8_t transaction_type = *pos++;
    uint8_t transaction_state = *pos++;
    uint8_t transaction_progress = *pos++;
    if(transaction_state == NEW_TRANSACTION){
        if(tid_to_transinfo_.find(transaction_id) == tid_to_transinfo_.end()){
            TransactionInfo transinfo;
            transinfo.id = transaction_id;
            transinfo.type = transaction_type;
            transinfo.state = transaction_state;
            transinfo.progress = 0;
            uint8_t file_path_len = *(uint8_t*)pos ++;
            transinfo.nfs_path = std::string(pos, file_path_len);
            tid_to_transinfo_[transaction_id] = transinfo;
        }else
          return false;
    }
    if(tid_to_transinfo_.find(transaction_id) == tid_to_transinfo_.end()){
        return false;
    }

    //更新业务状态和进度
    tid_to_transinfo_[transaction_id].state = transaction_state;
    tid_to_transinfo_[transaction_id].progress = transaction_progress;
    uint8_t file_path_len = *(uint8_t*)pos ++;
    tid_to_transinfo_[transaction_id].nfs_path = std::string(pos, file_path_len);

    //之前和28所后端模块设计的，没有 DOING_TRANSACTION 这个信令。保持兼容...
    if(transaction_state != DOING_TRANSACTION){
        SendNotificationToAppClient(transaction_id);
    }

    //要求，文件接收完成之后，必须发送 FINISH_TRANSACTION 信令
    if(transaction_state == CANCEL_TRANSACTION || transaction_state == FINISH_TRANSACTION)
        tid_to_transinfo_.erase(transaction_id);
    return true;
}

//主动向后端28所模块通知
void MonitorServer::SendNotificationToAppClient(uint32_t transaction_id)
{
    char cPacketType = 0x0c;
    char cControl = 0x20;
    uint32_t nTransactionId = transaction_id;
    TransactionInfo transinfo = tid_to_transinfo_[transaction_id];
    char cTransactionType = 0x00;//fixed to file transinfo.type;
    char cStateType = transinfo.state;
    int nPacketLength = sizeof(cControl)+
                        sizeof(nTransactionId)+
                        sizeof(cTransactionType)+
                        sizeof(cStateType)+
                        sizeof(uint16_t)+
                        FILE_PATH_LENGTH;

    char send_buf[1000];
    memset(send_buf, 0, sizeof(send_buf));
    char* send_pos = send_buf;
    *send_pos++ = cPacketType;

    *(uint32_t*)send_pos = htonl(nPacketLength);
    send_pos += sizeof(uint32_t);

    *send_pos++ = cControl;
    *(uint32_t*)send_pos = htonl(nTransactionId);
    send_pos += sizeof(uint32_t);
    *send_pos ++ = cTransactionType;
    *send_pos ++ = cStateType;
    *(uint16_t*)send_pos = 0; //udp 端口，没什么用
    send_pos += sizeof(uint16_t);
    
    memcpy(send_pos, transinfo.nfs_path.c_str(), transinfo.nfs_path.length());
    send_pos += FILE_PATH_LENGTH;
    printf("send transaction status to app client, transid = %u, status = %d\nprogress = %d, file name = %s\n", transinfo.id, transinfo.state, 
                transinfo.progress, transinfo.nfs_path.c_str());
    /*
    printf("transaction nfs path = %s\n", transinfo.nfs_path.c_str());
    for(int i = 0; i < (send_pos - send_buf); i ++){
        printf("%02X ", *(uint8_t*)(send_buf + i));
    } 
    printf("\n\n\n");
    */
	app_socket_.SendTo(send_buf, send_pos - send_buf, status_sockaddr_);
}


//向后端28所模块回复状态查询信令
bool MonitorServer::SendTransactionStatus(uint32_t seq, uint32_t transaction_id)
{
    char cPacketType = 0x0c;
    char cControlType = 0x11;
    char cStateType = 0x00;
    uint32_t nSequenceNumberRe = seq;
    int nIdRe = transaction_id;
    if(tid_to_transinfo_.find(transaction_id) == tid_to_transinfo_.end())
    {
        cout<<"can not find the transaction"<<endl;
        return false;
    }
    TransactionInfo transinfo = tid_to_transinfo_[transaction_id]; 
    char cTransactionType = 0x00;// fixed to file(file in CETC28 is 0x00) transinfo.type;
    char cProgress = transinfo.progress;

    uint32_t nPacketLength = sizeof(cControlType) + 
                             sizeof(nSequenceNumberRe) + 
                             sizeof(nIdRe) + 
                             sizeof(cTransactionType) + 
                             sizeof(cStateType)	+ 
                             sizeof(cProgress) + 
                             sizeof(uint16_t) + //端口，不知道干啥用
                             FILE_PATH_LENGTH;

    char send_buf[1000];
    memset(send_buf, 0, sizeof(send_buf));
    char* send_pos = send_buf;
    *send_pos ++ = cPacketType;
    *(uint32_t*)send_pos = htonl(nPacketLength);
    send_pos += sizeof(uint32_t);
    *send_pos ++ = cControlType;
    *(uint32_t*)send_pos = htonl(nSequenceNumberRe);
    send_pos += sizeof(uint32_t);
    *(uint32_t*)send_pos = htonl(nIdRe);
    send_pos += sizeof(uint32_t);
    *send_pos ++ = cTransactionType;
    *send_pos ++ = cStateType;
    *send_pos ++ = cProgress;
    *(uint16_t*)send_pos = 0; //port，设为0，没什么用
    send_pos += sizeof(uint16_t);
    memcpy(send_pos, transinfo.nfs_path.c_str(), transinfo.nfs_path.length());
    send_pos += FILE_PATH_LENGTH;
    
	app_socket_.SendTo(send_buf, send_pos - send_buf, status_sockaddr_);    
    return true;
}

//向后端28所模块回复状态查询信令
bool MonitorServer::SendTransactionStatus(uint32_t seq)
{
    map<uint32_t,TransactionInfo>::iterator iter;
    for(iter = tid_to_transinfo_.begin();iter!=tid_to_transinfo_.end();iter++)
    {
        SendTransactionStatus(seq, iter->first);
    }
    return true;
}

//向后端28所模块发送cpu和内存使用状况信令
bool MonitorServer::SendCpuAndMem()
{
    int CpuUsage = GetCpu();
    int MemUsage = GetMemory();
    printf("cpu usage is %u and mem is %u\n",CpuUsage,MemUsage);
    if(-1 == CpuUsage || -1 ==MemUsage)
    {
        cout<<"error when trying to get cpu and mem usage"<<endl;
        return false;
    }
    char send_buf[20];
    char *readPos = send_buf;
    memset(readPos,0x0c,sizeof(char));
    readPos+=sizeof(char);
    int nPacketLength = 3;
    int sentLength = htonl(nPacketLength);
    memcpy(readPos,&sentLength,sizeof(sentLength));
    readPos += sizeof(sentLength);
    unsigned char cCpuUsage = (unsigned char)(CpuUsage & 0x000000FF);
    unsigned char cMemUsage = (unsigned char)(MemUsage & 0x000000FF);
    memset(readPos,0x01,sizeof(char));
    readPos += sizeof(char);
    memcpy(readPos,&cCpuUsage,sizeof(unsigned char));
    readPos+=sizeof(unsigned char);
    memcpy(readPos,&cMemUsage,sizeof(unsigned char));
    readPos += sizeof(unsigned char);

	int result = app_socket_.SendTo(send_buf, readPos - send_buf, web_sockaddr_);
    if(result != readPos - send_buf)
    {
        cout<<"error: can not send CPU and mem status to application"<<endl;
        return false;
    }
    return true;
}

//设置本机的IP和端口
void MonitorServer::SetAddressAndPort(in_addr  address, int mask_len)
{   

    /*the following code: write teh new IP to etc/newwork/interfaces and save it
    */
#ifdef LINUX
    std::string netmask = "";
    //发给28所的回复信息
    char res[20];
	char *readpos = res;
	*readpos = 0X0c;
	readpos += sizeof(unsigned char);
	*((uint32_t *)readpos) = htonl(2);
	readpos += sizeof(uint32_t);
	*readpos = 0x31;
	readpos += sizeof(unsigned char);
	*readpos = 0;
    if(mask_len < 0 || mask_len > 32){
        printf("invalid mask len, mask len = %d\n", mask_len);
        //仍然返回成功, 我草。。
	   // *readpos = -1;
        //get netmask of ip address in /etc/network/interfaces
        netmask = GetOriginalNetmask();
        cout << "use original netmask, netmask = " << netmask << endl;
    }else{
        struct in_addr mask_addr;
        uint32_t mask_uint = ~((1 << 32 - mask_len) - 1);
        mask_addr.s_addr = htonl(mask_uint);
        netmask = std::string(inet_ntoa(mask_addr));
        cout << "new netmask = " << netmask << endl;
    }
	readpos += sizeof(unsigned char);


    //修改ip地址 和子网掩码
    const char * filename = "/etc/network/interfaces";
    const char * filename1 = "/etc/network/";
    char * ipaddr = inet_ntoa(address);
    FILE *fp = fopen(filename, "r");
    char *buf = new char[EtcStartPos];
    memset(buf, 0, sizeof(char) * EtcStartPos);
    int bytes = fread(buf,1,EtcStartPos,fp);
    fclose(fp);
    string filecont(buf);
    cout<<"the origin content:"<< "**********"<<endl;
    cout<<filecont<<endl;
    cout<<"************"<<endl;
    size_t found = filecont.rfind("address");
    if(found != std::string::npos){
      found += 7; // the length of string "address"
      while(found != filecont.length() && (filecont[found] == ' ' || filecont[found] == '\n'))
        found++;
      size_t temp = found;
      while(temp != filecont.length() && filecont[temp] != '\n' && filecont[temp] != ' ')
        temp++;
      size_t add_len = temp - found;
      filecont.replace(found, add_len, ipaddr, strlen(ipaddr));

      found = filecont.rfind("netmask");
      if(found != std::string::npos){
            found += 7; //size of "netmask"
            while(found != filecont.length() && (filecont[found] == ' ' || filecont[found] == '\n'))
                found++;
            temp = found;
            while(temp != filecont.length() && filecont[temp] != '\n' && filecont[temp] != ' ')
                temp++;

            filecont.replace(found, temp - found, netmask.c_str(), netmask.length());
      }
      cout<<"the new content:"<<"&&&&&&&&&&&"<<endl;
      cout<< filecont.c_str() <<endl;
      cout<<"&&&&&&&&&&&&"<<endl;
      fp = fopen(filename, "w+");
      fwrite(filecont.c_str(), 1, filecont.length(), fp);
      fclose(fp);
    }
    delete buf;
    /*the following code: change the ip immediately by calling ifconfig
    */
    string str1 = "ifconfig eth0:1 ";
    char str[100];
    sprintf(str, "ifconfig eth0:1 %s netmask %s", ipaddr, netmask.c_str());
    RunCommand(str);
    char sys_cmd[200];
    strcpy(sys_cmd, "sed '/ifconfig eth0:1/d' /etc/network/interfaces.exe > tmp");
    RunCommand(sys_cmd);
    sprintf(sys_cmd, "echo %s >> tmp", str);
    RunCommand(sys_cmd);
    strcpy(sys_cmd, "cat tmp > /etc/network/interfaces.exe");
    RunCommand(sys_cmd);

    app_socket_.SendTo(res, readpos - res, web_sockaddr_);
#endif
}

