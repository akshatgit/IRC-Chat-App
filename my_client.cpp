#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>  
#include <cstddef> 
#include <errno.h>
#include <sys/sendfile.h>
#define PORT 8888
using namespace std;

int main(int argc, char *argv[])
{
    int sock = 0;
    int i=0;
    struct stat stat_buf;
    off_t offset = 0;   
    struct sockaddr_in serv_addr; 
    fd_set readfds;   
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); 

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) //convert IPv4 and IPv6 addresses from text to binary form
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    } 

    if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\nConnection Failed \n");
       return -1;
    } 

    int readbytes = 0;
    char recvBuff[1024];
    memset(recvBuff, '\0',sizeof(recvBuff));
    int max_sd,activity;
    int flag=0;
    while (1)
    {
        FD_ZERO(&readfds); 
        FD_SET(sock, &readfds);
        FD_SET(0, &readfds);
        max_sd = sock;
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        if(FD_ISSET(fileno(stdin), &readfds))
        {
            //cout<<"stdin"<<endl;
            cin>>recvBuff;
            send(sock , recvBuff , sizeof(recvBuff) , 0 );
            //cout<<"compare value"<<strcmp(recvBuff,"6")<<endl;
            if(!strcmp(recvBuff,"6"))
            {
                //printf("here\n");
                int fd,rc;
                string file1;
                char filename[100];
                FD_ZERO(&readfds); 
                cout<<"Enter the filename"<<endl;
                cin>>file1;
                strcpy(filename, file1.c_str()); 
                fd = open(filename, O_RDONLY);
                if (fd == -1) 
                {
                    fprintf(stderr, "unable to open '%s': %s\n", filename, strerror(errno));
                    exit(1);
                }
                std::size_t found = file1.find_last_of("/\\");
                memset(filename, '\0',sizeof(filename)); 
                strcpy(filename, file1.substr(found+1).c_str());
                send(sock, filename, sizeof(filename), 0); 
                fstat(fd, &stat_buf);
                //memset(filename, '\0',sizeof(filename)); 
                long no=(long)stat_buf.st_size;
                no=htonl(no);
                cout<<no<<endl;
                send(sock, &no, sizeof(no), 0);
                cout<<"sent size"<<ntohl(no)<<" "<<no<<endl;
                rc=sendfile (sock, fd, &offset, stat_buf.st_size);
                if (rc == -1) 
                {
                    fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
                    exit(1);
                }
                if (rc != stat_buf.st_size) 
                {
                    fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",rc,(int)stat_buf.st_size);
                    exit(1);
                }
                else
                {
                    cout<<"File transfer complete"<<endl;
                }
                close(fd);
            }  
            memset(recvBuff, '\0',sizeof(recvBuff)); 
            //cout<<"end"<<endl;
        }
        else if (FD_ISSET(sock, &readfds))
        {
            //cout<<"socket"<<endl;
            readbytes=recv(sock, recvBuff, sizeof(recvBuff),MSG_DONTWAIT);
            cout<<recvBuff;
            memset(recvBuff, '\0',sizeof(recvBuff));
            //cout<<"end"<<endl;
        } 
    }
    return 0;
}
