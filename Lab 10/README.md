# DECS-Project
Course Project of CS744 (Design &amp; Engineering of Computer Systems) under Prof. Varsha Apte

## Lab 10

Asynchronous Serever design with Threadpool

#Code Files 

1. server10.cpp
2. client10.cpp
3. declarations.h
4. utility.cpp
5. handle.cpp
6. prepare_server.sh
7. load.sh
8. makefile
9. test.cpp


# 3 Directories

1. Backup
2. Analysis
3. Results


### HOW TO USE ###

### SERVER ###
1. bash prepare_server.sh
2. ./server <port_number> <Threadpool_size>

### CLIENT ###
1. bash load.sh <port_number> <number_of_clients>

// also we can run client one by one 
1. g++ client10.cpp -o submit

// for new request 
2. ./submit new 127.0.0.1:<port_number> <test_file_name>

// for status request
2. ./submit status 127.0.0.1:<port_number> <request_id>



