#include <iostream>
#include <winsock2.h>
#include <fstream>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void uploadFile(SOCKET socket, const string& fileName) {
    string command = "UPLOAD " + fileName; 
    send(socket, command.c_str(), command.size(), 0);

    ifstream inFile(fileName, ios::binary | ios::ate);
    if (!inFile.is_open()) {
        cerr << "ERROR: Cannot open file for reading." << endl;
        return;
    }

    int fileSize = inFile.tellg();
    send(socket, (char*)&fileSize, sizeof(fileSize), 0);

    inFile.seekg(0, ios::beg);
    char buffer[BUFFER_SIZE];
    cout << "Uploading file: " << fileName << " (Size: " << fileSize << " bytes)" << endl;
    while (!inFile.eof()) {
        inFile.read(buffer, BUFFER_SIZE);
        send(socket, buffer, inFile.gcount(), 0);
    }
    inFile.close();
    cout << "File uploaded: " << fileName << endl;
}

void downloadFile(SOCKET socket, const string& fileName) {
    string command = "DOWNLOAD " + fileName;
    send(socket, command.c_str(), command.size(), 0);

    int fileSize = 0;
    recv(socket, (char*)&fileSize, sizeof(fileSize), 0);

    if (fileSize <= 0) {
        cerr << "Error: Invalid file size or file not found on server." << endl;
        return;
    }

    ofstream outFile(fileName, ios::binary);
    if (!outFile.is_open()) {
        cerr << "ERROR: Cannot open file for writing." << endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    int totalBytesReceived = 0;
    while (totalBytesReceived < fileSize) {
        int bytesRead = recv(socket, buffer, min(BUFFER_SIZE, fileSize - totalBytesReceived), 0);
        if (bytesRead <= 0) {
            cerr << "Error: File transfer interrupted." << endl;
            break;
        }
        outFile.write(buffer, bytesRead);
        totalBytesReceived += bytesRead;
    }

    outFile.close();
    if (totalBytesReceived == fileSize) {
        cout << "File downloaded successfully: " << fileName << endl;
    } else {
        cerr << "Error: File download incomplete." << endl;
    }
}


int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection failed." << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server." << endl;

    //uploadFile(clientSocket, "upload.txt");
    downloadFile(clientSocket, "download.txt");

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

/*
Make sure you have gcc, g++, and gdb installed on your system.
To compile and run the file use the following commands
on a separate terminal than the one where the server is listening,
type in these commands:
g++ client.cpp -o client.exe -lws2_32
client.exe
*/
