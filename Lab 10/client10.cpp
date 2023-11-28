#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/time.h>
#include <pthread.h>
#include<fcntl.h>

using namespace std; 
struct sockaddr_in serverAddr;
int clientSocket;
const int MAX_BUFFER_SIZE = 10000;
pthread_mutex_t lc;
int req=0,timeout=0,errors=0,suc=0;
int polling_interval=1;
int main(int argc, char* argv[]) {
    pthread_mutex_init(&lc,NULL);
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <new | old> <serverIP:port> <sourceCodeFileTobeGraded | request_id>  " << endl;
        return 1;
    }
    string type=argv[1];
    char* serverAddress = strtok(argv[2], ":");
    char* serverPort = strtok(NULL, ":");
    
    char buffer[MAX_BUFFER_SIZE];
    timeval Tsend,Trecv,diff,sum,ti,te;
    sum.tv_sec=0,sum.tv_usec=0;
    memset(buffer, 0, sizeof(buffer)); 
    
    if(type=="new")
    
 {
    char* sourceCodeFile = argv[3];

    ifstream file(sourceCodeFile);
    string sourceCode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Error creating socket." << endl;
        return 1;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(serverPort));
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress);
           if ( connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
                
                return 0;
            }
    // Send the source code to the server
    ssize_t bytesSent = send(clientSocket, "new", sizeof("new"), 0);
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    bytesSent = send(clientSocket, sourceCode.c_str(), sourceCode.size(), 0);
    if (bytesSent == -1 || bytesSent == 0) {
                return 0;
    }
    else req++;  
    memset(buffer, 0, sizeof(buffer));
    // Receive result from the server
    bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
                return 0;
	    } 
    else {
	   pthread_mutex_lock(&lc);  
	   FILE* fd=fopen("req_id.txt","ab");
    	   ssize_t bw=fwrite(buffer,1,bytesRead,fd);
    	   bw = fwrite("\n", 1, 1, fd);
    	   fclose(fd);
           pthread_mutex_unlock(&lc); 
	 }
    cout << "Request ID: " << buffer << endl;
    close(clientSocket);
    return 0;
}



else
{
    
    char* req_id = argv[3];
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Error creating socket." << endl;
        return 1;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(serverPort));
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress);
           if ( connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
                errors++;
                cout << "Number of successful responses:"<<0<< endl;
		cout << "Total time elapsed between each request and response in microseconds:" << 150000000 << endl;
		cout << "Avg response time in microseconds: " << 15000000 << endl;
		cout << "Throughput:"<<suc<< endl;
		cout << "errors:" << errors << endl;

		cout << "Successful request rate(Goodput):"<<suc<<endl;
                return 0;
            }
    // Send the source code to the server
    while(true){
    ssize_t bytesSent = send(clientSocket, "old", sizeof("old"), 0);
    if (bytesSent == -1 || bytesSent == 0) {
        continue;	
                
    }
    else break;
    }
    while(true){
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1 || bytesRead == 0) {
        continue;	
                
    }
    else break;
    }
    gettimeofday(&Tsend, NULL);
    while(true){
    ssize_t bytesSent = send(clientSocket, req_id, strlen(req_id), 0);
    if (bytesSent == -1 || bytesSent == 0) {
        	continue;
    }
    else req++;
      
    memset(buffer, 0, sizeof(buffer));
    // Receive result from the server
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
		errors++;
		cout << "Number of successful responses:"<<0<< endl;
		cout << "Total time elapsed between each request and response in microseconds:" << 15000000 << endl;
		cout << "Avg response time in microseconds: " << 15000000 << endl;
		cout << "Throughput:"<<suc<< endl;
		cout << "errors:" << errors << endl;

		cout << "Successful request rate(Goodput):"<<suc<<endl;
                return 0;
	    }
    else if (strcmp(buffer, "DONE") == 0) { 
   	 suc++;
         bytesSent = send(clientSocket, "please send the result", sizeof("please send the result"), 0);
   	 bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);  
   	 gettimeofday(&Trecv, NULL);       
    	 break;
     }
     
    else if (strcmp(buffer, "Invalid request ID given") == 0) 
	    {	cout<<"Invalid request ID given\n";
    		break;
    	    }
    else  sleep(polling_interval);
    }
    cout << "Autograding Result: " << buffer << endl;
    diff.tv_sec = Trecv.tv_sec - Tsend.tv_sec;
    diff.tv_usec=Trecv.tv_usec-Tsend.tv_usec;
    if (diff.tv_usec < 0) {
        diff.tv_sec--;
        diff.tv_usec += 1000000; // 1,000,000 microseconds in a second
    }
    
    sum.tv_usec+=diff.tv_usec;
    if (sum.tv_usec >= 1000000) {
        sum.tv_sec++;
        sum.tv_usec -= 1000000;
    }
   
    
   
    close(clientSocket);
    cout<<"Number of successful responses:"<<suc<<endl;
    cout<<"total time elapsed between each request and response in microseconds:"<<sum.tv_usec<<endl;
    cout<<"Avg response time in microseconds:"<<sum.tv_usec<<endl;
    cout<<"Throughput: "<<suc*(1000000.0/((sum.tv_usec)))<<endl;
    cout<<"errors:"<<errors<<endl;

    cout<<"Successful request rate(Goodput):"<<req*(1000000.0/(sum.tv_usec))<<endl;
    return 0;
}
}

