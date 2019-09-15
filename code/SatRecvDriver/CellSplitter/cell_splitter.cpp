#include<utils.h>
#include<udp_socket.h>
#include<tcp_socket.h>
#include<logger.h>
#include<common_defs.h>
#include<fifo_queue.h>
#include<parameter.h>
#include"cell_splitter.h"
#include"monitor_interface.h"
#ifdef CONFIG_FILE
#undef CONFIG_FILE
#endif
#define CONFIG_FILE "../RecvDriver/config.json"
using namespace std;

extern Logger g_logger;

CellSplitter::CellSplitter(){
    monitor_interface_ = new MonitorInterface(this);
    recv_socket_ = new UdpSocket();
    send_socket_ = new UdpSocket();
    fifo_queue_ = new FifoQueue();
    work_over_ = false;
}

CellSplitter::~CellSplitter(){
    SafeDelete(monitor_interface_);
    SafeDelete(recv_socket_);
    SafeDelete(send_socket_);

    SafeDelete(fifo_queue_);
}

bool CellSplitter::Init(int argc, char* argv[]){
	Parameter parameter;
	parameter.LoadConfig(CONFIG_FILE);

	recv_cell_port_ = parameter.data_port.cell_recv_port;

    //BuildSockAddr("127.0.0.1", parameter.data_port.msg_cell_port, &message_addr_);
    BuildSockAddr("127.0.0.1", parameter.data_port.video_cell_port, &video_addr_);
    //BuildSockAddr("127.0.0.1", parameter.data_port.file_cell_port, &file_addr_);

   // BuildSockAddr("127.0.0.1", 8890, &file_config_addr_);
    //create socket
    if (!recv_socket_->Create()){
        return false;
    }

    if (! recv_socket_->Bind("0.0.0.0", recv_cell_port_)){
        return false;
    }

    if (! recv_socket_->SetRecvBufSize(10*1024*1024)){
		return false;
    }

    if (! send_socket_->Create()){
	return false;
    }
    //return monitor_interface_->ConnectToServer(parameter.module_port.cell_splitter_port, "127.0.0.1", parameter.module_port.monitor_server_port);
    return true;
}

void CellSplitter::MonitorLoop(){
    //首先注册到 RecvDriver
    const int kBufLen = 50000, kHeadLen = 4;
    char recv_buffer[kBufLen];
    char* recv_pos = recv_buffer;
    int recved_len = 0;

    //注册包
    if(!monitor_interface_->Register()){
        printf("Register to RecvDriver failed!\n");
        return;
    };
    printf("Register to RecvDriver successfully!\n");

    while(true){
        int complete_packet_num = monitor_interface_->RecvPacket(recv_buffer, &recved_len);
        if(complete_packet_num < 0){ //the connection is closed by peer-side, reconstruct the report socket and connect to server
            bool connected = false;
            while(!connected){
                printf("lost connection to RecvDriver....\n");
                connected = monitor_interface_->ReconnectToServer();
                if(connected)
                  printf("reconnect to RecvDriver successfully ..\n");
            } 
        }
        for(int i = 0; i < complete_packet_num; i ++){
            uint32_t packet_len = *(uint32_t*)recv_pos;
            recv_pos += kHeadLen;
            monitor_interface_->ProcessPacket(recv_pos, packet_len);
            recv_pos += packet_len;
        }
    }
}

void CellSplitter::ReceiveLoop(){
    const int kMaxBufLen = 1000;
    char recv_buffer[kMaxBufLen];
	int total_lost = 0;

    while(! work_over_){

        int recved_len = recv_socket_->Recv(recv_buffer, kMaxBufLen);

		if (recved_len > 0){
			fifo_queue_->PushPacket(recv_buffer, recved_len);	
		}	
    } 	 
}


//split cell by cell_type in header, and send them to corresponding udp port
void CellSplitter::ProcessLoop(){
    work_over_ = false;
    uint8_t service_type;
    uint16_t send_len;
    const int kMaxBufLen = 1000;
    char send_buffer[kMaxBufLen];
    uint32_t gSeq = 0;
    while(! work_over_){
        fifo_queue_->PopPacket(send_buffer, &send_len);
		service_type = *(uint8_t*)(send_buffer + SEC_HEAD_LEN);
		/*
        uint32_t seq_num = *(uint32_t*)(send_buffer + 18);
        if (seq_num != gSeq + 1){
            printf("lost num = %d\n", seq_num - gSeq);
        }    
        gSeq = seq_num;
		*/
		switch(service_type){
            /*case FILE_SERVICE:
	       	    send_socket_->SendTo(send_buffer, send_len, file_addr_);
				//send_socket_->SendTo(send_buffer, send_len, "127.0.0.1", 60003);
	       		 break;*/
		    case VIDEO_SERVICE:
	        	 send_socket_->SendTo(send_buffer, send_len, video_addr_);
	        	 break;
	    	/*case MESSAGE_SERVICE:
	        	 send_socket_->SendTo(send_buffer, send_len, message_addr_);
	         	break;*/
		}	    
    }
}

