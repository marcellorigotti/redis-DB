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
#include <unordered_map>
// #include "dataStructures/hashtable.h"
// #include "dataStructures/avlTree.h"
#include "dataStructures/sortedSet.h"

const size_t max_msg_size = 4096; 
const size_t max_args = 1024;
//our custom hashtable
static SortedSet database = SortedSet();

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
    msg("accept conn");
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

enum class Rescode : int32_t{
    RES_OK = 0,
    RES_ERR = 1,
    RES_NX = 2,
};
enum class SER: int{
    SER_NIL = 0,
    SER_ERR = 1,
    SER_STR = 2,
    SER_INT = 3,
    SER_DBL = 4,
    SER_ARR = 5,
};
enum class Error{
    ERR_UNKNOWN = 1,
    ERR_2BIG = 2,
    ERR_TYPE = 3,
    ERR_ARG = 4,
};


static bool try_write(Conn* conn){
    msg("try write");  
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
    msg("handle res");
    while(try_write(conn)) {}
}
static int32_t parse_req(const uint8_t* rbuf, const uint32_t rlen, std::vector<std::string>& cmd){
    msg("parse req");
    if(rlen < 4) //not enough data, cannot read the number of commands (4 bytes needed)
        return -1; 
    uint32_t ncmd = 0;
    memcpy(&ncmd, &rbuf[0], 4); //reading the number of commands
    if(ncmd > max_args) //too many arguments
        return -1;
    
    size_t pos = 4;
    while(ncmd--){ //loop until all the commands/args are gathered
        if(pos + 4 > rlen) //check if we can read the len of the next command 
            return -1;
        uint32_t cmdlen = 0;
        memcpy(&cmdlen, &rbuf[pos], 4);
        if(pos + 4 + cmdlen > rlen) //check if we can read the command
            return -1;
        cmd.push_back(std::string((char*)&rbuf[pos+4], cmdlen));
        pos += 4 + cmdlen;
    }
    if(pos != rlen)
        return -1;

    return 0;
}

static void out_nil(std::string &out) {
    out.push_back(int(SER::SER_NIL));
}

static void out_str(std::string &out, const std::string &val) {
    out.push_back(int(SER::SER_STR));
    uint32_t len = (uint32_t)val.size();
    out.append((char *)&len, 4);
    out.append(val);
}

static void out_int(std::string &out, int64_t val) {
    out.push_back(int(SER::SER_INT));
    out.append((char *)&val, 8);
}

static void out_dbl(std::string &out, double val) {
    out.push_back(int(SER::SER_DBL));
    out.append((char *)&val, 8);
}

static void out_err(std::string &out, Error code, const std::string &msg) {
    out.push_back(int(SER::SER_ERR));
    out.append((char *)&code, 4);
    uint32_t len = (uint32_t)msg.size();
    out.append((char *)&len, 4);
    out.append(msg);
}

static void out_arr(std::string &out, uint32_t n) {
    out.push_back(int(SER::SER_ARR));
    out.append((char *)&n, 4);
}

static void out_update_arr(std::string &out, uint32_t n) {
    assert(out[0] == int(SER::SER_ARR));
    memcpy(&out[1], &n, 4);
}

static void get(const std::vector<std::string>& cmd, std::string& out){
    msg("get");
    if(!database.has(cmd[1])){
        msg("here");
        return out_nil(out);
    }
    Node** res = database.get(cmd[1]);
    if(!res)
        return out_nil(out);
    std::string res_val = std::to_string((*res)->val);
    assert(res_val.size() <= max_msg_size);
    out_str(out, res_val);
}
static void set(const std::vector<std::string>& cmd, std::string& out){
    msg("set");
    database.add((uint32_t)std::stoi(cmd[2]), cmd[1]);
    return out_nil(out);
}
static void del(const std::vector<std::string>& cmd, std::string& out){
    msg("del");
    if(database.del(cmd[1])){
        return out_int(out, 1);
    }
    return out_int(out, 0);
}
static void keys(const std::vector<std::string>& cmd, std::string& out){
    msg("keys");
    std::vector<std::string> keys = database.keys();
    out_arr(out, keys.size());
    for(const std::string& key: keys){
        out_str(out, key);
    }
}
static void query(const std::vector<std::string>& cmd, std::string& out){
    msg("query");
    AvlNode* res = database.query(std::stoi(cmd[2]), cmd[1], std::stoi(cmd[3]));
    if(!res)
        return out_nil(out); //offset node is non existing
    out_arr(out, 2);
    out_str(out, res->name);
    out_int(out, res->val);
}

//computes the request parsing the commands and elaborating them
static void compute_req(std::vector<std::string>& cmd, std::string& out){
    //TODO handle errors, for example when a str in passed instead of a number
    //what should the server do? crash? no, return an error!
    //implement error handling!
    msg("compute req");
    if(cmd.size() == 1 && cmd[0].compare("keys") == 0){
        //keys
        keys(cmd, out);
    }else if(cmd.size() == 2 && cmd[0].compare("get") == 0){
        //get name
        get(cmd, out);
    }else if(cmd.size() == 2 && cmd[0].compare("del") == 0){
        //del name
        del(cmd, out);
    }else if(cmd.size() == 3 && cmd[0].compare("set") == 0){
        //set name score
        set(cmd, out);
    }else if(cmd.size() == 4 && cmd[0].compare("query") == 0){
        //query score name offset
        query(cmd, out);
    }
    else{
        //cmd not recognized
        out_err(out, Error::ERR_UNKNOWN, "Unknown command");
    }
}
static bool one_request(Conn* conn){
    msg("one request");
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
    
    ////
    //new code
    std::vector<std::string> cmd; 
    if(parse_req(&conn->rbuf[4], len, cmd)){
        msg("bad req");
        conn->state = State::STATE_END;
        return false;
    }
    std::string out; //used as temporary storage for the response
    compute_req(cmd, out);
    if(4 + out.size() > max_msg_size){
        out.clear();
        out_err(out, Error::ERR_2BIG, "response is too big");
    }

    uint32_t wlen = (uint32_t)out.size();
    memcpy(&conn->wbuf[0], &wlen, 4); //total length of the message 
    memcpy(&conn->wbuf[4], out.data(), wlen); //length of the data (can be 0)
    conn->wbuf_size = 4 + wlen;
    //
    ////    

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
    msg("try read");
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
    msg("handle req");
    while(try_read(conn)){}
}

static void handle_conn(Conn* conn){
    msg("handle conn");
    switch (conn->state)
    {
    case State::STATE_REQ:
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
    //TODO: modify AvlTree to be initialized with no nodes
    //remember to modify the tests as well to match this
    database.del("a");

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
