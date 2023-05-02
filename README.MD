# Redix-DB c++ project

This project focuses on the creation of a redix like database (a very basic version of it).

1) creation of a basic client/server application -> using socket and TCP.
2) implement a custom protocol to handle more than 1request from the same connection (at the moment the server can handle 1 connection at a time).
    Currently we are using blocking system call, we'll modify our DB later on to handle non blocking call and handle multiple connections.
3) added eventLoop to handle more connections simultaneously without blocking, client modified to send pipelining requests
4) implemented get, set, del operations 
5) implemented custom hashtable for our database (TODO: implement resizing when load factor too high)
6) implemented AvlTree and test for it, this will be used to build sorted set next
7) implemented sortedSet and test for it
8) used sortedSet in our server
    
To compile and run:
- cd ./build
- cmake ../
- cmake -- build .
- ./server OR ./serverEventLoop
- ./client (set || get || del || keys || query) (cmd args)

i.e. 
- ./client set aa bb -> will create an entry in the database with key aa and val bb
- ./client get aa -> return bb
- ./client del aa -> delete entry aa
- ./client keys -> list of the available keys
     
response code:
- SER_NIL = 0
- SER_ERR = 1
- SER_STR = 2
- SER_INT = 3 (1 = succefull operation, 0 = operation not done, it can be caused because we are trying to delete an element that is not present)
- SER_DBL = 4,
- SER_ARR = 5,


## PROTOCOL IMPLEMENTATION
Basic Server: get, set, del

| len | msg1 | len | msg2 | more... |
|-----|------|-----|------|---------|

len(4bytes int) = length of the following msg 

### Request:

where ms1, msg2, ... are:
| nstr | len | str1 | len | str2 | ... | len | strn |
|------|-----|------|-----|------|-----|-----|------|

nstr(4bytes int) = number of string, for each string we then have 

len(4bytes int) = lenght of the string 

str1/str2/.. = the string representing the command/value 

### Response:

where ms1, msg2, ... are:
| res | data... |
|-----|---------|

res = status code representing the outcome of the operation (4bytes int)

data = optional data (for example when calling get)
