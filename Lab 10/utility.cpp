#include "declarations.h"

// Function to check if output is correct or not
bool isOutputCorrect(const string& programOutput) 
{
    string expectedOutput = "1 2 3 4 5 6 7 8 9 10";
    return programOutput == expectedOutput;
}

// Function to add thread data from queue to file
void addElementFromQueueToFile(int element) 
{
    ofstream file("./Backup/queue_backup.txt", ios::out | ios::trunc);
    file << element << endl;
    // cout<<element<<" pushed in file"<<endl;
    file.close();
}

// Function to remove request id from file
void removeRequestIDFromFile(int req_id) 
{
    ifstream inFile("./Backup/queue_backup.txt");
    if (!inFile.is_open()) 
    {
        cout << "File not found or unable to open!" << endl;
        return;
    }
    string line;
    ofstream outFile("./Backup/temp.txt");
    while (getline(inFile, line)) 
    {
        istringstream iss(line);
        int id;
        if (iss >> id && id == req_id)
            continue;
        outFile << line << std::endl;
    }
    inFile.close();
    outFile.close();
    remove("./Backup/queue_backup.txt");
    rename("./Backup/temp.txt", "./Backup/queue_backup.txt");
}

// Function to reinitialize queue from file
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

// Function to reinitialize queue from still processing csv file
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
        ss >> comma; 
        ss >> value;
        ThreadData td ;
        td.clientSocket=value;
        td.counter=key;
        requestQueue.push(td);
    }
    file.close();
}

// Function to add entry to csv file
void addEntryToCSV(long long int key, int value , string FILE_NAME) {
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

// Function to load map from file
void loadMapFromFile(map<long long int , int>&mp , string FILE_NAME) {
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
            ss >> comma; 
            ss >> value;
            mp[key] = value;
        }
        file.close();
    } 
    else 
    {
        cout << "File not found or unable to open." << endl;
    }
}

// Function to reintialize counter
void counterReintialize()
{
    ifstream file("./Backup/counter.txt");
    if (!file.is_open()) 
    {
        cout << "File not found or unable to open!" << endl;
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
        else
        {
            counter=c;
        }
    }
    else
    {
        counter =0;
    }
    file.close();
}

// Function to update counter
void updateCounter()
{
    ofstream outFile("./Backup/counter.txt", ofstream::trunc);
    if (!outFile) {
        std::cerr << "Unable to open file!" << std::endl;
        return;
    }
    outFile << counter << endl;
    outFile.close();
}

// Function to reinitialize data
void reinitializeData()
{
    reinitializeQueueFromFile("./Backup/queue_backup.txt");
    reinitializeQueueFromFilesp("./Backup/map_backup.csv");
    counterReintialize();
    loadMapFromFile(in_queue,"./Backup/map_backup.csv");
    cout<<"Data Initialized"<<endl;
}
