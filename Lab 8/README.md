# DECS-Project
Course Project of CS744 (Design &amp; Engineering of Computer Systems) under Prof. Varsha Apte

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
