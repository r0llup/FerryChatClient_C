/*
    FerryChatClient.c
    By Sh1fT
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define SERVER_HOST "10.8.0.14"
#define SERVER_PORT 31025
#define MAXSTRING 100

int main(int argc, char *arv[])
{
    int hSocketServer;
    int hSocketGroup;
    struct hostent *infosHostServer;
    struct hostent *infosHostGroup;
    struct in_addr adresseIPServer;
    struct in_addr adresseIPGroup;
    struct sockaddr_in adresseSocketServer;
    struct sockaddr_in adresseSocketGroup;
    unsigned int tailleSockaddr_inServer;
    unsigned int tailleSockaddr_inGroup;
    char username[40];
    char password[40];
    char msgClient[MAXSTRING];
    char msgServeur[MAXSTRING];
    char msgGroup[MAXSTRING];
    int nbreRecv, ret, cpt=0;
    struct ip_mreq mreq;
    int flagReuse=1;

    /* 1. login */

    hSocketServer = socket(AF_INET, SOCK_STREAM, 0);

    if (hSocketServer == -1)
    {
        printf("Socket creation [KO] : %d\n", errno);
        exit(1);
    }

    printf("Socket creation [OK]\n");

    if ((infosHostServer = gethostbyname(SERVER_HOST)) == 0)
    {
        printf("Retrieve host's info [KO] : %d\n", errno);
        exit(1);
    }

    printf("Retrieve host's info [OK]\n");
    memcpy(&adresseIPServer, infosHostServer->h_addr, infosHostServer->h_length);
    printf("Host's ip address: %s\n", inet_ntoa(adresseIPServer));
    memset(&adresseSocketServer, 0, sizeof(struct sockaddr_in));
    adresseSocketServer.sin_family = AF_INET;
    adresseSocketServer.sin_port = htons(SERVER_PORT);
    memcpy(&adresseSocketServer.sin_addr, infosHostServer->h_addr, infosHostServer->h_length);
    tailleSockaddr_inServer = sizeof(struct sockaddr_in);

    if ((ret = connect(hSocketServer, (struct sockaddr *) &adresseSocketServer, tailleSockaddr_inServer)) == -1)
    {
         printf("Socket connection [KO] : %d\n", errno);
         close(hSocketServer);
         exit(1);
    }

    printf("Socket connection [OK]\n");
    printf("LOGiN\n");
    printf("Username: ");
    gets(username);
    printf("Password: ");
    gets(password);
    strcpy(msgClient, "LOGIN;");
    strcat(msgClient, username);
    strcat(msgClient, ";");
    strcat(msgClient, password);
    strcat(msgClient, "\n");
    printf("Request: %s\n", msgClient);

    if (send(hSocketServer, msgClient, MAXSTRING, 0) == -1)
    {
        printf("Send socket [KO] : %d\n", errno);
        close(hSocketServer);
        exit(1);
    }

    printf("Send socket [OK]\n");
    printf("Sended request: %s\n", msgClient);

    if (recv(hSocketServer, msgServeur, MAXSTRING, 0) == -1)
    {
        printf("Recv socket [KO] : %d\n", errno);
        close(hSocketServer);
        exit(1);
    }

    printf("Recv socket [OK]\n");
    printf("Received response: [%s]\n", msgServeur);

    if (strstr(msgServeur, "ko") == NULL)
    {
        /* 2. chat */

        printf("Welcome, %s !\n", username);
        char groupHost[9];
        strcpy(groupHost, strtok(msgServeur, ":"));
        groupHost[9] = '\0';
        char groupPort[5];
        strncpy(groupPort, strtok(NULL, ":"), 5);
        groupPort[5] = '\0';
        hSocketGroup = socket(AF_INET, SOCK_DGRAM, 0);

        if (hSocketGroup == -1)
        {
            printf("Socket creation [KO] : %d\n", errno);
            exit(1);
        }

        printf("Socket creation [OK]\n");

        if ((infosHostGroup = gethostbyname(groupHost)) == 0)
        {
            printf("Retrieve host's info [KO] : %d\n", errno);
            exit(1);
        }

        printf("Retrieve host's info [OK]\n");
        memcpy(&adresseIPGroup, infosHostGroup->h_addr, infosHostGroup->h_length);
        printf("Host's ip address: %s\n" , inet_ntoa(adresseIPGroup));
        tailleSockaddr_inGroup = sizeof(struct sockaddr_in);
        memset(&adresseSocketGroup, 0, tailleSockaddr_inGroup);
        adresseSocketGroup.sin_family = AF_INET;
        adresseSocketGroup.sin_port = atoi(groupPort);
        adresseSocketGroup.sin_addr.s_addr = inet_addr(groupHost);
        printf("Setup adresseSocketGroup [OK]\n");

        if (bind(hSocketGroup, (struct sockaddr *) &adresseSocketGroup, tailleSockaddr_inGroup) == -1)
        {
            printf("Socket bind [KO] : %d\n", errno);
            exit(1);
        }

        printf("Socket bind [OK]\n");
        memcpy(&mreq.imr_multiaddr, &adresseSocketGroup.sin_addr, tailleSockaddr_inGroup);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        printf("Usage of the following address: %s\n", inet_ntoa(mreq.imr_interface));
        setsockopt (hSocketGroup, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

        do
        {
            memset(msgGroup, 0, MAXSTRING);
            if ((nbreRecv = recvfrom(hSocketGroup, msgGroup, MAXSTRING, 0,
                (struct sockaddr *)&adresseSocketGroup, &tailleSockaddr_inGroup)) == -1)
            {
                printf("Recvfrom socket [KO] : %d\n", errno);
                close(hSocketGroup);
                exit(1);
            }

            printf("Recvfrom socket [OK]\n");
            msgGroup[nbreRecv+1] = 0;
            printf("Received message: %s\n", msgGroup);
            printf("Sender address: %s\n", inet_ntoa(adresseSocketGroup.sin_addr));
        } while (1); // ;)

        close(hSocketGroup);
        printf("Group socket closed.\n");
}
    else
        printf("Authentification error.\n");

    close(hSocketServer);
    printf("Server socket closed.\n");
    return 0;
}
