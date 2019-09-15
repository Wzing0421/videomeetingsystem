#include"../include/UDPSocket.h"
#include"../include/recv_json.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>

#define serverPort 30001
#define serverIP "99.1.1.101"
unsigned short sgport=31003;
uint8_t mode;
int gpioDrive(int value, string name){//通过设置value和name来确定个GPIO模块
    pid_t status;
    string cmd = "./gpioModule.sh ";
    cmd = cmd + to_string(value);
    cmd+= " ";
    cmd+= name;
    status = system(cmd.c_str());
    if(status ==-1){
        std::cout<<"system error"<<std::endl;
        return 0;
    }
    else{
        if(WIFEXITED(status)){
            if(0==WEXITSTATUS(status)){
                std::cout<<"shell succeeds"<<std::endl;
                return 1;
            }
            else{
               std::cout<<"shell fails, script exit code: "<<WEXITSTATUS(status)<<std::endl;
               return 0;
           }
        }
        else{
          std::cout<<"exit status ="<<WEXITSTATUS(status);
          return 0;
        }
    }

}
void ChDetc(){

    UDPSocket ChDetsock;
    ChDetsock.create(30001);
    char buf[100];
    char sendbuf;

    memcpy(&sendbuf, &mode,1);
    sendbuf = sendbuf | 0x80;
    string destIP;
    unsigned short destport;

    struct timeval timeout;
    fd_set fds;

    while(1){
        timeout.tv_usec = 0;
        timeout.tv_sec = 5;

	FD_ZERO(&fds);
	FD_SET(ChDetsock.sock, &fds);

	int maxfdp = ChDetsock.sock + 1;

        /*
         30002 should be changed to 30001, now i only test the code in the local environment,
         so I should define two different port.
         In the actual server, it should be changed to 30001
        */
	//ChDetsock.sendbuf(&sendbuf,sizeof(sendbuf),"99.1.1.101",30001);//change to 30001
        int len = ChDetsock.sendbuf(&sendbuf,sizeof(sendbuf),serverIP,serverPort);//change to 30001
	std::cout<<len<<std::endl;
        //const char *cmd1 = "./gpioModule.sh 0 4_D5";
        //const char *cmd2 = "./gpioModule.sh 1 4_D5";
        switch(select(maxfdp, &fds, NULL, NULL, &timeout)){
            case -1:
		perror("error");
		exit(-1);
		break;
	    case 0:
		printf("TimeOut! The channel is broken!\n");
                if(!gpioDrive(0,"4_D5")){
                    cout<<"channel gpio wrong!"<<endl;
                }
                continue;
		break;
	    default:
		if(FD_ISSET(ChDetsock.sock, &fds)){
		    //recvfrom(sockfd,buf,maxline,0,(struct sockaddr*)&server,&addr_len);
		    ChDetsock.recvbuf(buf,sizeof(buf),destIP,destport);
		    printf("receive from client: %s.\n", buf);
                    if(!gpioDrive(1,"4_D5")){
                        cout<<"channel gpio wrong!"<<endl;
                    }
		}
                break;
        }
        sleep(2);
    }

}

int main(){
    UDPSocket signal;//监听信号
	signal.create(sgport);
	char sign_buf[100];
	string localip_;
	if (signal.recvbuf(sign_buf, 100, localip_, sgport)< 0)
		cout << "signal is error" << endl;
    //strcpy(sign_buf,"m_open_");
    if (strcmp(sign_buf,"m_open_")==0){
        signal.Close();
        read_config("../web/config.json",mode);
        ChDetc();
        return 0;
    }
    else
    {
        signal.Close();
        cout<<"close myvideo....."<<endl;
	return 0;
    }

}
