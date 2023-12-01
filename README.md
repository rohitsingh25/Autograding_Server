# DECS-Project
Course Project of CS744 (Design &amp; Engineering of Computer Systems) under Prof. Varsha Apte

### VERSION 1 ###

## Lab 6
1. client.cpp
2. server.cpp
3. test.cpp

## Lab 7
1. client7.cpp
2. server7.cpp
3. loadtest7.sh
4. test.cpp

### VERSION 2 ###

## Lab 8
1. client8.cpp
2. server8.cpp
3. loadtest8.sh
4. test.cpp	
5. client7.cpp
6. server7.cpp

### VERSION 3 ###

## Lab 9
1. client9.cpp
2. server9.cpp
3. loadtest9.cpp
4. client7.cpp
5. server7.cpp
6. client8.cpp
7. server8.cpp
8. test.cpp

### VERSION 4 ###

## Lab 10
1. server10.cpp
2. client10.cpp
3. declarations.h
4. utility.cpp
5. handle.cpp
6. prepare_server.sh
7. load.sh
8. makefile
9. test.cpp


### VERSION 1 ###

## Lab 7

Single Threaded Implementation

#Code Files 

1. server7.cpp
2. client7.cpp
3. loadtest7.sh
4. test.cpp

### HOW TO USE ###

### SERVER ###

1. g++ server7.cpp -o server
2. ./server <port_number>

### CLIENT ###

1. g++ client7.cpp -o client
2. ./client 127.0.0.1:<port_number> <test_file_name> <loop_number> <sleep_time>


### VERSION 2 ###

## Lab 8

Multi Threaded Implementation with create-destroy and TimeOut 

#Code Files 

1. server8.cpp
2. client8.cpp
3. loadtest8.sh
4. test.cpp

### HOW TO USE ###

### SERVER ###

1. g++ server8.cpp -o server
2. ./server <port_number>

### CLIENT ###

1. g++ client8.cpp -o client
2. ./client 127.0.0.1:<port_number> <test_file_name> <loop_number> <sleep_time> <timeout_sec>


### VERSION 3 ###

## Lab 9

Multi Threaded Implementation with ThreadPool and TimeOut 

#Code Files 

1. server9.cpp
2. client9.cpp
3. loadtest9.sh
4. test.cpp

### HOW TO USE ###

### SERVER ###

1. g++ server9.cpp -o server
2. ./server <port_number> <ThreadPool_size>

### CLIENT ###

1. g++ client9.cpp -o client
2. ./client 127.0.0.1:<port_number> <test_file_name> <loop_number> <sleep_time> <timeout_sec>


### VERSION 4 ###

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



--------------------------Prepared by - Yogesh Mandlik & Rohit Singh Yadav---------------------------------

