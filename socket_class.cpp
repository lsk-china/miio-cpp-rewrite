#include "socket_class.h"


int UdpServer::Create(void)
{
    sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd_ < 0)
    {
        perror("Cannot create socket");
        return -1;
    }
    return 0;
}


void UdpServer::FillStructSockaddr(u_int16_t port)
{
    memset(&addr_serv_, 0, sizeof(struct sockaddr_in));
    addr_serv_.sin_family = AF_INET;
    addr_serv_.sin_port = htons(port);
    addr_serv_.sin_addr.s_addr = htonl(INADDR_ANY);
}

int UdpServer::Bind(void)
{
    return bind(sock_fd_, (struct sockaddr*) & addr_serv_, sizeof(addr_serv_));
}

void UdpServer::StartReceive(char* recv_buf, u_int32_t recv_buf_size, OnReceiveCallbackFunc callback_func, void* data)
{
    int loop = 1;
    int recv_num;
    struct sockaddr_in addr_client;
    size_t addr_size = sizeof(addr_client);
    while (loop)
    {
        recv_num = (int)recvfrom(sock_fd_, recv_buf, recv_buf_size, 0, (struct sockaddr*) & addr_client, (socklen_t*)&addr_size);
        if (recv_num < 0)
        {
            perror("Receive from socket failed");
        }
        loop = callback_func(recv_num, &addr_client, recv_buf, data);
    }
}

void UdpServer::Close(void)
{
    close(sock_fd_);
}

int UdpServer::OpenUdpServer(u_int16_t port, char* recv_buf, u_int32_t recv_buf_size,
                            OnReceiveCallbackFunc callback_func, void* data)
{
    if (Create() != 0) {
        return -1;
    }
    FillStructSockaddr(port);
    if (Bind() != 0) {
        perror("Cannot bind");
        return -1;
    }
    StartReceive(recv_buf, recv_buf_size, callback_func, data);
}

//------------------------------------------------------------------------------------------------

int UdpClient::Create(void)
{
    sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd_ < 0)
    {
        perror("socket (UdpClient) create fail");
        return 0;
    }
    return 1;
}

void UdpClient::FillStructSockaddr(struct sockaddr_in* addr_serv)
{
    addr_serv_ = *addr_serv;
}

void UdpClient::FillStructSockaddr(const char* ip_addr, u_int16_t port)
{
    memset(&addr_serv_, 0, sizeof(addr_serv_));
    addr_serv_.sin_family = AF_INET;
    addr_serv_.sin_addr.s_addr = inet_addr(ip_addr);
    addr_serv_.sin_port = htons(port);
}

int UdpClient::SendOne(char* send_buf, u_int32_t send_buf_size)
{
    int send_num;
    socklen_t addr_len = (socklen_t)sizeof(addr_serv_);
    send_num = (int)sendto(sock_fd_, send_buf, send_buf_size, 0, (struct sockaddr*) & addr_serv_, addr_len);
    if (send_num < 0)
    {
        perror("socket (UdpClient) send fail");
    }
    return send_num;
}

int UdpClient::RecvOne(char* recv_buf, u_int32_t recv_buf_size)
{
    int recv_num;
    socklen_t addr_len = (socklen_t)sizeof(addr_serv_);
    recv_num = (int)recvfrom(sock_fd_, recv_buf, recv_buf_size, 0, (struct sockaddr*) & addr_serv_, &addr_len);
    if (recv_num < 0)
    {
        perror("socket (UdpClient) receive fail");
    }
    return recv_num;
}

void UdpClient::Close(void)
{
    close(sock_fd_);
}

int UdpClient::OpenUdpClient(struct sockaddr_in* addr_serv)
{
    if (Create())
    {
        FillStructSockaddr(addr_serv);
        return 1;
    }
    return 0;
}

int UdpClient::SendSync(char* send_buf, u_int32_t send_buf_size, char* recv_buf, u_int32_t recv_buf_size)
{
    if (SendOne(send_buf, send_buf_size) < 0)
    {
        return -1;
    }
    int recv_num = RecvOne(recv_buf, recv_buf_size);
    if (recv_num < 0)
    {
        return -2;
    }
    return recv_num;
}

//------------------------------------------------------------------------------------------------


