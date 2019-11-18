#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 


#if defined(__linux__)
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    
    #define closesocket(param) close(param)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <Windows.h>
    typedef int socklen_t;
    typedef struct WSAData WSAData;
#endif
 

#define HOSTNAME_COMPUTER "hostname"
#define PORT 2000
#define HOST "127.0.0.1"
#define BACKLOG 5

 
int main(void)
{
#if defined(_WIN32)
    WSAData wsaData;
    WORD DllVersion = MAKEWORD(2, 2);

    if(WSAStartup(DllVersion, &wsaData) != 0)
        ExitProcess(EXIT_FAILURE);
#endif
 
    SOCKET socket_id, client_id;
    SOCKADDR_IN sockname;
 
    char szBuffer[1024];
    char szBufferReceive[32768];

    bool is_running = true;
    char szAttackUsername[256];
    int optval = 1;


    FILE* subprocess = NULL;

    subprocess = popen(HOSTNAME_COMPUTER, "r");

    if(subprocess == NULL)
    {
        perror("error");
    }
    else 
    { 
        fgets(szBuffer, sizeof(szBuffer)-1, subprocess);
        char* tmp = strchr(szBuffer, '\n');
        *tmp = '\0';
        strncpy(szAttackUsername, szBuffer, sizeof(szAttackUsername)-1);
        pclose(subprocess);
    }

    memset(&sockname, 0, sizeof(sockname));
    memset(szBuffer, '\0', sizeof(szBuffer));


    if((socket_id = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Server Failure:");
        exit(EXIT_FAILURE);
    }
#if defined(__linux__)
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
#endif
#if defined(_WIN32)
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
#endif

    sockname.sin_family = AF_INET;
    sockname.sin_port = htons(PORT);
    sockname.sin_addr.s_addr = htonl(INADDR_ANY);


    if(bind(socket_id, (struct sockaddr*)&sockname, sizeof(struct sockaddr)) < 0)
    {
        perror("Server Failure:");
        shutdown(socket_id, 2);
        closesocket(socket_id);
        exit(EXIT_FAILURE);
    }

    printf("+ %s:%d listen...\n", HOST, PORT);
    listen(socket_id, BACKLOG);

    client_id = accept(socket_id, NULL, NULL);

    printf("+ connected -%d\n", client_id);

    while(is_running)
    {
        printf("[%s] >", szAttackUsername);
        fgets(szBuffer, sizeof(szBuffer)-1, stdin);

        char* tmp = strchr(szBuffer, '\n');        
        *tmp = '\0';

        if(strncmp(szBuffer, "exit", sizeof(szBuffer)-1) == 0)
            is_running = false;
        
        send(client_id, szBuffer, sizeof(szBuffer) - 1, 0);
     
        if(recv(client_id, szBufferReceive, sizeof(szBufferReceive) - 1, 0) > 0)
            printf("%s", szBufferReceive);
        
        memset(szBuffer, '\0', sizeof(szBuffer) - 1);
        memset(szBufferReceive, '\0', sizeof(szBufferReceive) - 1);

    }

    printf("+ the server is closed !\n");
    shutdown(client_id, 2);
    shutdown(socket_id, 2);
    closesocket(client_id);
    closesocket(socket_id);

    return EXIT_SUCCESS;
}