#include <iostream>
#include<bits/stdc++.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream>
#include <cstdio>
#include <fcntl.h>
#include<pthread.h>
#include<sys/types.h>
#include<queue>
using namespace std;

const int MAX_BUFFER_SIZE = 10000;
pthread_mutex_t lck,q,old,lc,counter_lock;
pthread_cond_t cond;

float queue_size=0,done_req=0;
queue<int> requestQueue;

map<long long int, string> compile_err;
map<long long int, string> runtime_err;
map<long long int, string> diff_err;
map<long long int, string> pass;
map<long long int, int> in_queue;
map<long long int, int> still_processing;


struct ThreadData {
    int clientSocket;
    // char buffer[MAX_BUFFER_SIZE];
    long long int counter =0;
};


// rohit struct
// struct ThreadData {
//     int clientSocket;
//     char buffer[MAX_BUFFER_SIZE];
//     // long long int counter =0;
// };
// rohit global variable
long long int counter =0;


// ThreadData* threadData = new ThreadData;
bool isOutputCorrect(const string& programOutput) {
    string expectedOutput = "1 2 3 4 5 6 7 8 9 10";
    return programOutput == expectedOutput;
}
void *handlereq_new(void *data){
    pthread_mutex_lock(&lck);    
    ThreadData *threadData = static_cast<ThreadData *>(data);
    long long int g=threadData->counter;
    int clientSocket = (threadData->clientSocket);
    // int clientSocket = *clientSocket1;
    // cout<<clientSocket<<endl;
    // cout<<clientSocket<<" serving handle request"<<endl;
    // memset(threadData->buffer, 0, sizeof(threadData->buffer));
    char *buffer = new char[MAX_BUFFER_SIZE];
    // Receive source code from the client
    pthread_mutex_unlock(&lck);
    ssize_t bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
    
    if (bytesRead == 0 || bytesRead == -1) {
        // Closing the connection with the client when no data is sent by it
        // pthread_mutex_lock(&lck);
        close(clientSocket);
        // pthread_mutex_unlock(&lck);
        return nullptr;
    }
    string req_id=to_string(g); 
    still_processing[g]=clientSocket;
    ssize_t byteswrite = send(clientSocket, req_id.c_str(), req_id.size(), 0);
    // Generate a unique file name for each thread
    string tempFileName = "student_code_" + to_string(g) + ".cpp";
    string res = "res" + to_string(g) + ".txt";
    // Save the received code to a temporary file with a unique name
    //pthread_mutex_unlock(&lck);
    ofstream codeFile(tempFileName);
    codeFile << buffer;
    codeFile.close();
    // Compile the code
    int compileResult = system(("g++ -o student_program_" + to_string(g) + " " + tempFileName + " 2> " + res).c_str());   
    if (compileResult != 0) {
        compile_err[g]="COMPILER ERROR";
        system(("rm " + tempFileName).c_str());
        close(clientSocket);
        return NULL;
    }
    int executionResult = system(("./student_program_" + to_string(g) + " 1>student_output_"+to_string(g)+".txt 2> " + res).c_str());
    if (executionResult != 0) {
       runtime_err[g]="RUNTIME ERROR";
       system(("rm " + tempFileName).c_str());
       system(("rm student_output_" + to_string(g) + ".txt").c_str());
       system(("rm student_program_" + to_string(g)).c_str());
       close(clientSocket);
       return NULL;
        
    } 
    else 
    {
        system(("rm " + tempFileName).c_str());
        
        // Capture the program's output
        ifstream output_file(("student_output_"+to_string(g)+".txt"));
        string programOutput((istreambuf_iterator<char>(output_file)), istreambuf_iterator<char>());
        // Check if the output is correct
        bool outputCorrect = isOutputCorrect(programOutput);
        // Send the result back to the client
        if (outputCorrect) 
        {
            pass[g]="PASS";
            system(("cp student_output_" + to_string(g) + ".txt " + res).c_str());
            system(("rm student_output_" + to_string(g) + ".txt").c_str());
            system(("rm student_program_" + to_string(g)).c_str());
            close(clientSocket);
            return NULL;;	
        }
        else 
        {
            // There's an output error, compare it with the expected output
            diff_err[g]="OUTPUT ERROR";
            system(("diff student_output_" + to_string(g) + ".txt output.txt > " + res).c_str()); 
            system(("rm student_output_" + to_string(g) + ".txt").c_str()); 
            system(("rm student_program_" + to_string(g)).c_str());
            close(clientSocket);
            return nullptr;
        }
       
       
        
    }
    return nullptr;
   
}  

void *handlereq_status(void *cs){
    // pthread_mutex_lock(&lck);
    // ThreadData *threadData = static_cast<ThreadData *>(data);
    // int clientSocket = (int)cs;

    int *clientSocket1 = (int*)(cs);
    int clientSocket = *clientSocket1;
    // memset(threadData->buffer, 0, sizeof(threadData->buffer));
    // cout<<clientSocket<<" serving status request"<<endl;
    char *buffer = new char[MAX_BUFFER_SIZE];
    // Receiving the request ID from the client

    while(true)
    {
        ssize_t bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        // cout<<"checking buffer in loop : "<<buffer<<" value of buffer"<<endl;
        // pthread_mutex_unlock(&lck);
        if (bytesRead == 0 || bytesRead == -1) {
            // Closing the connection with the client when no data is sent by it
            // pthread_mutex_lock(&lck);
            close(clientSocket);
            // pthread_mutex_unlock(&lck);
            return nullptr;
        }
        buffer[bytesRead]='\0';
        char *endptr;
        long long int convertedValue = strtoll(buffer, &endptr, 10);
        long long int g=convertedValue;
        auto t1=pass.find(g);
        // cout<<"t1  "<<(t1==pass.end())<<endl;
        auto t2=compile_err.find(g);
        // cout<<"t1"<<(t1==pass.end())<<endl;
        // cout<<"t2  "<<(t2==compile_err.end())<<endl;
        auto t3=runtime_err.find(g);
        // cout<<"t1"<<(t1==pass.end())<<endl;
        // cout<<"t3  "<<(t3==runtime_err.end())<<endl;
        auto t4=diff_err.find(g);
        // cout<<"t1"<<(t1==pass.end())<<endl;
        // cout<<"t4  "<<(t4==diff_err.end())<<endl;
        auto t5=in_queue.find(g);
        // cout<<"t1"<<(t1==pass.end())<<endl;
        // cout<<"t5  "<<(t5==in_queue.end())<<endl;
        auto t6=still_processing.find(g);
        // cout<<"t6  "<<(t6==still_processing.end())<<endl;
        // cout<<"t1"<<(t1==pass.end())<<endl;  
        if(t1!=pass.end() || t2!=compile_err.end() || t3!=runtime_err.end() || t4!=diff_err.end()) 
        {
            char buf[MAX_BUFFER_SIZE];
            ssize_t bytessent = send(clientSocket, "DONE",sizeof("DONE"), 0);
            bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
            if(t1!=pass.end()) bytessent = send(clientSocket, pass[g].c_str(),sizeof(pass[g]), 0);       
            
            else if(t2!=compile_err.end()) bytessent = send(clientSocket, compile_err[g].c_str(),sizeof(compile_err[g]), 0);       
        
            else if(t3!=runtime_err.end()) bytessent = send(clientSocket, runtime_err[g].c_str(),sizeof(runtime_err[g]), 0);       
            
            else if(t4!=diff_err.end()) bytessent = send(clientSocket, diff_err[g].c_str(),sizeof(diff_err[g]), 0);       
        
            close(clientSocket);
        
            return nullptr;
        }
        else if(t6!=still_processing.end())
        {
            ssize_t bytessent = send(clientSocket, "Request is still being processed",sizeof("Request is still being processed"), 0); 
            // return nullptr;   
        }
        else if(t5!=in_queue.end()){
            ssize_t bytessent = send(clientSocket, "Request is in queue",sizeof("Request is in queue"), 0);
            // return nullptr;
        }
        else
        {
            ssize_t bytessent = send(clientSocket, "Invalid request ID given",sizeof("Invalid request ID given"), 0);
            close(clientSocket);
            return nullptr;
        }
        // Unlock the mutex and return
        //pthread_mutex_unlock(&lck);
    }
    return nullptr;
}  

void* workerThread(void* arg) {
    while (true) 
    {
      //  pthread_mutex_lock(&lck);
        while (requestQueue.empty()) {
            // Wait for a grading request in the queue
            pthread_cond_wait(&cond, &lc);
        }
        // Get the next grading request from the queue
        //ssize_t rqsz=requestQueue.size();
        int *clientSocket  = new int;
        // int clientSocket = requestQueue.front();
        *clientSocket = requestQueue.front();
        cout<<*clientSocket<<" poped from queue"<<endl;
        requestQueue.pop();
        //cout<<queue_size/counter;
        
        // Process the grading request
        
        // threadData->clientSocket = clientSocket;
        // memset(threadData->buffer, 0, sizeof(threadData->buffer));
        //threadData->counter=threadData->counter+1;
        in_queue[counter]=*clientSocket;
        // cout<<*clientSocket<<" in queue"<<endl<<counter<<" value of counter"<<endl;
        //pthread_mutex_unlock(&lck);

        char buf[MAX_BUFFER_SIZE];
        ssize_t bytesRead = recv(*clientSocket, buf, sizeof(buf), 0);
        
        buf[bytesRead]='\0';
        string type(buf);
        // cout<<buf<<" value of buf"<<endl;
        ssize_t byteswrite = send(*clientSocket, "ready to serve", sizeof("ready to serve"), 0);
        // string sst=to_string(counter);
        // ssize_t byteswrite = send(*clientSocket, sst.c_str(), sizeof(sst), 0); 
        pthread_t thr;
        if (strcmp(buf, "new") == 0) 

        {   ThreadData * td = new ThreadData;
            pthread_mutex_lock(&counter_lock);
            counter++;  
            td->counter=counter;
            td->clientSocket=*clientSocket;
            pthread_mutex_unlock(&counter_lock);

            // cout<<counter<<" value after incrementing"<<endl;
            pthread_create(&thr, nullptr, handlereq_new, td);
        }
        else
        { 
        // threadData->counter=threadData->counter+1;
         pthread_create(&thr, nullptr, handlereq_status, (void *)clientSocket);
        }
        
        //pthread_join(thr,nullptr);
    }
}      
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <port>" << " <number_of_threads> " << endl;
        return 1;
    }

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Error binding socket." << endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1000) == -1) {
        cerr << "Error listening for connections." << endl;
        return 1;
    }

    // creating file for comparison
     cout << "Server listening on port " << atoi(argv[1]) << "..." << endl;

     // for checking output
     int xd=open("abc.txt",O_WRONLY | O_TRUNC | O_CREAT, 0644);
     int fd=open("output.txt",O_WRONLY | O_TRUNC | O_CREAT, 0644);
     string x="1 2 3 4 5 6 7 8 9 10";
     const char *y = x.c_str();
     int z=write(fd,y,x.size());
     if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    // Initialize mutex and condition variable objects
    pthread_mutex_init(&lck, nullptr);
    pthread_cond_init(&cond, nullptr);
    // mutex for queue
    pthread_mutex_init(&q, nullptr);

    pthread_mutex_init(&old, nullptr);
    pthread_mutex_init(&lc, nullptr);
    pthread_mutex_init(&counter_lock, nullptr);

    // Get the number of worker threads from the command line
    int thread_pool_size = atoi(argv[2]);
    //make pool of threads
    pthread_t workerThreads[thread_pool_size];
    
    // Create worker threads
    for (int i = 0; i < thread_pool_size; i++) {
        pthread_create(&workerThreads[i], nullptr, workerThread, nullptr);
    }

    // Accept incoming connections and assign them to a worker thread
    while (true) 
    {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Error accepting connection." << endl;
            continue;
        }
       
        

        pthread_mutex_lock(&q);

        // Enqueue the client socket for grading
        cout<<clientSocket<<" pushed in queue"<<endl;
        requestQueue.push(clientSocket);
        ssize_t rqsz=requestQueue.size();
        //cout<<rqsz<<endl;
        queue_size+=rqsz;
        // Signal a worker thread to process the request

        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&q);
        
        
      }

        //for(int i=0;i<thread_pool_size;i++){
         // pthread_join(thr[i],NULL);}
     //close(clientSocket);
     //close(serverSocket);
    return 0;
}
