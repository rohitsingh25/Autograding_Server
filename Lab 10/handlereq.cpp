#include "declarations.h"

// Function to handle NEW request
void *handlereq_new(ThreadData threadData)
{
    int clientSocket = (threadData.clientSocket);
    char *buffer = new char[MAX_BUFFER_SIZE];

    // Receive the code file from the client if it were not in still processing map
    if(clientSocket!=0)
    {
        // Receive the code file from the client
        ssize_t bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        
        if (bytesRead == 0 || bytesRead == -1) 
        {
            close(clientSocket);
            return nullptr;
        }
        
        // generate request id
        string req_id=to_string(threadData.counter); 
        
        // send request id to client
        ssize_t byteswrite = send(clientSocket, req_id.c_str(), req_id.size(), 0);

        // add request id to still processing map
        still_processing[threadData.counter]=0;
        // update into file as well
        pthread_mutex_lock(&map_lock);
        addEntryToCSV(threadData.counter ,0, "./Backup/map_backup.csv");
        pthread_mutex_unlock(&map_lock);
        
        // change status to file received
        status_map[threadData.counter] = "file received ";
        
        // add request id to queue if processing is started
        pthread_mutex_lock(&qfile_lock);
        addElementFromQueueToFile(threadData.counter);
        pthread_mutex_unlock(&qfile_lock);
        close(clientSocket);
    }

    // temp file of code
    string tempFileName = "student_code_" + to_string(threadData.counter) + ".cpp";
    // result file
    string res = "./Results/res" + to_string(threadData.counter) + ".txt";

    // Write the code to a file if it were not in still processing map
    if(clientSocket!=0)
    {   ofstream codeFile(tempFileName);
        codeFile << buffer;
        codeFile.close();
    }
    
    // Compile the code
    int compileResult = system(("g++ -o student_program_" + to_string(threadData.counter) + " " + tempFileName + " 2> " + res).c_str());   
    //change status to file compiled
    status_map[threadData.counter]="file compiled";
    
    if (compileResult != 0) 
    {
        // Compilation error
        status_map[threadData.counter]="COMPILER ERROR";
        system(("rm " + tempFileName).c_str());
        
        // remove request id from still processing map
        still_processing.erase(still_processing.find(threadData.counter));
        pthread_mutex_lock(&qfile_lock);
        removeRequestIDFromFile(threadData.counter);
        pthread_mutex_unlock(&qfile_lock);
        return NULL;
    }
    
    // Execute the code
    int executionResult = system(("./student_program_" + to_string(threadData.counter) + " 1>student_output_"+to_string(threadData.counter)+".txt 2> " + res).c_str());
    //change status to file executed
    status_map[threadData.counter]="file executed";
    if (executionResult != 0) 
    {
        // Runtime error
       status_map[threadData.counter]="RUNTIME ERROR";
       system(("rm " + tempFileName).c_str());
       system(("rm student_output_" + to_string(threadData.counter) + ".txt").c_str());
       system(("rm student_program_" + to_string(threadData.counter)).c_str());
    } 
    else 
    {
        // Execution successful
        system(("rm " + tempFileName).c_str());
        
        // Capture the program's output
        ifstream output_file(("student_output_"+to_string(threadData.counter)+".txt"));
        string programOutput((istreambuf_iterator<char>(output_file)), istreambuf_iterator<char>());
        
        // Check if the output is correct
        status_map[threadData.counter]="checking output";
        bool outputCorrect = isOutputCorrect(programOutput);
        
        // if output is correct store it in result file
        if (outputCorrect) 
        {
            // Output is correct so saving it in result file
            status_map[threadData.counter]="PASS";
            system(("cp student_output_" + to_string(threadData.counter) + ".txt " + res).c_str());
            system(("rm student_output_" + to_string(threadData.counter) + ".txt").c_str());
            system(("rm student_program_" + to_string(threadData.counter)).c_str());
        }
        else 
        {
            // There's an output error, compare it with the expected output
            status_map[threadData.counter]="OUTPUT ERROR";
            system(("diff student_output_" + to_string(threadData.counter) + ".txt output.txt > " + res).c_str()); 
            system(("rm student_output_" + to_string(threadData.counter) + ".txt").c_str()); 
            system(("rm student_program_" + to_string(threadData.counter)).c_str());
        }     
    }
    // remove request id from still processing map
    still_processing.erase(still_processing.find(threadData.counter));
    
    // remove request id from backup file
    pthread_mutex_lock(&qfile_lock);
    removeRequestIDFromFile(threadData.counter);
    pthread_mutex_unlock(&qfile_lock);
    
    return nullptr;
}  

void *handlereq_status(int clientSocket)
{
    char *buffer = new char[MAX_BUFFER_SIZE];

    // Receive the request id from the client
    ssize_t bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
    
    if (bytesRead == 0 || bytesRead == -1) 
    {
        // Closing the connection with the client when no data is sent by it
        close(clientSocket);
        return nullptr;
    }
    
    // convert request id to long long int
    buffer[bytesRead]='\0';
    char *endptr;
    long long int convertedValue = strtoll(buffer, &endptr, 10);
    long long int g=convertedValue;
    
    // seeking request id in status map
    auto t1=status_map.find(g);
    auto t5=in_queue.find(g);
    auto t6=still_processing.find(g);

    // if request id is found in status map
    if(t1!=status_map.end()) 
    {
        // send status to client
        ssize_t bytessent = send(clientSocket,status_map[g].c_str(),sizeof(status_map[g]) , 0);      
        close(clientSocket);
        return nullptr;
    }
    else if(t6!=still_processing.end())
    {
        // send status to client
        ssize_t bytessent = send(clientSocket, status_map[g].c_str(),sizeof(status_map[g]), 0);  
        return nullptr;
    }
    else if(t5!=in_queue.end())
    {
        // send status to client
        ssize_t bytessent = send(clientSocket, "Request is in queue",sizeof("Request is in queue"), 0);
        return nullptr;
    }
    else
    {
        // send invalid request id to client
        ssize_t bytessent = send(clientSocket, "Invalid request ID given",sizeof("Invalid request ID given"), 0);
        close(clientSocket);
        return nullptr;
    }
    return nullptr;
}  
