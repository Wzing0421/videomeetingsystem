#ifndef MONITOR_INTERFACE_H_
#define MONITOR_INTERFACE_H_
#include<stdint.h>
#include<string>
#include<tcp_socket.h>
class CellSplitter;
class MonitorInterface: public TcpSocket{
public:
    MonitorInterface(CellSplitter*);
    ~MonitorInterface();

    bool ConnectToServer(uint16_t local_port, const char* remote_ip, uint16_t remote_port);
    bool ReconnectToServer();
    bool Register();
    void UnRegister();

    bool ProcessPacket(char* packet, int len);
private:
    enum ClientType{
        RECV_DRIVER = 0x00,
		FILE_RECEIVER = 0x01,
		VIDEO_RECEIVER = 0x02,
	    CELL_SPLITTER = 0x03,
		MSG_RECEIVER = 0x04,
        APP_CLIENT = 0x05
    };
    enum SignalType{ //signal type between module's communication with  monitor server
        REGISTER = 0x00,   
        UNREGISTER = 0x01, 
        REPORT_STATE = 0x02,
        QUERY_STATE = 0x03,
        TERMINATE = 0x10
    };


    CellSplitter* cell_splitter_;
    std::string remote_ip_;
    uint16_t remote_port_;
    uint16_t local_port_;
    uint32_t request_seq_;
};
#endif
