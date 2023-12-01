# DECS-Project
Course Project of CS744 (Design &amp; Engineering of Computer Systems) under Prof. Varsha Apte

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
