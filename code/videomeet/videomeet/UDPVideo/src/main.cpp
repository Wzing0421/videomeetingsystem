#include"../include/UDPSocket.h"
#include"../include/recv_json.h"
#include<vector>
using namespace std;
#define MSGBUFSIZE 50000     //BUF SIZE
string dstaddr="192.168.0.100";             //目的IP（中心站）
string MCASTADDR="225.0.0.1";           //视频接收IP地址（本地组播地址）
unsigned short MCASTPORT=5056;   //视频接收Port（本地组播端口）
unsigned short dstport =  30002; //目的端口（中心站）
unsigned short sgport=31001;
uint8_t mode;//=0x01;           //编码器ID
void config()
{
    ifstream fin("json.txt",ios::in);
    if(!fin.is_open())
	{
		cout<<"未成功"<<endl;
		return;
	}
	vector<string>tmp(6);
    string s;
    int index=0;
    while(getline(fin,s))
    {
       tmp[index]=s;
       ++index;
    }
   MCASTPORT=atoi(tmp[0].c_str());
   cout<<MCASTPORT<<endl;
   MCASTADDR=tmp[1];
   cout<<MCASTADDR<<endl;
   dstaddr=tmp[2];
   cout<<dstaddr<<endl;
   dstport=atoi(tmp[3].c_str());
   cout<<dstport<<endl;
   unsigned short mode_ = atoi(tmp[4].c_str());
   mode=(uint8_t)(mode_);
   cout<<mode<<endl;
   fin.close();
}
int main(int argc, char *argv[])
{
	UDPSocket signal;//监听信号
   	if(signal.create(sgport)<0)
    {
        cout<<"sock error"<<endl;
        return 0;
    }
	char sign_buf[100];
	string localip_;
	if (signal.recvbuf(sign_buf, 100, localip_, sgport)< 0)
		cout << "signal is error" << endl;
    //strcpy(sign_buf,"m_open_");
	if (strcmp(sign_buf,"m_open_")==0)
	{
		signal.Close();
		read_config("../web/config.json", mode);
		UDPSocket  ssock;
		ssock.create();
		struct sockaddr_in addr;
		int sock, recvsize, addrlen;
		struct ip_mreq mreq;
		char * recvbuf = new char[MSGBUFSIZE];
		char * sendbuf = new char[MSGBUFSIZE];
		//uint8_t mode=0x01;
		u_int yes = 1; /*** MODIFICATION TO ORIGINAL */
		/* create what looks like an ordinary UDP socket */
		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			perror("socket");
			exit(1);
		}
		/**** MODIFICATION TO ORIGINAL */
		/* allow multiple sockets to use the same PORT number */
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
		{
			perror("Reusing ADDR failed");
			exit(1);
		}
		/*** END OF MODIFICATION TO ORIGINAL */
		/* set up destination address */
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
		addr.sin_port = htons(MCASTPORT);
		/* bind to receive address */
		if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
		{
			perror("bind");
			exit(1);
		}
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = inet_addr(MCASTADDR.c_str());
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
		{
			perror("setsockopt");
			exit(1);
		}
		/* now just enter a read-print loop */
		int count = 0;
		while (1)
		{
			//ssize_t recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
			addrlen = sizeof(addr);
			recvsize = recvfrom(sock, recvbuf, MSGBUFSIZE, 0, (struct sockaddr *) &addr, (socklen_t *)&addrlen);
			if (recvsize < 0)
			{
				perror("recvfrom");
				exit(1);
			}
			memcpy(sendbuf, &mode, 1);
			memcpy(sendbuf + 1, recvbuf, recvsize + 1);
			printf("the length is %d,the count is %d\n", recvsize, count);
			++count;
			int ret = ssock.sendbuf(sendbuf, recvsize + 1, dstaddr, dstport);
			// std::cout<<ret<<std::endl;
			// printf("the sendbuf length is %d\n",ret);
			memset(recvbuf, 0, MSGBUFSIZE);
			memset(sendbuf, 0, MSGBUFSIZE);
			//puts(msgbuf);
		}
		ssock.Close();
		delete[] recvbuf;
		delete[] sendbuf;
		return 0;
	}
	else
	{
        cout<<"close myvideo....."<<endl;
		return 0;
	}
}


