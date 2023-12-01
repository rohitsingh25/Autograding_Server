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
#include<sstream>

using namespace std;

// max buffer size
const int MAX_BUFFER_SIZE = 10000;
char buffer[MAX_BUFFER_SIZE];

// mutex locks
pthread_mutex_t lck;

// global variable
int counter=0;

// structure of sockfd and buffer
struct ThreadData {
    int clientSocket;
    char buffer[MAX_BUFFER_SIZE];
};

// function to check output is correct or not
bool isOutputCorrect(const string& programOutput) {
    string expectedOutput = "1 2 3 4 5 6 7 8 9 10";
    return programOutput == expectedOutput;
}

// worker thread called for execution 
void *handlereq(void *data)
{
    // counter incremented
    pthread_mutex_lock(&lck);
    counter++; 
    int g=counter;
    pthread_mutex_unlock(&lck);

    // getting threadData from arguments
    ThreadData *threadData = static_cast<ThreadData *>(data);
    int clientSocket = threadData->clientSocket;
    memset(threadData->buffer, 0, sizeof(threadData->buffer));
    char *buffer = threadData->buffer;

    // Receive source code from the client
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(threadData->buffer), 0);
  
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
        // Compilation failed, send the error message
        ifstream errorFile(compileErrorFileName);
        string compileError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
        errorFile.close();
        string response = "COMPILER ERROR\n" + compileError;  
        
        // sending error message
        send(clientSocket, response.c_str(), response.size(), 0);
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
        // Runtime error, send the error message back    
        ifstream err(("runtime_error_"+to_string(g)+".txt").c_str());
        string runtimeError((istreambuf_iterator<char>(err)), istreambuf_iterator<char>());
        err.close(); 
        string response = "RUNTIME ERROR\n" + runtimeError;

        // sending error message
        send(clientSocket, response.c_str(), response.size(), 0);
        system(("rm runtime_error_"+to_string(g)+ ".txt").c_str());
        system(("rm student_output_"+to_string(g)+ ".txt").c_str());
        system(("rm student_code_" + to_string(g) + ".cpp").c_str());
        system(("rm compile_error_" + to_string(g) + ".txt").c_str());  
        close(clientSocket); 
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

            // send pass message to client
            send(clientSocket, response.c_str(), response.size(), 0);  	
            system(("rm runtime_error_"+to_string(g)+ ".txt").c_str());
            system(("rm student_output_"+to_string(g)+ ".txt").c_str());
            system(("rm student_code_" + to_string(g) + ".cpp").c_str());
            system(("rm compile_error_" + to_string(g) + ".txt").c_str());              
            close(clientSocket);
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
            
            // sending output error message
            send(clientSocket, response.c_str(), response.size(), 0); 
            system(("rm runtime_error_"+to_string(g)+ ".txt").c_str());
            system(("rm student_output_"+to_string(g)+ ".txt").c_str());
            system(("rm output_error_"+to_string(g)+ ".txt").c_str());
            system(("rm student_code_" + to_string(g) + ".cpp").c_str());
            system(("rm compile_error_" + to_string(g) + ".txt").c_str()); 
            close(clientSocket);   
            return nullptr;
        }
    }
    return nullptr;
}  

int main(int argc, char* argv[]) 
{
    if (argc != 2) 
    {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
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
    
    // initialize mutex
    pthread_mutex_init(&lck, nullptr);
    
    // create output file for matching
    int fd=open("output.txt",O_WRONLY | O_TRUNC | O_CREAT, 0644);
    string x="1 2 3 4 5 6 7 8 9 10";
    const char *y = x.c_str();
    int z=write(fd,y,x.size());
    if (close(fd) == -1) {
       perror("close");
       return 1;
    }

    // Accept connections
    while(true)
    {
        // Accept incoming connection
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Error accepting connection." << endl;
            continue;
        }

        // structure of threadData
        ThreadData *threadData = new ThreadData;
        threadData->clientSocket = clientSocket;
        memset(threadData->buffer, 0, sizeof(threadData->buffer));
        
        // creating a thread
        pthread_t thr;

        // making this thread to execute handlereq and pass threadData as arguments
        pthread_create(&thr,NULL,handlereq,threadData); 

        // finally detach thread
        pthread_detach(thr);
    }
    return 0;
}
