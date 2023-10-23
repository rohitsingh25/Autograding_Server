#!/bin/bash
numclients=$1
a=$1
loop=$2
sleeptime=$3
g++ server7.cpp -o server
#./server 1528 &
sleep 2
g++ client7.cpp -o submit
# Create a file to store the Throughput and response time values
throughput_file="throughput_data1.txt"
response_time_file="response_time_data1.txt"
echo "Clients Throughput AverageResponseTime" > "$throughput_file"
echo "Clients AverageResponseTime" > "$response_time_file"
#for loop  for varying number of clients
for ((j=1; j<=10; j++)); do
for ((i=1; i<=$numclients; i++)); do
    (./submit 127.0.0.1:1528 test.cpp "$loop" "$sleeptime" > "otpt_$j$i.txt" ) &
    #f_name="otpt_$j$i.txt"
    #echo "$otpt" > "$f_name"
done
wait
#changing number of clients for next iteration by a step size equal to 10
numclients=$(($numclients + 10))
done
# Kill the server after all iterations
killall server


for ((j=1; j<=10; j++)); do
th=0
overallThroughput=0
resTime=0
res=0
totalN=0
for ((i=1; i<=$a; i++)); do
    th=$(grep "Throughput" "otpt_$j$i.txt" | cut -d ':' -f 2)
    overallThroughput=$(echo "$overallThroughput + $th" | bc -l)
    n=$(grep "Number of successful responses" "otpt_$j$i.txt" | cut -d ':' -f 2)
    res=$(grep "Avg response time in microseconds" "otpt_$j$i.txt" | cut -d ':' -f 2)
    resTime=$(echo "$res + $resTime" | bc -l)
    totalN=$((totalN + n))   
done
th=$(echo "scale=6; $overallThroughput" | bc -l)
res=$(echo "scale=6; $resTime / $totalN" | bc -l)
# Append data to throughput file
echo "$(($a)) $th" >> "$throughput_file"
# Append data to response time file
echo "$(($a)) $res" >> "$response_time_file"
a=$((a + 10))
done


# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "throughput_graph.png"
set title "Number of Clients vs. Throughput"
set xlabel "Number of Clients"
set ylabel "Throughput"
set grid
plot "$throughput_file" using 1:2 with linespoints title "Throughput"
EOF

# Use Gnuplot to create the response time graph
gnuplot <<EOF
set terminal png
set output "response_time_graph.png"
set title "Number of Clients vs. Average Response Time"
set xlabel "Number of Clients"
set ylabel "Average Response Time(in microseconds)"
set grid
plot "$response_time_file" using 1:2 with linespoints title "Average Response Time"
EOF

echo "Graphs created: throughput_graph.png and response_time_graph.png"

