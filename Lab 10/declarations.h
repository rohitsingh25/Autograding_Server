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

// buffer size 
extern const int MAX_BUFFER_SIZE ;

// mutex and condition variable
extern pthread_mutex_t lc,queue_lock,counter_lock , map_lock , qfile_lock;
extern pthread_cond_t cond;

// variables for analysis purpose
extern float queue_size,done_req;

// structure for thread data which includes client socket and request id
struct ThreadData {
    int clientSocket;
    long long int counter;
};

// queue for storing request id
extern queue<ThreadData> requestQueue;

// map for storing request id and client socket
extern map<long long int, int> in_queue;

// map for storing request id and processing status
extern map<long long int, int> still_processing;

// map for storing request id and status result
extern map<long long int , string> status_map;

// global variable for generating request id
extern long long int counter ;

// function declarations

// function to check if output is correct or not
bool isOutputCorrect(const string& );

// function to add thread data from queue to file
void addElementFromQueueToFile(int ); 

// function to remove request id from file
void removeRequestIDFromFile(int ); 

// function to reinitialize queue from file
void reinitializeQueueFromFile(string ); 

// function to reinitialize queue from still processing csv file
void reinitializeQueueFromFilesp(string ); 

// function to add entry to csv file
void addEntryToCSV(long long int , int  , string );

// function to load map from file
void loadMapFromFile(map<long long int , int>& , string ); 

// function to reintialize counter
void counterReintialize();

// function to update counter
void updateCounter();

// function to reinitialize data
void reinitializeData();


// FUNCTIONS FOR THREADS

// function to handle STATUS request
void *handlereq_status(int );

// function to handle NEW request
void *handlereq_new(ThreadData );