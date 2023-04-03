#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>

const size_t max_msg_size = 4096; 

static void msg(const char *msg) {
    std::cout << msg << std::endl;
}

static void die(const char *msg) {
    std::cout << msg << std::endl;
    abort();
}
 

static int32_t read_full(int fd, char* buf, size_t n){
    while(n > 0){
        size_t len = read(fd, buf, n);
        if(len <= 0)
            return -1; //error, unexpected EOF
        assert(len <= n);
        n -= len;
        buf += len;
    }
    return 0;
}
static int32_t write_all(int fd, char* buf, size_t n){
    while(n > 0){
        size_t len = write(fd, buf, n);
        if(len <= 0)
            return -1; //error, unexpected EOF
        assert(len <= n);
        n -= len;
        buf += len;
    }
    return 0;
}


static int32_t one_request(int connfd) {
    //let's use a protocol where the first 4 bytes represent the length of the message
    // +-----+------+-----+------+--------
    // | len | msg1 | len | msg2 | more...
    // +-----+------+-----+------+--------
    char rbuf[4 + max_msg_size + 1];
    
    //4 bytes header
    int32_t err = read_full(connfd, rbuf, 4);
    if(err){ 
        if(err == -1)
            msg("EOF");
        else
            msg("Read error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4); //endianess to be taken into account
    if(len > max_msg_size){
        msg("Msg content too long");
        return -1;
    }

    //body content of the message
    //read a message of length len starting from the 5th position of the buffer 
    err = read_full(connfd, &rbuf[4], len);
    if(err){
        msg("Read() error");
        return err;
    }
    rbuf[4 + len] = '\0';
    std::cout << "Client msg: " << &rbuf[4] << std::endl;


    //OLD CODE
    // std::cout << "client says: " << std::string(rbuf, 0 , n) << std::endl;

    //send back an answer (using same protocol as the sender)
    const char reply[] = "Message received!";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)std::strlen(reply);
    memcpy(&wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);
    return write_all(connfd, wbuf, 4 + len);
    // send(connfd, wbuf, strlen(wbuf), 0); // same as using write if flags set to 0 (this case)
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

    while (true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        std::cout << "accepting client call..." << std::endl;
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd < 0) {
            continue;   // error
        }
        std::cout << "Client address: " << inet_ntoa(client_addr.sin_addr) << " and port: " << client_addr.sin_port << std::endl;
        while(true){
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }
        close(connfd);
    }

    close(fd);
}