#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>


const size_t max_msg_size = 4096; 

static void msg(const char *msg) {
    std::cout << msg << std::endl;
}
static void die(const char *msg) {
    std::cout << msg << std::endl;
    abort();
}

enum class State : uint8_t{
    STATE_REQ = 0, //read request
    STATE_RES = 1, //send response
    STATE_END = 2, //terminate connection
};

struct Conn {
    int fd = -1;
    State state = State::STATE_REQ; //default value
    
    size_t rbuf_size = 0;
    uint8_t rbuf[4 + max_msg_size];

    size_t wbuf_size = 0;
    uint8_t wbuf[4 + max_msg_size];
    size_t wbuf_sent = 0;
};

static int32_t accept_conn(std::unordered_map<int, Conn*> &fd2conn, int fd){
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
    if (connfd < 0) {
        msg("accept_conn():accept() error");
        return -1;   // error
    }
    //setting it to non blocking
    fcntl(connfd, F_SETFL, O_NONBLOCK);
    //create new struct conn containing the new connection
    Conn* conn = new Conn;
    if (!conn) {
        close(connfd);
        return -1;
    }
    conn->fd = connfd;
    //add it to our hashmap
    fd2conn[connfd] = conn;
    return 0;
}




int main(){
    std::cout << "creating the server socket..." << std::endl;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        std::cerr << "Can't create a socket!";
        return -1;
    }
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind, this is the syntax that deals with IPv4 addresses (AF_INET)
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);    // wildcard address 0.0.0.0
    std::cout << "binding socket to addr..." << std::endl;

    if (bind(fd, (const sockaddr *)&addr, sizeof(addr)) == -1) {
        die("bind(): Can't bind to IP/port");
    }

    // listen
    std::cout << "listening..." << std::endl;
    if (listen(fd, SOMAXCONN) == -1) {
        die("listen(): Can't listen");
    }

    //map of all the client connections
    std::unordered_map<int, Conn*> fd2conn; 
    //set the listening mode to non blocking
    fcntl(fd, F_SETFL, O_NONBLOCK);
    //vector of all the events
    // struct pollfd {
    //     int   fd;         /* file descriptor */
    //     short events;     /* requested events */
    //     short revents;    /* returned events */
    // };
    std::vector<pollfd> poll_args;

    while (true) {
        poll_args.clear();
        //create a pollfd for our (server) listening socket and put it in the vector
        //of events
        pollfd pfd = {fd, POLLIN, 0};
        poll_args.push_back(pfd);

        for(auto const& [key, val]: fd2conn){
            if(!val)
                continue;
                pfd = {};
                pfd.fd = val->fd;
                pfd.events = (val->state == State::STATE_REQ) ? POLLIN : POLLOUT;
                pfd.events = pfd.events | POLLERR;
                poll_args.push_back(pfd);
        }

        //check for events
        int n = poll(poll_args.data(), poll_args.size(), 1000);
        if (n < 0)
            die("Poll() error");
        
        //loop through poll_args and process active connection
        for(const pollfd pfd: poll_args){
            if(pfd.revents){
                Conn *conn = fd2conn[pfd.fd];
                //process the event relative to the connection
                //TODO
                //*****
                if(conn->state == State::STATE_END){
                    //terminate connection
                    fd2conn.erase(pfd.fd);
                    close(pfd.fd);
                    delete conn;
                }

            }
        }
        //if we are listening accept new connections
        if(poll_args[0].revents){
            accept_conn(fd2conn, fd);
        }
    }

    close(fd);
    return 0;
}