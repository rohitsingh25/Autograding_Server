#!/bin/bash
numclients=$1
x=1
sleep 2
g++ client10.cpp -o submit
    for ((i=1; i<=$numclients; i++)); do        
          ./submit new 127.0.0.1:1839 test.cpp 
          done
    for ((i=1; i<=$numclients; i++)); do
          ./submit status 127.0.0.1:1839 $x 
          let x=$x+1    
   	  done
    wait
    rm req_id.txt
    echo "DONEEEE"
