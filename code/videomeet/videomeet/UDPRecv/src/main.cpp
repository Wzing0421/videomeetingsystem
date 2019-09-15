#include"../include/UDPSocket.h"
#include"../include/recv_json.h"
#include<vector>
using namespace std;
unsigned short sport=10002;// Video Port
unsigned short rport=30002; // Recv Port
unsigned short sgport = 31002;// signal Port
uint8_t channel;// Channel_ID = 0x01;
map<uint8_t, string>mapc_p;


int main()
{
	UDPSocket signal;//signal socket
	if(signal.create(sgport)<0)
    {
        cout<<"sock error"<<endl;
        return 0;
    }
	char sign_buf[100];
	string localip_;
    if (signal.recvbuf(sign_buf, 100, localip_, sgport) < 0)
		cout << "signal is error" << endl;
	//signal.Close();
    //strcpy(sign_buf,"m_open_1");
	if (strcmp(sign_buf,"m_open_")==0)
	{
		read_config("../web/ch2ip.json", mapc_p);
		UDPSocket rsock;
		UDPSocket ssock;
		rsock.create(rport);
		ssock.create();
		char *buf = new char[50000];
		int recvsize;
		int count = 0;
		unsigned short port;
		string localip;
		while (1)
		{
			recvsize = rsock.recvbuf(buf, 50000, localip, port);
			cout << port << endl;
			if (port != 9212)
			{
				++count;
				printf("the buf is %d,the length is %d\n", count, recvsize);
				uint8_t tmp = (uint8_t)buf[0];
				//printf("the dest is %x",tmp);
				//if(tmp==channel)
				//{
				int send = ssock.sendbuf(buf + 1, recvsize - 1, mapc_p[tmp], sport);
				printf("the sendbuf is %d\n", send);
				memset(buf, 0, 50000);
				//}
			}
			else
			{
				sleep(1);
				read_config("../web/ch2ip.json", mapc_p);
			}
		}
		delete[] buf;
		rsock.Close();
		ssock.Close();
		return 0;
	}
	else
	{
        cout<<"close myrecv....."<<endl;
		return 0;
	}
}

