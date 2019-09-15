#include"../include/UDPSocket.h"
#include<thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include<fstream>
#include<vector>
#include <map>
#include<string>
#include <pthread.h>

using namespace std;

unsigned short sport;
unsigned short rport;
unsigned short localport;
string localip;
string destip;
uint8_t channel=0x01;
char c = 0xFF;

map <int, int> map1;
int Find_ID_Exist(int codeID){// to find if the codeID has joined the meeting
    map<int,int>::iterator it;
    int hasFound = 0;
    for(it = map1.begin(); it!= map1.end(); it++){
        if(it->first == codeID){
	    hasFound = 1;
    	    break;
	}
    }
    return hasFound;
}
void Insert_CodeID(int codeID){
/*
If the codeID in the map is bigger than the codeID, don't do anything.
If the codeID in the map is smaller than the codeID, for codeIDs in the map ,channel num++;
Find the biggest codeID that smaller than codeID, the new channel num of the codeID is his codeID++;
*/
    if(map1.empty()){
        map1.insert(make_pair(codeID,1));
        return;
    }
    map<int,int>::iterator it;
    int tmp = -1;
    int NewChannelNum=1;
    for(it = map1.begin(); it != map1.end(); it++){//Find the biggest codeID that smaller than codeID
        if(it->first < codeID){
            if(tmp < it->first){
                tmp = it->first;
		NewChannelNum = it->second +1;
            }
    	    continue;
        }
        else{
	    it->second++;
	}
    }
    map1.insert(make_pair(codeID,NewChannelNum));
}

int getChannelNum(int srcID){//If there isn't a srcID,then return 0; else return the channel number
	map<int, int> ::iterator it;
	bool hasFound = 0;
	for (it = map1.begin(); it != map1.end(); it++){
		if (it->first == srcID){
			hasFound = 1;
			break;
		}
	}
	if (hasFound) return it->second;
	else return 0;
}

void config()
{
	ifstream fin("json.txt", ios::in);
	if (!fin.is_open())
	{
		cout << "未成功" << endl;
		return;
	}
	vector<string>tmp(3);
	string s;
	int index = 0;
	while (getline(fin, s))
	{
		tmp[index] = s;
		++index;
	}
	rport = atoi(tmp[0].c_str());//信号端口号
	cout << rport << endl;
	sport = atoi(tmp[1].c_str());//视频数据转到本机拆解包端口号
	cout << sport << endl;
	destip = tmp[2];
	cout << destip << endl;//本地IP
}
void *ChaDet_ACK(void *arg){
    cout<<"Thread ack is on!"<<endl;
    UDPSocket rsock;
    rsock.create(rport);

    char buf[100];
    int recvsize;

    struct timeval timeout;
    fd_set fds;
    string clientip;
    unsigned short clientport;
    while(1)
    {

	timeout.tv_usec = 0;
        timeout.tv_sec = 100;

        FD_ZERO(&fds);
	FD_SET(rsock.sock, &fds);

	int maxfdp = rsock.sock + 1;

	switch(select(maxfdp, &fds, NULL, NULL, &timeout)){
            case -1:
		perror("error");
		exit(-1);
		break;
	    case 0:
		printf("TimeOut! The meeting ends!\n");
                if(!map1.empty()) map1.clear();
                continue;
		break;
	    default:
		if(FD_ISSET(rsock.sock, &fds)){

                    recvsize=rsock.recvbuf(buf,sizeof(buf),clientip,clientport);

                    //recv the buf and get the last 7 bits to know what codeID it is.
                    int ID = 0;
		    char c = buf[0];
		    for (int i = 6; i >= 0; i--){
	    	        int t = ((c >> i) & 1);
	    		ID = 2 * ID + t;
		    }
        	    //ACK to the sender
        	    char ack = 0xFF;
                    int send = rsock.sendbuf(&ack,sizeof(char),clientip,clientport);
            	    memset(buf,0,100);

        	    //if the codeID is not in the map, insert it in the map
        	    if(!Find_ID_Exist(ID)) Insert_CodeID(ID);

		}
                break;
        }

    }
    rsock.Close();

}

void *VideoTrans(void *arg){
    cout<<"Thread trans is on!"<<endl;
    UDPSocket Transock;
    Transock.create(30002);

    char *sendbuf=new char[50006];
    char *recvbuf=new char[50000];
    int recvsize;
    string recvIP;
    unsigned short recvport;
    while(1){

        recvsize = Transock.recvbuf(recvbuf,50000,recvIP,recvport);

        //get the codeID from the 1st bit
        uint8_t tmp=(uint8_t)recvbuf[0];
	int isSignal=((tmp >> 7) & 1);

        int codeID=0;
	if(!isSignal){//That means this packet is video
	    for (int i = 6; i >=0; i--){//last 6 bit of recvbuf[0], the codeID
		 int t = ((recvbuf[0] >> i) & 1);
		 codeID =2*codeID+ t;
	    }
 	}

        //change the codeID to the channel number
	int channelNumber = getChannelNum(codeID);
        char num = (char)channelNumber;
	recvbuf[0] = num;
        /* Here the destine IP and the destine port should be changed accoding to the json*/
        int sendlen = Transock.sendbuf(recvbuf,recvsize,destip,sport);
        cout<<sendlen<<endl;
        memset(recvbuf,0,sizeof(recvbuf));
    }
    Transock.Close();
}
int main()
{
    config();
    //ChaDet_ACK();
    //VideoTrans();
    pthread_t thread_ack;
    pthread_t thread_trans;
    pthread_create(&thread_ack,NULL,ChaDet_ACK,NULL);
    pthread_create(&thread_trans,NULL,VideoTrans,NULL);
    pthread_join(thread_ack,NULL);
    pthread_join(thread_trans,NULL);
    return 0;
}
