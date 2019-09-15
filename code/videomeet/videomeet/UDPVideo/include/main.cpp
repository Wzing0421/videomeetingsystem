#include"UDPSocket.h"
#define MCASTPORT  5052    
#define MCASTADDR "225.0.0.1"    
#define MSGBUFSIZE 40000
string dstaddr;
int dstport =  30002;   
int main(int argc, char *argv[])    
{    
    //send_udp initialization
    UDPSocket  ssock;
    ssock.create(10000);
    dstaddr=argv[1];
    //define mcastaddr
    struct sockaddr_in addr;    
    int sock, recvsize,addrlen;    
    struct ip_mreq mreq;    
    char recvbuf[MSGBUFSIZE];    
    u_int yes=1; /*** MODIFICATION TO ORIGINAL */    
    /* create what looks like an ordinary UDP socket */    
    if ((sock=socket(AF_INET,SOCK_DGRAM,0)) < 0)     
    {    
        perror("socket");    
        exit(1);    
    }    
    /**** MODIFICATION TO ORIGINAL */    
    /* allow multiple sockets to use the same PORT number */    
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0)     
    {    
        perror("Reusing ADDR failed");    
        exit(1);    
    }    
    /*** END OF MODIFICATION TO ORIGINAL */    
    /* set up destination address */    
    memset(&addr,0,sizeof(addr));    
    addr.sin_family=AF_INET;    
    addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */    
    addr.sin_port=htons(MCASTPORT);    
    /* bind to receive address */    
    if (bind(sock,(struct sockaddr *) &addr,sizeof(addr)) < 0)    
    {    
        perror("bind");    
        exit(1);    
    }    
    /* use setsockopt() to request that the kernel join a multicast group */    
    mreq.imr_multiaddr.s_addr=inet_addr(MCASTADDR);    
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);    
    if (setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)     
    {    
        perror("setsockopt");    
        exit(1);    
    }    
    /* now just enter a read-print loop */    
    int count=0;
    while (1)     
    {    
        //ssize_t recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);    
        addrlen=sizeof(addr);    
        recvsize=recvfrom(sock, recvbuf, MSGBUFSIZE, 0, (struct sockaddr *) &addr, (socklen_t *)&addrlen);
        if (recvsize< 0)     
        {    
            perror("recvfrom");    
            exit(1);    
        }
        printf("the length is %d,the count is %d\n",recvsize,count);
        ++count;
       //printf("buf1 %x,buf2 is%x",recvbuf[0]&0xff,recvbuf[1]&0xff);
       //uint16_t n=ntohs(*(uint16_t*)recvbuf);
       //printf("buf is %x\n",n);
       //printf("buf1 is %x,buf2 is %x\n",n&0xff00,n&0x00ff);
    	//printf("the group ip",inet_ntoa())
       // sendto(ssockfd,recvbuf, recvsize, 0, (struct sockaddr *)&ssin, sizeof(ssin));
        int ret=ssock.sendbuf(recvbuf,recvsize,dstaddr,dstport);
        std::cout<<ret<<std::endl;
        memset(recvbuf,0,MSGBUFSIZE);
        //puts(msgbuf);
            
    }    
    return 0;    
}    
