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
#include<sys/wait.h>
#include<queue>
using namespace std;

const int MAX_BUFFER_SIZE = 10000;
pthread_mutex_t lck,q,old,lc,counter_lock;
pthread_cond_t cond;
float queue_size=0,done_req=0;

struct ThreadData {
    int clientSocket;
    long long int counter;
};

queue<ThreadData> requestQueue;

map<long long int, string> compile_err;
map<long long int, string> runtime_err;
map<long long int, string> diff_err;
map<long long int, string> pass;
map<long long int, int> in_queue;
map<long long int, int> still_processing;
// map<long lo>



// rohit global variable
long long int counter =0;

bool isOutputCorrect(const string& programOutput) 
{
    string expectedOutput = "1 2 3 4 5 6 7 8 9 10";
    return programOutput == expectedOutput;
}

void addElementFromQueueToFile(int element) 
{
    ofstream file("queue_backup.txt", ios::out | ios::trunc);
    file << element << endl;
    cout<<element<<" pushed in file"<<endl;
    file.close();
}
// Function to remove the first line from the file
void removeRequestIDFromFile(int req_id) 
{
    ifstream inFile("queue_backup.txt");
    if (!inFile.is_open()) 
    {
        cout << "File not found or unable to open!" << endl;
        return;
    }
    string line;
    // Read and discard the first line from the file
    // getline(inFile, line); 
    // for debugging - 1
    // cout<<line<<" poped from file"<<endl;
    ofstream outFile("temp.txt");
    while (getline(inFile, line)) 
    {
        istringstream iss(line);
        int id;
        if (iss >> id && id == req_id)
            continue;
        // cout<<"status of getline : "<<line;
        outFile << line << std::endl;
    }
    inFile.close();
    outFile.close();
    remove("queue_backup.txt");
    rename("temp.txt", "queue_backup.txt");
}

void reinitializeQueueFromFile(string FILE_NAME) 
{
    ifstream file(FILE_NAME);
    if (!file.is_open()) 
    {
        cout << "File not found or unable to open!" << endl;
        return;
    }

    string line;
    while (getline(file, line)) 
    {
        int value;
        istringstream iss(line);
        if (iss >> value) {
            ThreadData td ;
            td.clientSocket=0;
            td.counter=value;
            requestQueue.push(td);
        }
    }
    file.close();
}

void reinitializeQueueFromFilesp(string FILE_NAME) 
{
    ifstream file(FILE_NAME);
    if (!file.is_open()) 
    {
        cout << "File not found or unable to open!" << endl;
        return;
    }
    string line;
    while (getline(file, line)) 
    {
        stringstream ss(line);
        long long int key;
        int value;
        ss >> key;
        char comma;
        ss >> comma; // Read the comma separating key and value
        ss >> value;
        ThreadData td ;
        td.clientSocket=value;
        td.counter=key;
        requestQueue.push(td);
    }
    file.close();
    cout << "Map data loaded from file." << endl;
}

void addEntryToCSVis(long long int key, const string& value , string FILE_NAME) {
    ofstream file(FILE_NAME, ios::app); 
    if (file.is_open()) {
        file << key << "," << value << endl;
        file.close();
    } 
    else 
    {
        cout << "Unable to open file for adding entry." << endl;
    }
}

void loadMapFromFileis(map<long long int , string>&mp,string FILE_NAME) {
    ifstream file(FILE_NAME);
    if (file.is_open()) 
    {
        string line;
        while (getline(file, line)) 
        {
            stringstream ss(line);
            long long key;
            string value;
            getline(ss, line, ',');
            ss >> value;
            key = stoll(line);
            mp[key] = value;
        }
        file.close();
        // cout << "Map data loaded from file." << endl;
    } 
    else 
    {
        cout << "File not found or unable to open." << endl;
    }
}

void addEntryToCSVii(long long int key, int value , string FILE_NAME) {
    ofstream file(FILE_NAME, ios::app); 
    if (file.is_open()) 
    {
        file << key << "," << value << endl;
        file.close();
    } 
    else 
    {
        cout << "Unable to open file for adding entry." << endl;
    }
}

void loadMapFromFileii(map<long long int , int>&mp , string FILE_NAME) {
    ifstream file(FILE_NAME);
    if (file.is_open()) 
    {
        string line;
        while (getline(file, line)) 
        {
            stringstream ss(line);
            long long int key;
            int value;
            ss >> key;
            char comma;
            ss >> comma; // Read the comma separating key and value
            ss >> value;
            mp[key] = value;
        }
        file.close();
        cout << "Map data loaded from file." << endl;
    } 
    else 
    {
        cout << "File not found or unable to open." << endl;
    }
}


void counterReintialize()
{
    ifstream file("counter.txt");
    if (!file.is_open()) 
    {
        // cout << "File not found or unable to open!" << endl;
        return;
    }
    int c = 0;
    string line;
    if (getline(file, line)) {
        istringstream iss(line);
        if (!(iss >> c)) 
        {
            counter = 0;
        }
    }
    file.close();
    cout << "counter loaded from file." << endl;
}

void updateCounter()
{
    ofstream outFile("counter.txt", ofstream::trunc);
    if (!outFile) {
        std::cerr << "Unable to open file!" << std::endl;
        return;
    }
    outFile << counter << endl;
    outFile.close();
}

void reinitializeData()
{
    reinitializeQueueFromFile("queue_backup.txt");
    counterReintialize();

    // loadMapFromFileis(compile_err,"compile_err.csv");
    // loadMapFromFileis(runtime_err,"runtime_err.csv");
    // loadMapFromFileis(diff_err,"diff_err.csv");
    // loadMapFromFileis(pass,"pass.csv");

    // loadMapFromFileii(in_queue,"still_processing.csv");

    cout<<"Data reinitialized"<<endl;
}

void *handlereq_new(ThreadData threadData)
{
    pthread_mutex_lock(&lck);    
    // ThreadData *threadData = static_cast<ThreadData *>(data);
    int clientSocket = (threadData.clientSocket);
    char *buffer = new char[MAX_BUFFER_SIZE];
    pthread_mutex_unlock(&lck);
    if(clientSocket!=0)
    {
        ssize_t bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        
        if (bytesRead == 0 || bytesRead == -1) {
            // Closing the connection with the client when no data is sent by it
            close(clientSocket);
            return nullptr;
        }
        
        string req_id=to_string(threadData.counter); 
        
        ssize_t byteswrite = send(clientSocket, req_id.c_str(), req_id.size(), 0);
        still_processing[threadData.counter]=0;
        addElementFromQueueToFile(threadData.counter);

        close(clientSocket);
        // Generate a unique file name for each thread
    }
    string tempFileName = "student_code_" + to_string(threadData.counter) + ".cpp";
    string res = "res" + to_string(threadData.counter) + ".txt";
        
        // Save the received code to a temporary file with a unique name
    if(clientSocket!=0)
    {   ofstream codeFile(tempFileName);
        codeFile << buffer;
        codeFile.close();
    }
    
    
    // Compile the code
    int compileResult = system(("g++ -o student_program_" + to_string(threadData.counter) + " " + tempFileName + " 2> " + res).c_str());   
    
    if (compileResult != 0) 
    {
        compile_err[threadData.counter]="COMPILER ERROR";
        system(("rm " + tempFileName).c_str());
        // close(clientSocket);
        still_processing.erase(still_processing.find(threadData.counter));
        removeRequestIDFromFile(threadData.counter);
        return NULL;
    }
    
    int executionResult = system(("./student_program_" + to_string(threadData.counter) + " 1>student_output_"+to_string(threadData.counter)+".txt 2> " + res).c_str());
    
    if (executionResult != 0) 
    {
       runtime_err[threadData.counter]="RUNTIME ERROR";
       system(("rm " + tempFileName).c_str());
       system(("rm student_output_" + to_string(threadData.counter) + ".txt").c_str());
       system(("rm student_program_" + to_string(threadData.counter)).c_str());
    //    close(clientSocket);
    //    return NULL;
    } 
    else 
    {
        system(("rm " + tempFileName).c_str());
        
        // Capture the program's output
        ifstream output_file(("student_output_"+to_string(threadData.counter)+".txt"));
        string programOutput((istreambuf_iterator<char>(output_file)), istreambuf_iterator<char>());
        // Check if the output is correct
        bool outputCorrect = isOutputCorrect(programOutput);
        // Send the result back to the client
        if (outputCorrect) 
        {
            pass[threadData.counter]="PASS";
            system(("cp student_output_" + to_string(threadData.counter) + ".txt " + res).c_str());
            system(("rm student_output_" + to_string(threadData.counter) + ".txt").c_str());
            system(("rm student_program_" + to_string(threadData.counter)).c_str());
            close(clientSocket);
            // return nullptr;	
        }
        else 
        {
            // There's an output error, compare it with the expected output
            diff_err[threadData.counter]="OUTPUT ERROR";
            system(("diff student_output_" + to_string(threadData.counter) + ".txt output.txt > " + res).c_str()); 
            system(("rm student_output_" + to_string(threadData.counter) + ".txt").c_str()); 
            system(("rm student_program_" + to_string(threadData.counter)).c_str());
            // close(clientSocket);
            // return nullptr;
        }     
    }
    still_processing.erase(still_processing.find(threadData.counter));
    removeRequestIDFromFile(threadData.counter);
    return nullptr;
}  

void *handlereq_status(int clientSocket)
{
    // int *clientSocket1 = (int*)(cs);
    // int clientSocket = *clientSocket1;
    char *buffer = new char[MAX_BUFFER_SIZE];
    // while(true)
    // {
        ssize_t bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytesRead == 0 || bytesRead == -1) {
            // Closing the connection with the client when no data is sent by it
            close(clientSocket);
            return nullptr;
        }
        buffer[bytesRead]='\0';
        char *endptr;
        long long int convertedValue = strtoll(buffer, &endptr, 10);
        long long int g=convertedValue;
        auto t1=pass.find(g);
        auto t2=compile_err.find(g);
        auto t3=runtime_err.find(g);
        auto t4=diff_err.find(g);
        auto t5=in_queue.find(g);
        auto t6=still_processing.find(g);
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
            return nullptr;
        }
        else if(t5!=in_queue.end()){
            ssize_t bytessent = send(clientSocket, "Request is in queue",sizeof("Request is in queue"), 0);
            return nullptr;
        }
        else
        {
            ssize_t bytessent = send(clientSocket, "Invalid request ID given",sizeof("Invalid request ID given"), 0);
            close(clientSocket);
            return nullptr;
        }
    // }
    return nullptr;
}  

void* workerThread(void* arg) 
{
    while (true) 
    {
        // sleep(1);
        pthread_mutex_lock(&lc);
        while (requestQueue.empty()) 
        {
            // Wait for a grading request in the queue
            pthread_cond_wait(&cond, &lc);
        }
        pthread_mutex_unlock(&lc);
        
        // Get the next grading request from the queue
        ThreadData tdc  = requestQueue.front();
        requestQueue.pop();
        // removeFirstLineFromFile();

        // Process the grading request
        in_queue[counter]= tdc.clientSocket;

        char buf[MAX_BUFFER_SIZE];
        ssize_t bytesRead = recv(tdc.clientSocket, buf, sizeof(buf), 0);
        buf[bytesRead]='\0';
        string type(buf);
        ssize_t byteswrite = send(tdc.clientSocket, "ready to serve", sizeof("ready to serve"), 0);
        // pthread_t thr;
        if (strcmp(buf, "new") == 0) 
        {   
            ThreadData td;
            pthread_mutex_lock(&counter_lock);
            counter++;  
            updateCounter();
            td.counter=counter;
            td.clientSocket=tdc.clientSocket;
            pthread_mutex_unlock(&counter_lock);
            // pthread_create(&thr, nullptr, handlereq_new, td);
            handlereq_new(td);
        }
        else
        { 
            //  pthread_create(&thr, nullptr, handlereq_status, (void *)clientSocket);
            sleep(1);
            handlereq_status(tdc.clientSocket);

        }
    }
}      
int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
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
    reinitializeData();
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
        ThreadData tdc;
        tdc.clientSocket = clientSocket;
        tdc.counter=0;
        pthread_mutex_lock(&q);
        // Enqueue the client socket for grading
        requestQueue.push(tdc);
        // addElementFromQueueToFile(clientSocket);
        // Signal a worker thread to process the request
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&q);    
    }
    return 0;
}
