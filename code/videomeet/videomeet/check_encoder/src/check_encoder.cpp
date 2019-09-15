#include"../include/recv_json.h"
#include"../include/UDPSocket.h"
#include<stdio.h>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/types.h>
extern "C"
{
#include<libavformat/avformat.h>
#include<libavutil/avutil.h>
}
#include<iostream>
#include<unistd.h>
const char *cmd="./GPIO.sh 1 1_B2";
uint8_t mode;
unsigned short UDPVideo_port= 31001;
unsigned short UDPRecv_port = 31002;
unsigned short Signal_port  = 31003;
string IP="127.0.0.1";
#define OPEN "m_open_"
#define CLOSE "m_close_"
int gpioDrive(int value, string name){
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
void closepro(UDPSocket & signal_sock)
{
	signal_sock.sendbuf(CLOSE, sizeof(CLOSE), IP, Signal_port);
    signal_sock.sendbuf(CLOSE, sizeof(CLOSE), IP, UDPRecv_port);
    signal_sock.sendbuf(CLOSE, sizeof(CLOSE), IP, UDPVideo_port);
}
void openpro(UDPSocket & signal_sock)
{
    signal_sock.sendbuf(OPEN, sizeof(OPEN), IP, Signal_port);
    sleep(1);
    signal_sock.sendbuf(OPEN, sizeof(OPEN), IP, UDPRecv_port);
	signal_sock.sendbuf(OPEN, sizeof(OPEN), IP, UDPVideo_port);
}
int main()
{
    UDPSocket signal_sock;
    signal_sock.create(31000);
 if(!gpioDrive(1,"1_A7")){
        cout<<"decoder gpio wrong!"<<endl;
		closepro(signal_sock);
		return 0;
    }
	//±àÂëÆ÷Ì½²â
    string str="../web/config.json";
    fstream fin;
    fin.open(str);
    if(!fin)
    {
        cout<<"json is not exist,exit the main process "<<endl;
		closepro(signal_sock);
        return 0;
    }
    av_register_all();
    avformat_network_init();
    AVFormatContext *pFormatCtx;
    AVPacket pkt;
    pFormatCtx = avformat_alloc_context();
    //const char * rtsppath="rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov";
    read_config("../web/config.json",mode);
    string s;
    get_Encoder_IP("../web/id2ip_de.json",mode,s);
    string tmpstr = "rtsp://" + s;
    std::cout<<tmpstr<<std::endl;
    const char * rtsppath=tmpstr.c_str();
    if(avformat_open_input(&pFormatCtx,rtsppath,0,0)<0)
    {
        std::cout<<"the rtsp cant open"<<std::endl;
		closepro(signal_sock);
        return 0;
    }
    if(av_read_frame(pFormatCtx,&pkt)<0){
        cout<<"encoder is not working,exit the main process"<<endl;
		closepro(signal_sock);
        return 0;
    }
    else
    {
        cout<<"encoder is working "<<endl;
		//openpro();
        if(!gpioDrive(1,"1_B2")){
            cout<<"encoder gpio wrong!"<<endl;
        }
        //int res=system("/home/")
		openpro(signal_sock);
    }
    return 0;
}
