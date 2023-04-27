#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string>
#include <vector>

const size_t max_msg_size = 4096; 

enum class SER: int{
    SER_NIL = 0,
    SER_ERR = 1,
    SER_STR = 2,
    SER_INT = 3,
    SER_DBL = 4,
    SER_ARR = 5,
};

static void msg(const char *msg) {
    std::cout << msg << std::endl;
}

static void die(const char *msg) {
    std::cout << msg << std::endl;
    abort();
}
static int32_t read_full(int fd, char* buf, size_t n){
    while(n > 0){
        ssize_t len = read(fd, buf, n);
        if(len <= 0)
            return -1; //error, unexpected EOF
        assert((size_t)len <= n);
        n -= (size_t)len;
        buf += len;
    }
    return 0;
}
static int32_t write_all(int fd, char* buf, size_t n){
    while(n > 0){
        ssize_t len = write(fd, buf, n);
        if(len <= 0)
            return -1; //error, unexpected EOF
        assert((size_t)len <= n);
        n -= (size_t)len;
        buf += len;
    }
    return 0;
}

static int32_t send_req(int fd, const std::vector<std::string>& cmd){
    uint32_t len = 4;
    for(const std::string& s: cmd){
        len += 4 + s.size();
    }
    if(len > max_msg_size)
        return -1;
    
    char wbuf[4 + max_msg_size];
    memcpy(&wbuf[0], &len, 4);
    uint32_t ncmd = cmd.size();
    memcpy(&wbuf[4], &ncmd, 4);
    size_t current = 8;
    for(const std::string& s: cmd){
        uint32_t cmd_size = (uint32_t)s.size();
        memcpy(&wbuf[current], &cmd_size, 4);
        memcpy(&wbuf[current + 4], s.data(), s.size());
        current += 4 + s.size();
    }

    return write_all(fd, wbuf, 4 + len);
}

static int32_t on_response(const uint8_t* data, size_t size){
    if(size < 1){
        msg("bad response0");
        return -1;
    }

    switch (SER(data[0])){
        case SER::SER_NIL:
            std::cout << "(nil)" << std::endl;
            return 1;

        case SER::SER_ERR: {
            if(size < 1 + 8){ //1byte of SERIALIZATION CODE + 4 bytes ERRCODE + 4bytes LEN of error message
                msg("bad response1");
                return -1;
            }
            int32_t code = 0;
            uint32_t len = 0;
            memcpy(&code, &data[1], 4);
            memcpy(&len, &data[1+4], 4);
            if(size < 1+8+len){
                msg("bad response2");
                return -1;
            }
            std::cout << "(err) " << code << ": " << std::string((char*)&data[1+4+4], len) << std::endl;
            return 1+8+len;
        }
        case SER::SER_STR: {
            if(size < 1 + 4){
                msg("bad response3");
                return -1;
            }
            uint32_t len = 0;
            memcpy(&len, &data[1], 4);
            if(size < 1+4+len){
                msg("bad response4");
                return -1;
            }
            std::cout << "(str): " << std::string((char*)&data[1+4], len) << std::endl;
            return 1+4+len;
        }
        case SER::SER_INT: {
            if(size < 1 + 8){ //int are 8 bytes
                msg("bad response5");
                return -1;        
            } 
            int64_t val = 0;
            memcpy(&val, &data[1], 8);
            std::cout << "(int): " << val << std::endl;
            return 1+8;
        }
        case SER::SER_ARR: {
            if(size < 1 + 4){ //4 bytes representing the number of elements of the array
                msg("bad response6");
                return -1;        
            }
            uint32_t len = 0;
            memcpy(&len, &data[1], 4);
            std::cout << "(arr) len: " << len << std::endl;
            int arr_bytes = 1 + 4;
            for(int i=0; i<len; i++){
                int32_t rv = on_response(&data[arr_bytes], size - arr_bytes);
                if(rv < 0){
                    return rv;
                }
                arr_bytes += rv;
            }
            std::cout << "(arr) end" << std::endl;
            return arr_bytes;
        }
        default: {
            msg("bad response7");
            return -1;
        }
    }
}

static int32_t read_res(int fd){
    //Read messages if present
    char rbuf[4 + max_msg_size + 1];
    errno = 0;
    //4 bytes header
    int32_t err = read_full(fd, rbuf, 4);
    if(err){
        if(errno == 0)
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

    //reply body
    err = read_full(fd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    //handle response
    int32_t rv = on_response((uint8_t*)&rbuf[4], len);
    if(rv > 0 && (uint32_t)rv != len){
        msg("bad response-2");
        return -1;
    }

    return rv;
}

int main(int argc, char **argv) {
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

    std::vector<std::string> cmd;
    for(int i=1; i<argc; i++){
        cmd.push_back(argv[i]);
    }
    int32_t err = send_req(fd, cmd);
    if(err)
        goto L_DONE;
    err = read_res(fd);
    if(err)
        goto L_DONE;
    // int32_t err = query(fd, "hello1!");
    // if(err)
    //     goto L_DONE;

    // err = query(fd, "hello server, this is the 2nd message!");
    // if(err)
    //     goto L_DONE;

    // err = query(fd, "hello, here we are with the 3rd!");
    // if(err)
    //     goto L_DONE;



    // err = read_res(fd);
    // if(err)
    //     goto L_DONE;
    
    // err = read_res(fd);
    // if(err)
    //     goto L_DONE;
    
    // err = read_res(fd);
    // if(err)
    //     goto L_DONE;

L_DONE:
    close(fd);
    return 0;
}
