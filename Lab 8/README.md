# DECS-Project
Course Project of CS744 (Design &amp; Engineering of Computer Systems) under Prof. Varsha Apte

## Lab 8
1. client8.cpp
2. server8.cpp
3. loadtest8.sh
4. test.cpp	
5. client7.cpp
6. server7.cpp

Steps :
1. terminal 1 : $ g++ server7.cpp -o server7
2. terminal 1 : $ ./server7 1828
3. terminal 2 : $ g++ server8.cpp -o server8
4. terminal 2 : $ ./server8 1829
5. terminal 3 : $ bash loadtest8.sh <number_of_clients> <loop_number> <sleep_time> 
