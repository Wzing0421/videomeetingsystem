#include"UDPSocket.h"
#include<iostream>
//#include<WinSock2.h>

UDPSocket::UDPSocket()
{

}
UDPSocket::~UDPSocket()
{
	close(sock);
}
int UDPSocket::create()
{
    sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0)
{
     std::cout<<"socket initialize error"<<std::endl;
close(sock);
return -1;
}
return sock;
}
int UDPSocket::create(unsigned short port)
{
	//struct sockaddr_in localaddr;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		std::cout << "socket initialize error" << std::endl;
		close(sock);
		return -1;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	int addr_len = sizeof(addr);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
	int ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1)
	{
		std::cout << "socket bind error" << std::endl;
		close(sock);
		return -1;
	}
     return sock;
}
int UDPSocket::create(string mcastip,int mcastport)
{
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	u_int yes = 1;
	if ((m_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		std::cout << "mcastsocket error" << std::endl;
		return -1;
	}
  	if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(yes)) < 0)
	{
		std::cout << "setsockopt error" << std::endl;
		return -1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(mcastport);
	if (bind(m_sock, (struct sockaddr*)&addr, sizeof(addr))!=0)
	{
		std::cout << "msockt bind error" << std::endl;
		return -1;
	}
	mreq.imr_multiaddr.s_addr = inet_addr(mcastip.c_str());
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(m_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
	{
		std::cout << "msockt bind error" << std::endl;
		return -1;
	}
	return m_sock;
}
int UDPSocket::sendbuf(char *buf,int buflen,string destip,unsigned short destport)
{
	struct sockaddr_in dstaddr;
	memset(&dstaddr, 0, sizeof(dstaddr));
	dstaddr.sin_family = AF_INET;
	dstaddr.sin_addr.s_addr = inet_addr(destip.c_str());
	dstaddr.sin_port = htons(destport);
	return sendto(sock, buf, buflen, 0, (struct sockaddr *)&dstaddr, sizeof(dstaddr));
}
int UDPSocket::recvbuf(char *buf, int buflen, string &srcip, unsigned short  &srcport)
{
	struct sockaddr_in srcaddr;
	int from_addr_len = sizeof(struct sockaddr);
	int recvsize;
	recvsize = recvfrom(sock,buf,buflen,0,(struct sockaddr *)&srcaddr,(socklen_t *)&from_addr_len);
	if (recvsize > 0)
	{
		srcip = inet_ntoa(srcaddr.sin_addr);
		srcport = ntohs(srcaddr.sin_port);
	}
	return recvsize;
}
int UDPSocket::Close()
{
	close(sock);
	return 0;
}

