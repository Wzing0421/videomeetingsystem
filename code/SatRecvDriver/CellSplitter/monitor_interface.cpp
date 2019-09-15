#include<stdint.h>
#include<iostream>
#include<udp_socket.h>
#include<tcp_socket.h>
#include<logger.h>

#include"monitor_interface.h"
#include"cell_splitter.h"

using namespace std;
extern Logger g_logger;

MonitorInterface::MonitorInterface(CellSplitter* splitter):
cell_splitter_(splitter),
request_seq_(0){

}

MonitorInterface::~MonitorInterface(){

}

bool MonitorInterface::ConnectToServer(uint16_t local_port, const char* remote_ip, uint16_t remote_port){
    if (! this->Create()){    //video receiver's tcp port as a subprocess client   
            return false;
    }
    local_port_ = local_port;
    if (! this->Bind("0.0.0.0", local_port)){
        return false;
    }

    int count = 3;
    while(count -- ){
        if (this->Connect(remote_ip, remote_port)){
            remote_ip_ = std::string(remote_ip);
            remote_port_ = remote_port;
            return true;
        }
#ifdef LINUX
        sleep(1);
#else 
		Sleep(1000);
#endif
    }
    return false;
}


bool MonitorInterface::ReconnectToServer(){
    this->Close();
    if (! this->Create()){    //video receiver's tcp port as a subprocess client   
         return false;
    }

    if (! this->Bind("0.0.0.0", local_port_)){
        return false;
    }
    int count = 3;
    while(count -- ){
        if (this->Connect(remote_ip_.c_str(), remote_port_)){
            return true;
        }
#ifdef LINUX
        sleep(1);
#else 
		Sleep(1000);
#endif
    }
    return false;
}


bool MonitorInterface::Register(){
    char send_buf[100];
    char* send_pos = send_buf;
    *send_pos++ = CELL_SPLITTER;    
    *(uint32_t*)send_pos = ++request_seq_;
    send_pos += sizeof(uint32_t);
    *send_pos++ = 0x00;
    *send_pos++ = REGISTER;
    *(uint32_t*)send_pos = htonl(0);
    send_pos += sizeof(uint32_t);
    this->SendPacket(send_buf, send_pos - send_buf);
    
    char recv_buffer[100];
    int recved_len;
    int count = 0, complete_pack = 0;
    while(true){
        complete_pack = this->RecvPacket(recv_buffer, &recved_len);
        if(complete_pack)
          break;
#ifdef LINUX
        usleep(10000);
#else
        Sleep(10);
#endif
    }
    if (complete_pack < 0 || count == 1000)
        return false;
    //process the pakcet....
    //
    return true;
}


bool MonitorInterface::ProcessPacket(char* packet, int len){
    return true;
}


