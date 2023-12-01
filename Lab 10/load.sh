#!/bin/bash
numclients=$2
port=$1
sleep 1
g++ client10.cpp -o submit

      for ((i=1; i<=$numclients; i++)); do        
            ./submit new 127.0.0.1:$port test.cpp &
      done
      
      for ((i=1; i<=$numclients; i++)); do
            ./submit status 127.0.0.1:$port $i >> Analysis/req_id_$i.txt &
      done 
      
      wait
      for ((i=1; i<=$numclients; i++)); do
            ./submit status 127.0.0.1:$port $i >> Analysis/req_id_$i.txt &
      done 
      wait
      for ((i=1; i<=$numclients; i++)); do
            ./submit status 127.0.0.1:$port $i >> Analysis/req_id_$i.txt &
      done 
      
      wait

rm submit

echo "DONEEEE"