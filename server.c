#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {

#ifdef _WIN32

    // Initialize Windows networking
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

#endif

    int serverSocket;
    int clientSocket;

    struct sockaddr_in serverAddress;

    char buffer[BUFFER_SIZE];


    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0) {
        printf("Socket creation failed\n");
        return 1;
    }


    // Configure server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);


    // Bind socket
    if (bind(serverSocket,
             (struct sockaddr*)&serverAddress,
             sizeof(serverAddress)) < 0) {

        printf("Bind failed\n");
        return 1;
    }


    // Start listening
    if (listen(serverSocket, 5) < 0) {
        printf("Listen failed\n");
        return 1;
    }


    printf("=================================\n");
    printf("       C HTTP SERVER STARTED\n");
    printf("=================================\n");
    printf("Listening on port %d\n", PORT);
    printf("Open your browser:\n");
    printf("http://localhost:8080\n\n");


    // Continuously accept clients
    while (1) {

        printf("Waiting for connection...\n");


        // Accept client connection
        clientSocket = accept(serverSocket, NULL, NULL);

        if (clientSocket < 0) {
            printf("Client connection failed\n");
            continue;
        }


        printf("Client Connected!\n\n");


        // Receive browser request
        memset(buffer, 0, BUFFER_SIZE);

        int bytesReceived = recv(
            clientSocket,
            buffer,
            BUFFER_SIZE - 1,
            0
        );


        if (bytesReceived <= 0) {

            printf("Failed to receive request\n");

#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif

            continue;
        }


        buffer[bytesReceived] = '\0';


        printf("Browser Request:\n");
        printf("%s\n", buffer);


        // Open HTML file
        FILE *file = fopen("index.html", "r");


        if (file == NULL) {

            printf("index.html not found\n");


            // Send 404 response
            char errorResponse[] =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 34\r\n"
                "\r\n"
                "<h1>404 - Page Not Found</h1>";


            send(
                clientSocket,
                errorResponse,
                strlen(errorResponse),
                0
            );
        }

        else {

            // Read HTML file
            char html[4096];

            size_t bytesRead = fread(
                html,
                1,
                sizeof(html) - 1,
                file
            );

            html[bytesRead] = '\0';

            fclose(file);


            // Create HTTP response
            char response[5000];

            sprintf(
                response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %zu\r\n"
                "\r\n"
                "%s",
                bytesRead,
                html
            );


            // Send response to browser
            send(
                clientSocket,
                response,
                strlen(response),
                0
            );


            printf("Page sent successfully!\n\n");
        }


        // Close client connection
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif

    }


    // Close server socket
#ifdef _WIN32
    closesocket(serverSocket);
    WSACleanup();
#else
    close(serverSocket);
#endif


    return 0;
}