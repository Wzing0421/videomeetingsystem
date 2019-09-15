#ifndef CELL_SPLITTER_H_
#define CELL_SPLITTER_H_
#include<utils.h>
class MonitorInterface;
class UdpSocket;
class FifoQueue;
class CellSplitter{
public:
    CellSplitter();
    ~CellSplitter();

    bool Init(int argc, char* argv[]);
    void MonitorLoop(); //communicate with monitor server
    void ReceiveLoop();
    void ProcessLoop();
private:
    int recv_cell_port_;     //recv cell port UDP
    bool work_over_;
    
    //struct sockaddr_in file_addr_;
    struct sockaddr_in video_addr_;
    //struct sockaddr_in message_addr_;
    //struct sockaddr_in file_config_addr_;

    MonitorInterface* monitor_interface_;
    UdpSocket* recv_socket_;
    UdpSocket* send_socket_;

    FifoQueue* fifo_queue_;
};
#endif
