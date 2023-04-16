#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char **argv){
    if (argc != 4)
        {
            printf("Missing arguments\n");
            exit(1);
        }

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listener == -1){
        perror("Socket failed!");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (client == -1)
    {
        printf("Accept connection failed\n");
        exit(1);
    }
    printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // Send hello message to client
    char *filename = argv[2];
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    char buf[512];
    int ret;
    while (!feof(f))
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        if (send(client, buf, ret, 0) == -1)
        {
            printf("Send hello to client failed\n");
            exit(1);
        }
    }
    send(client, "\r\n\r\n", 4, 0);   // send close signal to client
    fclose(f);

// Write client's message to file
    filename = argv[3];
    f = fopen(filename, "a");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
    }

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0) break;
        fwrite(buf, 1, ret, f);
        printf("Writed\n");
    }
    fclose(f);


    close(client);
    close(listener);
    printf("Close Connection.");
}