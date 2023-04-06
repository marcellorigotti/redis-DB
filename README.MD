# Redix-DB c++ project

This project focuses on the creation of a redix like database (a very basic version of it).

1) creation of a basic client/server application -> using socket and TCP.
2) implement a custom protocol to handle more than 1request from the same connection (at the moment the server can handle 1 connection at a time).
    Currently we are using blocking system call, we'll modify our DB later on to handle non blocking call and handle multiple connections.
3) added eventLoop to handle more connections simultaneously without blocking, client modified to send pipelining requests
    
To compile and run:
- cd ./build
- cmake ../
- cmake -- build .
- ./server OR ./serverEventLoop
- ./client