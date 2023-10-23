#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sys/time.h>

using namespace std; 

const int MAX_BUFFER_SIZE = 10000;
int success=0;
 
int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded> <loopNum> <sleepTimeSeconds>" << endl;
        return 1;
    }

    char* serverAddress = strtok(argv[1], ":");
    char* serverPort = strtok(NULL, ":");
    char* sourceCodeFile = argv[2];

    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
     // Connect to the server
    
    
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(serverPort));
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress);

    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Error connecting to server." << endl;
        return 1;
    }
    // Read the source code file
    ifstream file(sourceCodeFile);
    string sourceCode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    
    int lp=atoi(argv[3]);
    int sl=atoi(argv[4]);
    char buffer[MAX_BUFFER_SIZE];
    timeval Tsend,Trecv,diff,sum,ti,te;
    sum.tv_sec=0,sum.tv_usec=0;
    float suc=0;
    gettimeofday(&ti, NULL);
    
    
    for(int i=0;i<lp;i++){
    // Send the source code to the server
    gettimeofday(&Tsend, NULL);
    ssize_t bytesSent = send(clientSocket, sourceCode.c_str(), sourceCode.size(), 0);
    if (bytesSent == -1) {
        cerr << "Error sending data." << endl;
        close(clientSocket);
        return 1;
    }
    
    memset(buffer, 0, sizeof(buffer));
    // Receive result from the server
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
  
    if (bytesRead == -1) {
        cerr << "Error receiving data." << endl;
        close(clientSocket);
        return 1;
    }
    else suc++;
    gettimeofday(&Trecv, NULL);
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
   
    sleep(sl);
 }
    gettimeofday(&te, NULL);
    
    close(clientSocket);
    cout<<"Number of successful responses:"<<suc<<"\ntotal time elapsed between each request and response in microseconds: "<<sum.tv_usec<<"\nAvg response time in microseconds: "<<sum.tv_usec/float(lp)<<"\nThroughput:"<<(suc)*(1000000.0/sum.tv_usec)<<"\ntime taken by loop in seconds: "<<te.tv_sec-ti.tv_sec<<"\n";
    return 0;
}

