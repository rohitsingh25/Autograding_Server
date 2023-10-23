# DECS-Project
Course Project of CS744 (Design &amp; Engineering of Computer Systems) under Prof. Varsa Apte

## Lab 7
1. client7.cpp
2. server7.cpp
3. loadtest7.sh
4. test.cpp

Steps :

terminal 1 : $ g++ server7.cpp -o server7
terminal 1 : $ ./server7 1828
terminal 2 : $ g++ server8.cpp -o server8
terminal 2 : $ ./server8 1829
terminal 3 : $ bash loadtest8.sh <number_of_clients> <loop_number> <sleep_time> 
