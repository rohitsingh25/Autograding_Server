#include "declarations.h"

// buffer size 
const int MAX_BUFFER_SIZE = 10000;

// mutex and condition variable
pthread_mutex_t lc,queue_lock,counter_lock , map_lock , qfile_lock;
pthread_cond_t cond;

// variables for analysis purpose
float queue_size=0,done_req=0;

// structure for thread data which includes client socket and request id
// struct ThreadData {
//     int clientSocket;
//     long long int counter;
// };

// queue for storing request id
queue<ThreadData> requestQueue;

// map for storing request id and client socket
map<long long int, int> in_queue;

// map for storing request id and processing status
map<long long int, int> still_processing;

// map for storing request id and status result
map<long long int , string> status_map;

// global variable for generating request id
long long int counter = 0;


// function of worker threads
void* workerThread(void* arg) 
{
    while (true) 
    {
        sleep(1);
        // waiting for request signal
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

        // change status to in queue
        in_queue[counter]= tdc.clientSocket;

        char buf[MAX_BUFFER_SIZE];
        // Receive the request type from the client
        ssize_t bytesRead = recv(tdc.clientSocket, buf, sizeof(buf), 0);
        buf[bytesRead]='\0';
        string type(buf);

        // send ready to serve message
        ssize_t byteswrite = send(tdc.clientSocket, "ready to serve", sizeof("ready to serve"), 0);
        
        // if request type is new
        if (strcmp(buf, "new") == 0) 
        {   
            ThreadData td;

            // generate request id and assign to request
            pthread_mutex_lock(&counter_lock);
            counter++;  
            updateCounter();
            td.counter=counter;
            td.clientSocket=tdc.clientSocket;
            pthread_mutex_unlock(&counter_lock);

            // call handlereq_new function
            handlereq_new(td);
        }
        else
        { 
            sleep(1);
            // if request type is status
            // call handlereq_status function
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
    
    // data from backup if server crashed previously
    // reinitializeData();

    // server is ready to accept connections
    cout << "Server listening on port " << atoi(argv[1]) << "..." << endl;

    // for purpose of checking output
    int xd=open("./Backup/abc.txt",O_WRONLY | O_TRUNC | O_CREAT, 0644);
    int fd=open("./Backup/output.txt",O_WRONLY | O_TRUNC | O_CREAT, 0644);
    string x="1 2 3 4 5 6 7 8 9 10";
    const char *y = x.c_str();
    int z=write(fd,y,x.size());
    if (close(fd) == -1) 
    {
       perror("close");
       return 1;
    }

    // Initialize mutex and condition variable objects
    pthread_cond_init(&cond, nullptr);
    pthread_mutex_init(&queue_lock, nullptr);
    pthread_mutex_init(&counter_lock, nullptr);
    pthread_mutex_init(&map_lock, nullptr);
    pthread_mutex_init(&lc, nullptr);

    // Get the number of worker threads from the command line
    int thread_pool_size = atoi(argv[2]);
    
    //make pool of threads
    pthread_t workerThreads[thread_pool_size];
    
    // Create worker threads
    for (int i = 0; i < thread_pool_size; i++) 
    {
        pthread_create(&workerThreads[i], nullptr, workerThread, nullptr);
    }

    // Accept incoming connections and assign them to a worker thread
    while (true) 
    {
        // Accept a new connection
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) 
        {
            cerr << "Error accepting connection." << endl;
            continue;
        }

        // Create a new thread data structure for the new connection
        ThreadData tdc;
        tdc.clientSocket = clientSocket;
        tdc.counter=0;

        // Enqueue the client socket for grading
        pthread_mutex_lock(&queue_lock);
        requestQueue.push(tdc);
        // Signal a worker thread to process the request
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&queue_lock);    
    }
    return 0;
}