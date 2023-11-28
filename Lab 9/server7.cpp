#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream>
#include <cstdio>
#include <fcntl.h>


using namespace std;

const int MAX_BUFFER_SIZE = 10000;

bool isOutputCorrect(const string& programOutput) {
    string expectedOutput = "1 2 3 4 5 6 7 8 9 10";
    return programOutput == expectedOutput;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
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

    cout << "Server listening on port " << atoi(argv[1]) << "..." << endl;
    
     int fd=open("output.txt",O_WRONLY | O_TRUNC | O_CREAT, 0644);
     string x="1 2 3 4 5 6 7 8 9 10";
     const char *y = x.c_str();
     int z=write(fd,y,x.size());
     if (close(fd) == -1) {
        perror("close");
        return 1;
    }
     while(true) {
        // Accept incoming connection
        
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Error accepting connection." << endl;
            continue;
            }
        char buffer[MAX_BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        // Receive source code from client
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == 0 || bytesRead == -1) {
            //cerr << "Error receiving data." << endl;
            //closing connection with client when no data is sent by it
            close(clientSocket);
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket == -1) {
            cerr << "Error accepting connection." << endl;
            continue;
            }
            continue;
        }

        // Save the received code to a temporary file
        ofstream codeFile("student_code.cpp");
        codeFile << buffer;
        codeFile.close();
        ifstream eFile("student_code.cpp");
        string er((istreambuf_iterator<char>(eFile)), istreambuf_iterator<char>());
        // Compile and run the code
        int compileResult = system("g++ -o student_program student_code.cpp 2> compile_error.txt");
        if (remove("student_code.cpp") != 0) {
            cerr << "Error deleting temporary code file." << endl;
            }
        if (compileResult != 0) {
            // Compilation failed, send the error message back
            ifstream errorFile("compile_error.txt");
            string compileError((istreambuf_iterator<char>(errorFile)), istreambuf_iterator<char>());
            string response = "COMPILER ERROR\n" + compileError;
            send(clientSocket, response.c_str(), response.size(), 0);
            close(clientSocket);
            errorFile.close();
             if (remove("compile_error.txt") != 0) {
            cerr << "Error deleting temporary code file." << endl;
            }
            //close(clientSocket);
            
            continue;
        }
        if (remove("compile_error.txt") != 0) {
            cerr << "Error deleting temporary code file." << endl;
            }

        int executionResult = system("./student_program > student_output.txt 2>runtime_error.txt");

        if (executionResult != 0) {
            // Runtime error, send the error message back
            ifstream err("runtime_error.txt");
            string runtimeError((istreambuf_iterator<char>(err)), istreambuf_iterator<char>());
            string response = "RUNTIME ERROR\n"+runtimeError;
            send(clientSocket, response.c_str(), response.size(), 0);
            close(clientSocket);
            err.close();
            //close(clientSocket);
            if (remove("runtime_error.txt") != 0) {
            cerr << "Error deleting temporary code file." << endl;
            }
            continue;
        }
        if (remove("runtime_error.txt") != 0) {
            cerr << "Error deleting temporary code file." << endl;
            }
        // Capture the program's output
        ifstream output_file("student_output.txt");
        string programOutput((istreambuf_iterator<char>(output_file)), istreambuf_iterator<char>());
        output_file.close();
        // Check if the output is correct
        bool outputCorrect = isOutputCorrect(programOutput);

        // Send result back to the client
        string response = outputCorrect ? "PASS" : "OUTPUT ERROR";
        if(response=="OUTPUT ERROR") {
        system("diff student_output.txt output.txt > output_error.txt");
        ifstream err("output_error.txt");
        string OutErr((istreambuf_iterator<char>(err)), istreambuf_iterator<char>());
        OutErr="OUTPUT ERROR\n"+OutErr;
        send(clientSocket, OutErr.c_str(), OutErr.size(), 0);
        close(clientSocket);
        err.close();
        if (remove("output_error.txt") != 0) {
            cerr << "Error deleting temporary code file." << endl;
            }
         
        }
        else{send(clientSocket, response.c_str(), response.size(), 0);  close(clientSocket); }
         if (remove("student_output.txt") != 0) {
            cerr << "Error deleting temporary code file." << endl;
            }
        }      
     //close(clientSocket);
     //close(serverSocket);
    return 0;
}
