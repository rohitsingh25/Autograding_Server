if [ $# -ne 4 ]; then
    echo "Usage <number of clients> <loop num> <sleep time> <timeout>"
    exit
fi
CPU_AFFINITY="0,1"
g++ -o client client9.cpp
mkdir -p outputs
rm -f outputs/*

for (( i=1 ; i<=$1 ; i++ )); 
do
    ./client 127.0.0.1:3003 test.cpp $2 $3 $4 > outputs/op$i.txt &
    timeout 10 mpstat -P 0,1 5 > outputs/cu.txt &
done
wait
for (( i=1 ; i<=$1 ; i++ )); 
do
	sed -i '/CPU/d' outputs/cu.txt > /dev/null
done
cat outputs/op*.txt | awk '
    BEGIN{
    	FS=":";
        sum=0;
        total=0;
        thru=0;
        th=0;
        k=0;
        service_time=0;
        serv=0;
    }
    
    {
    	if($1 ~ /Throughput/ ){
    		thru=thru+$2;
    		k=k+1;
    	}
    	if($1 ~ /total time elapsed between each request and response in microseconds/ ){
    		ti = $2;
    	}
    	if($1 ~ /Avg response time in microseconds/ ){
    		avg = $2;
    	}
        sum=sum+(ti*avg)
        total=total+ti;
        
        th=th+thru;
      
    }

    END{
        printf("Average time taken = %f microseconds.\nThroughput = %f\n", sum/total, th/k)
    }'
cat outputs/cu.txt | awk ' 
	BEGIN{
    		FS=" ";
    		sum=0;
    		k=0;
	} 
	{	
         sum+=$14;
         k+=1;
        }
        END{
        printf("CPU utilization =%f\n",100-sum/k)   ;     
}'
