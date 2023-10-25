#!/bin/bash
numclients=$1
a=$1
b=$1
loop=$2
sleeptime=$3
g++ tryserv.cpp -o server
#CPU_AFFINITY="0,1"
#./server 1829 &
#server_pid=$!
sleep 2
g++ client9.cpp -o submit
# Create a file to store the Throughput and response time values
throughput_file="throughput_data.txt"
response_time_file="response_time_data.txt"
cpu_utilization_file="cpu_utilization.txt"
active_threads_file="active_threads.txt"
error_rate_file="errors.txt"
timeout_rate_file="tmt.txt"
req_rate_file="req.txt"
Average_number_of_request_in_queue_file="Average_number_of_request_in_queue.txt"
echo "Clients Throughput" > "$throughput_file"
echo "Clients AverageResponseTime" > "$response_time_file"
echo "Clients Error_rate" > "$error_rate_file"
echo "Clients Timeout_rate" > "$timeout_rate_file"
echo "Clients req_rate" > "$req_rate_file"
# Loop for varying numbers of clients
for ((j=1; j<=2; j++)); do
    for ((i=1; i<=$numclients; i++)); do        
          #otpt=$(./submit 127.0.0.1:1830 test.cpp "$loop" "$sleeptime" 1000 &) 
          (./submit 127.0.0.1:1831 test.cpp "$loop" "$sleeptime" 9000 > "otpt_$j$i.txt") &
          (timeout $((loop * sleeptime * numclients + 5)) mpstat -P 0,1 10 >> "cpu_utilization$j.txt") &
          (ps -C server -Lf >> "active_threads$j.txt") &
          #f_name="otpt_$j$i.txt"
          #echo "$otpt" > "$f_name"    
    done
    wait
    average_requests_in_queue=$(awk '{sum+=$1} END {print sum/(NR+1)}' "abc.txt")
    echo "$((numclients)) $average_requests_in_queue" >> "$Average_number_of_request_in_queue_file"
    #sed -i '/UID/d' "active_threads$j.txt"
    sed -i '/CPU/d' "cpu_utilization$j.txt"
     # Calculate average number of active threads from the ps output
    average_active_threads=$(awk '{sum+=$6} END {print sum/(NR+1)}' "active_threads$j.txt")
    rm "active_threads$j.txt"
    echo "$(($numclients)) $average_active_threads" >> "$active_threads_file" 
    # Calculate average CPU utilization from the vmstat output
    average_cpu_utilization=$(awk '{sum+=100-$14} END {print sum/(NR+1)}' "cpu_utilization$j.txt")
    rm "cpu_utilization$j.txt"
    echo "$(($numclients)) $average_cpu_utilization" >> "$cpu_utilization_file"
    # Changing the number of clients for the next iteration by a step size equal to 10
    numclients=$(($numclients+10))  
done

# Kill the server after all iterations
#killall server
for ((j=1; j<=2; j++)); do
th=0
overallThroughput=0
resTime=0
res=0
totalN=0
err=0
terr=0
tmt=0
tt=0
reqrate=0
treqrate=0
for ((i=1; i<=$a; i++)); do
    th=$(grep "Throughput" "otpt_$j$i.txt" | cut -d ':' -f 2)
    overallThroughput=$(echo "$overallThroughput + $th" | bc -l)
    err=$(grep "errors" "otpt_$j$i.txt" | cut -d ':' -f 2)
    terr=$(echo "$terr + $err" | bc -l)
    tmt=$(grep "timeout" "otpt_$j$i.txt" | cut -d ':' -f 2)
    tt=$(echo "$tt + $tmt" | bc -l)
    n=$(grep "Number of successful responses" "otpt_$j$i.txt" | cut -d ':' -f 2)
    res=$(grep "Avg response time in microseconds" "otpt_$j$i.txt" | cut -d ':' -f 2)
    resTime=$(echo "$resTime + ($res * $n)" | bc -l)
    reqrate=$(grep "Request rate sent" "otpt_$j$i.txt" | cut -d ':' -f 2)
    treqrate=$(echo "$treqrate + $reqrate" | bc -l)
    totalN=$((totalN + n))   
done
if [ "$totalN" -eq 0 ]; then
    res=500000
else
    th=$(echo "scale=6; $overallThroughput" | bc -l)
    res=$(echo "scale=6; $resTime / $totalN" | bc -l)
    tmt=$(echo "scale=6; $tt" | bc -l)
    err=$(echo "scale=6; $terr" | bc -l)
    
fi
# Append data to throughput file
echo "$(($a)) $overallThroughput" >> "$throughput_file"
# Append data to response time file
echo "$(($a)) $res" >> "$response_time_file"
# Append data to error.txt
echo "$(($a)) $err" >> "$error_rate_file"
# Append data to tmt.txt
echo "$(($a)) $tmt" >> "$timeout_rate_file"
# Append data to req.txt
echo "$(($a)) $treqrate" >> "$req_rate_file"
a=$((a+10))
done
for ((j=1; j<=2; j++)); do
	for ((i=1; i<=$b; i++)); do
	rm "otpt_$j$i.txt"
done 
b=$((b+9))	
done
# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "Throughput_graph_v3.png"
set title "Number of Clients vs. Throughput"
set xlabel "Number of Clients"
set ylabel "Throughput"
set grid
plot "$throughput_file" using 1:2 with linespoints title "Throughput"
EOF
# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "Response_time_graph_v3.png"
set title "Number of Clients vs. Response Time"
set xlabel "Number of Clients"
set ylabel "Response Time"
set grid
plot "$response_time_file" using 1:2 with linespoints title "Response Time"
EOF
# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "utilization_graph_v3.png"
set title "Number of Clients vs. CPU Utilization"
set xlabel "Number of Clients"
set ylabel "CPU Utilization (%)"
set grid
plot "$cpu_utilization_file" using 1:2 with linespoints title "CPU Utilization"
EOF

# Use Gnuplot to create the response time graph
gnuplot <<EOF
set terminal png
set output "active_threads_graph_v3.png"
set title "Number of Clients vs. Average Active Threads"
set xlabel "Number of Clients"
set ylabel "Average Active Threads"
set grid
plot "$active_threads_file"  using 1:2 with linespoints title "Active Threads"
EOF
# Use Gnuplot to create the error rate graph
gnuplot <<EOF
set terminal png
set output "error_rate_graph_v3.png"
set title "Number of Clients vs. Error Rate"
set xlabel "Number of Clients"
set ylabel "Error Rate"
set grid
plot "$error_rate_file"  using 1:2 with linespoints title "ERROR RATE"
EOF

# Use Gnuplot to create the timeout rate graph
gnuplot <<EOF
set terminal png
set output "timeout_rate_graph_v3.png"
set title "Number of Clients vs. Timeout Rate"
set xlabel "Number of Clients"
set ylabel "Timeout Rate"
set grid
plot "$timeout_rate_file"  using 1:2 with linespoints title "TIMEOUT RATE"
EOF

# Use Gnuplot to create the request sent rate graph
gnuplot <<EOF
set terminal png
set output "req_rate_graph_v3.png"
set title "Number of Clients vs. Request Sent Rate"
set xlabel "Number of Clients"
set ylabel "Request Sent Rate"
set grid
plot "$req_rate_file"  using 1:2 with linespoints title "Request Sent Rate"
EOF
# Use Gnuplot to create the Average number of request in queue graph
gnuplot <<EOF
set terminal png
set output "Average_number_of_request_in_queue_graph_v3.png"
set title "Number of Clients vs. Average number of request in queue"
set xlabel "Number of Clients"
set ylabel "Average number of request in queue"
set grid
plot "$Average_number_of_request_in_queue_file"  using 1:2 with linespoints title "Average number of request in the queue"
EOF

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
############################################################################
######################   version 3 ended	############################
############################################################################
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXxXXXXX
#!/bin/bash
numclients=$1
a=$1
b=$1
loop=$2
sleeptime=$3
#g++ tryserv.cpp -o server
#CPU_AFFINITY="0,1"
#./server 1829 &
#server_pid=$!
sleep 2
g++ client9.cpp -o submit
# Create a file to store the Throughput and response time values
throughput_file_v2="throughput_data_ver2.txt"
response_time_file_v2="response_time_data_ver2.txt"
cpu_utilization_file_v2="cpu_utilization_ver2.txt"
active_threads_file_v2="active_threads_ver2.txt"
error_rate_file_v2="errors_ver2.txt"
timeout_rate_file_v2="tmt_ver2.txt"
req_rate_file_v2="req_ver2.txt"
echo "Clients Throughput" > "$throughput_file_v2"
echo "Clients AverageResponseTime" > "$response_time_file_v2"
echo "Clients Error_rate" > "$error_rate_file_v2"
echo "Clients Timeout_rate" > "$timeout_rate_file_v2"
echo "Clients req_rate" > "$req_rate_file_v2"
# Loop for varying numbers of clients
for ((j=1; j<=2; j++)); do
    for ((i=1; i<=$numclients; i++)); do        
          #otpt=$(./submit 127.0.0.1:1829 test.cpp "$loop" "$sleeptime" 5 &) 
          (./submit 127.0.0.1:1829 test.cpp "$loop" "$sleeptime" 9000 > "otpt_$j$i.txt") &
          (timeout $((loop * sleeptime * numclients + 5)) mpstat -P 0,1 10 >> "cpu_utilization$j.txt") &
          (ps -C server -Lf >> "active_threads$j.txt") &
          #f_name="otpt_$j$i.txt"
          #echo "$otpt" > "$f_name"    
    done
    wait
    #sed -i '/UID/d' "active_threads$j.txt"
    sed -i '/CPU/d' "cpu_utilization$j.txt"
     # Calculate average number of active threads from the ps output
    average_active_threads=$(awk '{sum+=$6} END {print sum/(NR+1)}' "active_threads$j.txt")
    echo "$(($numclients)) $average_active_threads" >> "$active_threads_file" 
    rm "active_threads$j.txt"
    # Calculate average CPU utilization from the vmstat output
    average_cpu_utilization=$(awk '{sum+=100-$14} END {print sum/(NR+1)}' "cpu_utilization$j.txt")
    rm "cpu_utilization$j.txt"
    echo "$(($numclients)) $average_cpu_utilization" >> "$cpu_utilization_file"
    # Changing the number of clients for the next iteration by a step size equal to 10
    numclients=$(($numclients+10)) 
    loop=$(($loop+5))  
done

# Kill the server after all iterations
#killall server
for ((j=1; j<=2; j++)); do
th=0
overallThroughput=0
resTime=0
res=0
totalN=0
err=0
terr=0
tmt=0
tt=0
reqrate=0
treqrate=0
for ((i=1; i<=$a; i++)); do
    th=$(grep "Throughput" "otpt_$j$i.txt" | cut -d ':' -f 2)
    overallThroughput=$(echo "$overallThroughput + $th" | bc -l)
    err=$(grep "errors" "otpt_$j$i.txt" | cut -d ':' -f 2)
    terr=$(echo "$terr + $err" | bc -l)
    tmt=$(grep "timeout" "otpt_$j$i.txt" | cut -d ':' -f 2)
    tt=$(echo "$tt + $tmt" | bc -l)
    n=$(grep "Number of successful responses" "otpt_$j$i.txt" | cut -d ':' -f 2)
    res=$(grep "Avg response time in microseconds" "otpt_$j$i.txt" | cut -d ':' -f 2)
    resTime=$(echo "$resTime + ($res * $n)" | bc -l)
    reqrate=$(grep "Request rate sent" "otpt_$j$i.txt" | cut -d ':' -f 2)
    treqrate=$(echo "$treqrate + $reqrate" | bc -l)
    totalN=$((totalN + n))   
done
if [ "$totalN" -eq 0 ]; then
    res=500000
else
    th=$(echo "scale=6; $overallThroughput" | bc -l)
    res=$(echo "scale=6; $resTime / $totalN" | bc -l)
    tmt=$(echo "scale=6; $tt" | bc -l)
    err=$(echo "scale=6; $terr" | bc -l)
    
fi
# Append data to throughput file
echo "$(($a)) $overallThroughput" >> "$throughput_file_v2"
# Append data to response time file
echo "$(($a)) $res" >> "$response_time_file_v2"
# Append data to error.txt
echo "$(($a)) $err" >> "$error_rate_file_v2"
# Append data to tmt.txt
echo "$(($a)) $tmt" >> "$timeout_rate_file_v2"
# Append data to req.txt
echo "$(($a)) $treqrate" >> "$req_rate_file_v2"
a=$((a+10))
done
for ((j=1; j<=2; j++)); do
	for ((i=1; i<=$b; i++)); do
	rm "otpt_$j$i.txt"
done 
b=$((b+9))	
done
# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "Throughput_graph_v2.png"
set title "Number of Clients vs. Throughput"
set xlabel "Number of Clients"
set ylabel "Throughput"
set grid
plot "$throughput_file_v2" using 1:2 with linespoints title "Throughput"
EOF
# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "Response_time_graph_v2.png"
set title "Number of Clients vs. Response Time"
set xlabel "Number of Clients"
set ylabel "Response Time"
set grid
plot "$response_time_file_v2" using 1:2 with linespoints title "Response Time"
EOF
# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "utilization_graph_v2.png"
set title "Number of Clients vs. CPU Utilization"
set xlabel "Number of Clients"
set ylabel "CPU Utilization (%)"
set grid
plot "$cpu_utilization_file_v2" using 1:2 with linespoints title "CPU Utilization"
EOF

# Use Gnuplot to create the response time graph
gnuplot <<EOF
set terminal png
set output "active_threads_graph_v2.png"
set title "Number of Clients vs. Average Active Threads"
set xlabel "Number of Clients"
set ylabel "Average Active Threads"
set grid
plot "$active_threads_file_v2"  using 1:2 with linespoints title "Active Threads"
EOF
# Use Gnuplot to create the error rate graph
gnuplot <<EOF
set terminal png
set output "error_rate_graph_v2.png"
set title "Number of Clients vs. Error Rate"
set xlabel "Number of Clients"
set ylabel "Error Rate"
set grid
plot "$error_rate_file_v2"  using 1:2 with linespoints title "ERROR RATE"
EOF

# Use Gnuplot to create the timeout rate graph
gnuplot <<EOF
set terminal png
set output "timeout_rate_graph_v2.png"
set title "Number of Clients vs. Timeout Rate"
set xlabel "Number of Clients"
set ylabel "Timeout Rate"
set grid
plot "$timeout_rate_file_v2"  using 1:2 with linespoints title "TIMEOUT RATE"
EOF

# Use Gnuplot to create the request sent rate graph
gnuplot <<EOF
set terminal png
set output "req_rate_graph_v2.png"
set title "Number of Clients vs. Request Sent Rate"
set xlabel "Number of Clients"
set ylabel "Request Sent Rate"
set grid
plot "$req_rate_file_v2"  using 1:2 with linespoints title "Request Sent Rate"
EOF
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
############################################################################
######################   version 2 ended	############################
############################################################################
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXxXXXXX
numclients=$1
a=$1
b=$1
loop=$2
sleeptime=$3
g++ server7.cpp -o server
#./server 1528 &
sleep 2
g++ client7.cpp -o submit
# Create a file to store the Throughput and response time values
throughput_file_a="throughput_data_a.txt"
response_time_file_a="response_time_data_a.txt"
echo "Clients Throughput" > "$throughput_file_a"
echo "Clients AverageResponseTime" > "$response_time_file_a"
#for loop  for varying number of clients
for ((j=1; j<=2; j++)); do
for ((i=1; i<=$numclients; i++)); do
    #otpt=$(./submit 127.0.0.1:1528 test.cpp "$loop" "$sleeptime" &)
    (./submit 127.0.0.1:1821 test.cpp "$loop" "$sleeptime" > "otpt_$j$i.txt") &
    #f_name="otpt_$j$i.txt"
    #echo "$otpt" > "$f_name"
done
wait
#changing number of clients for next iteration by a step size equal to 10
numclients=$(($numclients + 10))
done
# Kill the server after all iterations
#killall server


for ((j=1; j<=2; j++)); do
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
    resTime=$(echo "$resTime + ($res * $n)" | bc -l)
    totalN=$((totalN + n))   
done
th=$(echo "scale=6; $overallThroughput" | bc -l)
res=$(echo "scale=6; $resTime / $totalN" | bc -l)
# Append data to throughput file
echo "$(($a)) $th" >> "$throughput_file_a"
# Append data to response time file
echo "$(($a)) $res" >> "$response_time_file_a"
a=$((a + 10))
done
for ((j=1; j<=2; j++)); do
	for ((i=1; i<=$b; i++)); do
	rm "otpt_$j$i.txt"
done 
b=$((b+9))	
done
# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "throughput_graph7_v1.png"
set title "Number of Clients vs. Throughput"
set xlabel "Number of Clients"
set ylabel "Throughput"
set grid
plot "$throughput_file_a" using 1:2 with linespoints title "Throughput"
EOF

# Use Gnuplot to create the response time graph
gnuplot <<EOF
set terminal png
set output "response_time_graph_v1.png"
set title "Number of Clients vs. Average Response Time"
set xlabel "Number of Clients"
set ylabel "Average Response Time(in microseconds)"
set grid
plot "$response_time_file_a" using 1:2 with linespoints title "Average Response Time"
EOF
gnuplot <<EOF
set terminal png
set output "Combined_Throughput_Graph.png"
set title "Number of Clients vs. Throughput"
set xlabel "Number of Clients"
set ylabel "Throughput"
set grid
plot "$throughput_file" using 1:2 with linespoints title "Throughput ver3" linecolor rgb "blue", \
     "$throughput_file_a" using 1:2 with linespoints title "Throughput ver1" linecolor rgb "red", \
     "$throughput_file_v2" using 1:2 with linespoints title "Throughput ver2" linecolor rgb "green"
EOF
gnuplot <<EOF
set terminal png
set output "Combined_Response_time_Graph.png"
set title "Number of Clients vs. Response_Time"
set xlabel "Number of Clients"
set ylabel "Response Time"
set grid
plot "$response_time_file" using 1:2 with linespoints title "Response Time ver3" linecolor rgb "blue", \
     "$response_time_file_a" using 1:2 with linespoints title "Response Time ver1" linecolor rgb "red", \
     "$response_time_file_v2" using 1:2 with linespoints title "Response Time ver2" linecolor rgb "green"
EOF

echo "Graphs created"
