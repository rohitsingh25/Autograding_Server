#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>

using namespace std;

// variables for socket 
struct sockaddr_in serverAddr;
int clientSocket;
const int MAX_BUFFER_SIZE = 10000;

// mutex lock
pthread_mutex_t lc;

// variables for analysis purpose
int req=0,timeout=0,errors=0,suc=0;
int polling_interval=1;

int main(int argc, char* argv[]) 
{
    // mutex initialization
    pthread_mutex_init(&lc,NULL);

    if (argc != 4) 
    {
        cerr << "Usage: " << argv[0] << " <new | status> <serverIP:port> <sourceCodeFileTobeGraded | request_id>  " << endl;
        return 1;
    }
    
    // extract type, server ip and port from command line arguments
    string type=argv[1];
    char* serverAddress = strtok(argv[2], ":");
    char* serverPort = strtok(NULL, ":");
    
    // buffer for sending and receiving data
    char buffer[MAX_BUFFER_SIZE];
    timeval Tsend,Trecv,diff,sum,ti,te;
    sum.tv_sec=0,sum.tv_usec=0;
    memset(buffer, 0, sizeof(buffer)); 
    
    // if type is new
    if(type=="new")
    {
        char* sourceCodeFile = argv[3];
        ifstream file(sourceCodeFile);
        string sourceCode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        
        // Create socket
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            cerr << "Error creating socket." << endl;
            return 1;
        }
        // set socket parameters
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(atoi(serverPort));
        serverAddr.sin_addr.s_addr = inet_addr(serverAddress);

        // connect to server
        if ( connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
        {            
            return 0;
        }
        
        // Send the request type NEW to the server
        ssize_t bytesSent = send(clientSocket, "new", sizeof("new"), 0);

        // Receive the reply from the server
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0); 

        // Send the source code to the server
        bytesSent = send(clientSocket, sourceCode.c_str(), sourceCode.size(), 0);
        if (bytesSent == -1 || bytesSent == 0) {
            return 0;
        }
        else req++;  
        memset(buffer, 0, sizeof(buffer));
        
        // Receive requestID from the server
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) 
        {
            return 0;
        } 
        else 
        {
            // save request id in file
            pthread_mutex_lock(&lc);  
            FILE* fd=fopen("req_id.txt","ab");
        	ssize_t bw=fwrite(buffer,1,bytesRead,fd);
        	bw = fwrite("\n", 1, 1, fd);
        	fclose(fd);
            pthread_mutex_unlock(&lc); 
        }
        // output request id on terminal
        cout << "Request ID: "<< buffer << endl;
        sleep(1);
    }
    else
    {
        // if type is status
        char* req_id = argv[3];
        // Create socket
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            cerr << "Error creating socket." << endl;
            return 1;
        }
        // set socket parameters
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(atoi(serverPort));
        serverAddr.sin_addr.s_addr = inet_addr(serverAddress);

        // connect to server
        if ( connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
        {
            errors++;
            return 0;
        }

        // Send the request type STATUS to the server
        while(true)
        {
            // send requests of type status to server
            ssize_t bytesSent = send(clientSocket, "status", sizeof("status"), 0);
            if (bytesSent == -1 || bytesSent == 0) 
            {
                continue;	 
            }
            else break;
        }

        // Receive the reply from the server
        while(true)
        {
            // receive ready to serve message from server
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead == -1 || bytesRead == 0) {
                continue;	           
            }
            else break;
        }

        gettimeofday(&Tsend, NULL);
        
        // Send the request id to the server
        while(true)
        {
            // send request id to server
            ssize_t bytesSent = send(clientSocket, req_id, strlen(req_id), 0);
            if (bytesSent == -1 || bytesSent == 0) {
                    continue;
            }
            else req++;
            
            memset(buffer, 0, sizeof(buffer));
            
            // Receive result from the server
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            
            // if no data is received from server
            if (bytesRead <= 0) 
            {
                errors++;
                close(clientSocket);
                return 0;
            }
            // if DONE message is received from server
            else if (strcmp(buffer, "DONE") == 0) 
            { 
                suc++;
                gettimeofday(&Trecv, NULL);       
                break;
            }
            // if TIMEOUT message is received from server
            else if (strcmp(buffer, "Invalid request ID given") == 0) 
            {	
                cout<<"Invalid request ID given"<<endl;
                break;
            }
            else 
            {
                cout<<buffer<<"ReqID : "<<req_id<<endl;
                break;
            }
        }

        // printing analysis results on terminal of client
        cout << "Autograding Result of RequestID "<<req_id<<" : " << buffer << endl;
        diff.tv_sec = Trecv.tv_sec - Tsend.tv_sec;
        diff.tv_usec=Trecv.tv_usec-Tsend.tv_usec;
        if (diff.tv_usec < 0) 
        {
            diff.tv_sec--;
            diff.tv_usec += 1000000; // 1,000,000 microseconds in a second
        }
        sum.tv_usec+=diff.tv_usec;
        if (sum.tv_usec >= 1000000) {
            sum.tv_sec++;
            sum.tv_usec -= 1000000;
        }
        cout<<"Response time (micro sec) : "<<sum.tv_usec<<endl;
    }

    close(clientSocket);
    
    return 0;
}