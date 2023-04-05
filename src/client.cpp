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
        std::cout << n << std::endl;
        size_t len = read(fd, buf, n);
        std::cout << len << std::endl;
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

static int32_t query(int fd, const char* text){
    //send message using the protocol stated in server.cpp
    uint32_t len = (uint32_t)std::strlen(text);
    if(len > max_msg_size){
        return -1;
    }
    char wbuf[4 + max_msg_size];
    memcpy(&wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);
    if(int32_t err = write_all(fd, wbuf, 4 + len))
        return err;
    

    //Read messages if present
    char rbuf[4 + max_msg_size + 1];
    //4 bytes header
    int32_t err = read_full(fd, rbuf, 4);
    if(err){
        if(err == -1)
            msg("EOF");
        else
            msg("Read error");
        return err;
    }
    
    memcpy(&len, rbuf, 4); //endianess to be taken into account
    if(len > max_msg_size){
        msg("Msg content too long");
        return -1;
    }

    err = read_full(fd, &rbuf[4], len);
    if(err){
        msg("Read() error");
        return err;
    }
    rbuf[4 + len] = '\0';
    std::cout << "Server msg: " << &rbuf[4] << std::endl;
    
    return 0;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("connect");
    }

    int32_t err = query(fd, "hello1!");
    if(err)
        goto L_DONE;

    err = query(fd, "hello server, this is the 2nd message!");
    if(err)
        goto L_DONE;

    err = query(fd, "hello, here we are with the 3rd!");
    if(err)
        goto L_DONE;


L_DONE:
    close(fd);
    return 0;
}
