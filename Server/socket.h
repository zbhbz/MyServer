#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace myserver{

// RAII
class Socket {
public:
    Socket();
    ~Socket();

    int getFd() const {return serverFd_;}

    void setReuseAddr();
    void setNonBlocking();

    bool bindAddr(int port);
    bool listen();
    int accept(struct sockaddr_in &clientAddr);
    bool close();
private:
    int serverFd_;
};
}
#endif // SOCKET_H
