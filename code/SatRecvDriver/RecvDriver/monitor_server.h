#ifndef MONITOR_SERVER_H_
#define MONITOR_SERVER_H_
#define FILE_PATH_LENGTH 256
#include<map>
#include<string>
#include<udp_socket.h>
#include<tcp_socket.h>
#include"epoll_wrapper.h"
//pack between client module and monitor server
//    length          4 Byte
//    signal type     1 Byte
//    signal data     x Byte
//

struct TransactionInfo{
    uint32_t id;
    uint8_t type;
    uint8_t state;          //接收状态
    uint8_t progress;       //接收百分比
    std::string nfs_path;   //文件在nfs服务器上的路径，用于向后端28所模块报告
    std::string local_path; //本地存储路径
    uint64_t file_length;
    uint64_t received_length;
};

class MonitorServer{
public:
    MonitorServer();
    ~MonitorServer();

    void WorkLoop(); //work loop, epoll monitor all file descriptors and process msg
    bool StartServer();

private:
    void AddObservedClient(int socket_fd);
    void DelObservedClient(int socket_fd);
    void HandleEvent(int socket_fd);

    void ProcessPacketFromOtherModule(TcpSocket* client, int len,  char* data);
	void ProcessPacketFromAppClient(int len, char* data);

    void SetAddressAndPort(struct in_addr  address, int Port);
	void OnReceiveRegister(TcpSocket* client, char* recv_data, int len);
	void OnReceiveUnregister(TcpSocket* client, char* recv_data, int len);
	bool OnReceiveState(TcpSocket* client, char* recv_data, int len);
	void SendNotificationToAppClient(uint32_t transaction_id);
	bool SendCpuAndMem();
	bool SendTransactionStatus(uint32_t seq);
	bool SendTransactionStatus(uint32_t seq, uint32_t transaction_id);

	std::map<uint32_t, TransactionInfo>  tid_to_transinfo_;

    enum ClientType{
        RECV_DRIVER = 0x00,
		FILE_RECEIVER = 0x01,
		VIDEO_RECEIVER = 0x02,
	    CELL_SPLITTER = 0x03,
		MSG_RECEIVER = 0x04,
        APP_CLIENT = 0x05
    };

    enum TransactionState{
        NEW_TRANSACTION = 0x00,
        CANCEL_TRANSACTION = 0x01,
        UPDATE_TRANSACTION = 0x02,
        BEGIN_TRANSACTION = 0x03,
        FINISH_TRANSACTION = 0x04,
        DOING_TRANSACTION = 0x05
    };

    enum SignalType{ //signal type between module's communication with  monitor server
        REGISTER = 0x00,   
        UNREGISTER = 0x01, 
        REPORT_STATE = 0x02,
        QUERY_STATE = 0x03,
        TERMINATE = 0x10
    };

    enum AppRequest{
        HEARTBEAT = 0x00,
        STATUS = 0x10,
        SETIPANDPORT = 0x30,
        OTHER1 = 0x04
    };

    enum AppAnswer{
        HEARTBEATREPLY = 0x01,
        STATUSREPLY = 0x02,
        SETIPANDPORTREPLY = 0x03,
        REMIND = 0x04
    };
    uint32_t request_seq_;
    std::map<int, int>  client_type_to_sockfd_;
    std::map<int, CommonSocket*> observed_client_;
    //接受各个模块的tcp连接
    TcpSocket server_socket_;
    //向后端28所模块发送udp
    UdpSocket app_socket_; 
    sockaddr_in web_sockaddr_;
    sockaddr_in status_sockaddr_;
#ifdef PTHREAD
    Epoll epoll_;  //epoll
#endif
};
#endif

