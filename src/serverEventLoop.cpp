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
static bool try_write(Conn* conn){  
    ssize_t len = 0;
    do{
        size_t remain = conn->wbuf_size - conn->wbuf_sent;
        len = write(conn->fd, &conn->wbuf[conn->wbuf_sent], remain);
    }while(len < 0 && errno == EINTR);
    if (len < 0 && errno == EAGAIN) 
        return false; // got EAGAIN, try later;
    if (len < 0){
        msg("write() error");
        conn->state = State::STATE_END;
        return false;
    }

    conn->wbuf_sent += (size_t)len;
    assert(conn->wbuf_sent <= conn->wbuf_size);

    if(conn->wbuf_sent == conn->wbuf_size){
        //response fully sent
        conn->state = State::STATE_REQ;
        conn->wbuf_sent = 0;
        conn->wbuf_size = 0;
        return false;
    }
    //if not fully send try again and send what's missing
    return true;
}
static void handle_res(Conn* conn){
    while(try_write(conn)) {}
}
static bool one_request(Conn* conn){
    //try to parse one request
    if(conn->rbuf_size < 4)
        return false; //not enough data, we'll try later on
    uint32_t len = 0;
    memcpy(&len, &conn->rbuf[0], 4);
    if(len > max_msg_size){
        msg("Msg content too long");
        conn->state = State::STATE_END;
        return false;
    }
    if(4 + len > conn->rbuf_size)
        return false; //not enough data, we'll try later on
    std::cout << "Msg received, providing response" << std::endl;
    //create echo response
    memcpy(&conn->wbuf[0], &len, 4);
    memcpy(&conn->wbuf[4], &conn->rbuf[4], len);
    conn->wbuf_size = (size_t)(4 + len);

    //remove what we have just processed from the buffer
    //multiple memmove call are inefficient, optimize this later on
    //TODO
    size_t remain = conn->rbuf_size - 4 - len;
    if(remain){
        memmove(conn->rbuf, &conn->rbuf[4+len], remain);
    }
    conn->rbuf_size = remain;

    conn->state = State::STATE_RES;
    handle_res(conn);

    return (conn->state == State::STATE_REQ);
}

static bool try_read(Conn* conn){
    assert(conn->rbuf_size <= sizeof(conn->rbuf));
    ssize_t len = 0;
    do{
        size_t cap = sizeof(conn->rbuf) - conn->rbuf_size;
        len = read(conn->fd, &conn->rbuf[conn->rbuf_size], cap);
    }while(len < 0 && errno == EINTR); //retry if errno == EINTR
    if (len < 0 && errno == EAGAIN) 
        return false; // got EAGAIN, try later;
    if (len < 0) {
        msg("read() error");
        conn->state = State::STATE_END;
        return false;
    }
    if(len == 0){
        if (conn->rbuf_size > 0) {
            msg("unexpected EOF");
        } else {
            msg("EOF");
        }
        conn->state = State::STATE_END;
        return false;
    }

    conn->rbuf_size += (size_t)len;
    assert(conn->rbuf_size <= sizeof(conn->rbuf) - conn->rbuf_size);

    //process the requests, there could be more than 1!
    while(one_request(conn)) {}
    return(conn->state == State::STATE_REQ);
}


static void handle_req(Conn* conn){
    while(try_read(conn)){}
}

static void handle_conn(Conn* conn){
    switch (conn->state)
    {
    case State::STATE_REQ :
        handle_req(conn);
        break;
    case State::STATE_RES:
        handle_res(conn);
        break;
    default:
        return;
    }
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
        for(size_t i = 1; i<poll_args.size(); i++){
            if(poll_args[i].revents){
                Conn* conn = fd2conn[poll_args[i].fd];
                //process the event relative to the connection
                handle_conn(conn);
                if(conn->state == State::STATE_END){
                    //terminate connection
                    fd2conn.erase(conn->fd);
                    close(conn->fd);
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