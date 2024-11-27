#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <sstream>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;

    bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead <= 0) {
        cerr << "Error receiving command from client." << endl;
        closesocket(clientSocket);
        return;
    }
    buffer[bytesRead] = '\0';

    stringstream ss(buffer);
    string command, fileName;
    ss >> command >> fileName;

    cout << "Received command: " << command << " " << fileName << endl;

    if (command == "UPLOAD") {
        ofstream outFile(fileName, ios::binary);
        if (!outFile.is_open()) {
            string error = "ERROR: Cannot open file for writing.\n";
            send(clientSocket, error.c_str(), error.size(), 0);
            return;
        }

        int fileSize = 0;
        recv(clientSocket, (char*)&fileSize, sizeof(fileSize), 0);

        int totalBytesReceived = 0;
        while (totalBytesReceived < fileSize) {
            bytesRead = recv(clientSocket, buffer, min(BUFFER_SIZE, fileSize - totalBytesReceived), 0);
            if (bytesRead <= 0) break;
            outFile.write(buffer, bytesRead);
            totalBytesReceived += bytesRead;
        }

        outFile.close();
        if (totalBytesReceived == fileSize) {
            cout << "File uploaded successfully: " << fileName << endl;
        } else {
            cerr << "Error: Incomplete file upload." << endl;
        }
    } else if (command == "DOWNLOAD") {
        // Handle file download
        ifstream inFile(fileName, ios::binary | ios::ate);
        if (!inFile.is_open()) {
            string error = "ERROR: File not found.\n";
            send(clientSocket, error.c_str(), error.size(), 0);
            return;
        }

        int fileSize = inFile.tellg();
        inFile.seekg(0, ios::beg);
        send(clientSocket, (char*)&fileSize, sizeof(fileSize), 0);

        int totalBytesSent = 0;
        while (totalBytesSent < fileSize) {
            inFile.read(buffer, BUFFER_SIZE);
            int bytesToSend = inFile.gcount();
            send(clientSocket, buffer, bytesToSend, 0);
            totalBytesSent += bytesToSend;
        }

        inFile.close();
        if (totalBytesSent == fileSize) {
            cout << "File sent successfully: " << fileName << endl;
        } else {
            cerr << "Error: Incomplete file transfer." << endl;
        }
    } else {
        string error = "ERROR: Invalid command.\n";
        send(clientSocket, error.c_str(), error.size(), 0);
    }

    closesocket(clientSocket);
}


int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 3) == SOCKET_ERROR) {
        cerr << "Listen failed." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port " << PORT << "..." << endl;

    while ((clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
        cout << "Client connected." << endl;
        handleClient(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}


/*
Make sure you have gcc, g++, and gdb installed on your system.
To compile and run the file use the following commands
on a separate terminal than the one where the client sends requests,
type in these commands:
g++ server.cpp -o server.exe -lws2_32
server.exe
*/

