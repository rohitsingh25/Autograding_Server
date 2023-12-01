#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/time.h>
#include <thread>
#include<fcntl.h>

using namespace std; 

// socket variables
struct sockaddr_in serverAddr;
int clientSocket;

// max buffer size
const int MAX_BUFFER_SIZE = 10000;

// Analysis variables
int req=0,timeout=0,errors=0,suc=0;


int main(int argc, char* argv[]) 
{
    if (argc != 6) 
    {
        cerr << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded> <loopNum> <sleepTimeSeconds> <timeoutseconds> " << endl;
        return 1;
    }
    
    // Extracting values
    char* serverAddress = strtok(argv[1], ":");
    char* serverPort = strtok(NULL, ":");
    char* sourceCodeFile = argv[2];
    int lp=atoi(argv[3]);
    int sl=atoi(argv[4]);
    
    struct timeval ntimeout;
    int r=atoi(argv[5]);
    ntimeout.tv_sec = r;
    ntimeout.tv_usec = 0;
    
    //  code file
    ifstream file(sourceCodeFile);
    string sourceCode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    char buffer[MAX_BUFFER_SIZE];
    
    timeval Tsend,Trecv,diff,sum,ti,te;
    sum.tv_sec=0,sum.tv_usec=0;
    memset(buffer, 0, sizeof(buffer));
    
    gettimeofday(&ti, NULL); 
    
    for(int i=0;i<lp;i++)
    {
        // socket 
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            cerr << "Error creating socket." << endl;
            return 1;
        }

        // Assign parameters
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(atoi(serverPort));
        serverAddr.sin_addr.s_addr = inet_addr(serverAddress);
    
        // timeout
        int timeoutSeconds = atoi(argv[5]);

        // connect socket
        if ( connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
        {
            errors++;
            cout << "Number of successful responses:"<<0<< endl;
            cout << "Total time elapsed between each request and response in microseconds:" << timeoutSeconds * 1000000 << endl;
            cout << "Avg response time in microseconds: " << timeoutSeconds * 1000000 << endl;
            cout << "Throughput:"<<suc<< endl;
            cout << "Time taken by loop in seconds: " << timeoutSeconds * 1000000 << endl;
            cout << "errors:" << errors << endl;
            cout << "timeout:" << 0<< endl;
            cout << "Request rate sent:"<<req<<endl;
            cout << "Successful request rate(Goodput):"<<suc<<endl;
            return 0;
        }
        
        gettimeofday(&Tsend, NULL);

        // Send the source code to the server
        ssize_t bytesSent = send(clientSocket, sourceCode.c_str(), sourceCode.size(), 0);
        
        if (bytesSent == -1 || bytesSent == 0) {
            errors++;
            continue;
        }
        else req++;  
        
        memset(buffer, 0, sizeof(buffer));
        
        // flags for readSet
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);
    
        // Receive result from the server
        int selectResult = select(clientSocket + 1, &readSet, NULL, NULL, &ntimeout);
        if (selectResult == 0) 
        {
            // timeout count increment
            timeout++;
        } 
        else 
        {
            // Data is ready to be received
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead < 0) 
            {
                errors++;
            } 
            else if (bytesRead == 0) 
            {
                cout<<"connection closed"<<endl;
            }   
            else 
            {
                gettimeofday(&Trecv, NULL);
                suc++;
            }
	    }
        
        // printing result
        cout << "Autograding Result: " << buffer << endl;
        diff.tv_sec = Trecv.tv_sec - Tsend.tv_sec;
        diff.tv_usec=Trecv.tv_usec-Tsend.tv_usec;
        if (diff.tv_usec < 0) 
        {
            diff.tv_sec--;
            diff.tv_usec += 1000000; // 1,000,000 microseconds in a second
        }
        sum.tv_usec+=diff.tv_usec;
        if (sum.tv_usec >= 1000000) 
        {
            sum.tv_sec++;
            sum.tv_usec -= 1000000;
        }
        sleep(sl);
    }  

    // time of loop calculation
    gettimeofday(&te, NULL);
    timeval x;
    x.tv_usec=te.tv_usec-ti.tv_usec;
    x.tv_sec=te.tv_sec-ti.tv_sec;
    if (x.tv_usec < 0) 
    {
        x.tv_sec--;
        x.tv_usec += 1000000; 
    }
   
    close(clientSocket);
    
    //printing the Analysis 
    int q=te.tv_sec-ti.tv_sec;
    cout<<"Number of successful responses:"<<suc<<endl;
    cout<<"total time elapsed between each request and response in microseconds:"<<sum.tv_usec<<endl;
    cout<<"Avg response time in microseconds:"<<sum.tv_usec/float(lp)<<endl;
    cout<<"Throughput: "<<suc*(1000000.0/((sum.tv_usec)))<<endl;
    cout<<"time taken by loop in seconds:"<<te.tv_sec-ti.tv_sec<<endl;
    cout<<"errors:"<<errors<<endl;
    cout<<"timeout:"<<timeout<<endl;
    cout<<"Request rate sent:"<<lp*(1000000.0/(sum.tv_usec))<<endl;
    cout<<"Successful request rate(Goodput):"<<req*(1000000.0/(sum.tv_usec))<<endl;
    
    return 0;
}
