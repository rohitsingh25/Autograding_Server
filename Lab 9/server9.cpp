#include <iostream>
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

// max buffer size
const int MAX_BUFFER_SIZE = 10000;

// mutex and conditional variables
pthread_mutex_t lck,lc;
pthread_cond_t cond;

// global variable
int counter =0;

// Analysis variables
float queue_size=0,done_req=0;

// Queue for threads
queue<int> requestQueue;

// structure of threadData 
struct ThreadData {
    int clientSocket;
    char buffer[MAX_BUFFER_SIZE];
};

// function for checking if output is correct or not
bool isOutputCorrect(const string& programOutput) {
    string expectedOutput = "1 2 3 4 5 6 7 8 9 10";
    return programOutput == expectedOutput;
}

//worker thread
void *handlereq(int clientSocket)
{
    // incrementing global variable
    pthread_mutex_lock(&lck);
    counter++; 
    int g=counter;
    pthread_mutex_unlock(&lck);

    // extracting data from argument
    // ThreadData *threadData = static_cast<ThreadData *>(data);
    // int clientSocket = threadData->clientSocket;
    // memset(threadData->buffer, 0, sizeof(threadData->buffer));
    // char *buffer = threadData->buffer;
    char buffer[MAX_BUFFER_SIZE];
    // Receive source code from the client
    ssize_t bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
    if (bytesRead == 0 || bytesRead == -1) 
    {
        close(clientSocket);
        return nullptr;
    }
    
    // Generate a unique file name for each thread
    string tempFileName = "student_code_" + to_string(g) + ".cpp";
    string compileErrorFileName = "compile_error_" + to_string(g) + ".txt";

    // Save the received code to a temporary file with a unique name
    ofstream codeFile(tempFileName);
    codeFile << buffer;
    codeFile.close();
    
    // Compile the code
    int compileResult = system(("g++ -o student_program_" + to_string(g) + " " + tempFileName + " 2> " + compileErrorFileName).c_str());
   
    if (compileResult != 0) 
    {
        // Compilation failed, send the error message back
        ifstream errorFile(compileErrorFileName);
        string compileError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
        errorFile.close();
        string response = "COMPILER ERROR\n" + compileError;
        
        // send error message
        send(clientSocket, response.c_str(), response.size(), 0);
        done_req++;
        system(("echo "+to_string(queue_size/done_req)+" >>abc.txt").c_str());
        close(clientSocket);
        system(("rm student_code_" + to_string(g) + ".cpp").c_str());
        system(("rm compile_error_" + to_string(g) + ".txt").c_str());  
        return nullptr;
    }

    // Execute the code
    int executionResult = system(("./student_program_" + to_string(g) + " 1>student_output_"+to_string(g)+".txt 2>runtime_error_"+to_string(g)+".txt").c_str());
    
    // Remove the temporary executable
    if (remove(("student_program_" + to_string(g)).c_str()) != 0) {
        cerr << "Error deleting temporary executable file." << endl;
    }
    if (executionResult != 0) 
    {
        // Runtime error, send the error message 
        ifstream err(("runtime_error_"+to_string(g)+".txt").c_str());
        string runtimeError((istreambuf_iterator<char>(err)), istreambuf_iterator<char>());
        err.close();
        string response = "RUNTIME ERROR\n" + runtimeError;

        // send error message
        send(clientSocket, response.c_str(), response.size(), 0);
        done_req++;
        system(("echo "+to_string(queue_size/done_req)+" >>abc.txt").c_str());
        close(clientSocket);
        system(("rm runtime_error_"+to_string(g)+ ".txt").c_str());
        system(("rm student_output_"+to_string(g)+ ".txt").c_str());
        system(("rm student_code_" + to_string(g) + ".cpp").c_str());
        system(("rm compile_error_" + to_string(g) + ".txt").c_str());   
        return nullptr;
    } 
    else 
    {
        // Capture the program's output
        ifstream output_file(("student_output_"+to_string(g)+".txt"));
        string programOutput((istreambuf_iterator<char>(output_file)), istreambuf_iterator<char>());
        
        // Check if the output is correct
        bool outputCorrect = isOutputCorrect(programOutput);
        
        // Send the result back to the client
        if (outputCorrect) 
        {
            string response = "PASS";
            
            // send pass message
            send(clientSocket, response.c_str(), response.size(), 0);
            done_req++;
            system(("echo "+to_string(queue_size/done_req)+" >>abc.txt").c_str());
            close(clientSocket);	
            system(("rm runtime_error_"+to_string(g)+ ".txt").c_str());
            system(("rm student_output_"+to_string(g)+ ".txt").c_str());
            system(("rm student_code_" + to_string(g) + ".cpp").c_str());
            system(("rm compile_error_" + to_string(g) + ".txt").c_str());               
            return nullptr;	
        }
        else 
        {
            // There's an output error, compare it with the expected output
            system(("diff student_output_" + to_string(g) + ".txt output.txt > output_error_" + to_string(g) + ".txt").c_str());
            ifstream err(("output_error_"+to_string(g)+".txt").c_str());
            string OutErr((istreambuf_iterator<char>(err)), istreambuf_iterator<char>());
            err.close();
            string response = "OUTPUT ERROR\n" + OutErr;
            
            // send output error message
            send(clientSocket, response.c_str(), response.size(), 0);
            done_req++;
            system(("echo "+to_string(queue_size/done_req)+" >>abc.txt").c_str());
            close(clientSocket);
            system(("rm runtime_error_"+to_string(g)+ ".txt").c_str());
            system(("rm student_output_"+to_string(g)+ ".txt").c_str());
            system(("rm output_error_"+to_string(g)+ ".txt").c_str());
            system(("rm student_code_" + to_string(g) + ".cpp").c_str());
            system(("rm compile_error_" + to_string(g) + ".txt").c_str());    
            return nullptr;
        }
    }
    return nullptr;
}  
void* workerThread(void* arg) 
{
    while (true) 
    {
        pthread_mutex_lock(&lc);
        while (requestQueue.empty()) 
        {
            // Wait for a grading request in the queue
            pthread_cond_wait(&cond, &lc);
        }
        pthread_mutex_unlock(&lc);

        // Get the next grading request from the queue
        int clientSocket = requestQueue.front();
        requestQueue.pop();
        
        // call handle request function for further processing
        handlereq(clientSocket);
    }
}      
int main(int argc, char* argv[]) 
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <port>" << " <number_of_threads> " << endl;
        return 1;
    }

    // socket variables
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    // assign parameters
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

     cout << "Server listening on port " << atoi(argv[1]) << "..." << endl;

     //creating output file
     int xd=open("abc.txt",O_WRONLY | O_TRUNC | O_CREAT, 0644);
     int fd=open("output.txt",O_WRONLY | O_TRUNC | O_CREAT, 0644);
     string x="1 2 3 4 5 6 7 8 9 10";
     const char *y = x.c_str();
     int z=write(fd,y,x.size());
     if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    // intializing mutex & conditional variables
    pthread_mutex_init(&lck, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_mutex_init(&lc, nullptr);

    // getting thraed pool size
    int thread_pool_size = atoi(argv[2]);

    // creating thread pool
    pthread_t workerThreads[thread_pool_size];
    
    // Create worker threads
    for (int i = 0; i < thread_pool_size; i++) {
        pthread_create(&workerThreads[i], nullptr, workerThread, nullptr);
    }

    // Scheduling requests
    while (true) 
    {
        // accept connection 
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Error accepting connection." << endl;
            continue;
        }

        // Enqueue the client socket for grading
        requestQueue.push(clientSocket);
        ssize_t rqsz=requestQueue.size();
        queue_size+=rqsz;

        pthread_mutex_lock(&lck);
        // Signal a worker thread to process the request
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lck);
    } 
    return 0;
}
