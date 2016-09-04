#include <stdio.h>
#include <string.h>   
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>  
#include <arpa/inet.h>   
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>
#include <cstring>
#include <vector> 
#include <fcntl.h> 
#include <cstddef>
#include <errno.h>
#include <assert.h> 
#include <sys/sendfile.h>
#define PORT 8888

using namespace std;
ssize_t read_write_bs(int in, int out, ssize_t bs,ssize_t t)
{
    ssize_t w = 0, r = 0,  n, m;

    char *buf = (char *)malloc(bs);
    if(buf==NULL)
    {
        cout<<"error in malloc\n";
        return 0;
    }

    while(r < t && (n = read(in, buf, bs))) {
        if(n == -1) { assert(errno == EINTR); continue; }
        r = n;
        w = 0;
        while(w < r && (m = write(out, buf + w, (r - w)))) {
            if(m == -1) { assert(errno == EINTR); continue; }
            w += m;
        }
    }

    free(buf);
    cout<<"success"<<endl;
    return w;
}
int main(int argc, char const *argv[])
{
    pair<string,string> * client_details;
    vector<pair<string, pair <string,string> > >messages;
    vector<pair< string, vector<string> > >group;
    int transfer[30];
    char * group_message[30];
    int server_fd, client_list[30],client_state[30], addrlen, activity , valread , sd,new_socket;
    int fptr[30];
    size_t size[30];
    int max_sd,i;
    struct sockaddr_in address;
    char *client_meta[30];
    fd_set readfds;
    client_details = new std::pair<string,string> [30]; 
    int opt=1;
    vector<pair<string,string> > account;
    char buffer[1025];
    int max_client =30; 
    char *instructions="\n\tPress 1 for /who\n\tPress 2 for /msg\n\tPress 3 for /create_grp/\n\tPress 4 for /join_grp\n\tPress 5 for /msg_grp\n\tPress 6 for /send\n\tPress 7 for /recv\n\tPress 8 for logout\n";
    for (i = 0; i < max_client; i++) 
    {
        client_list[i] = 0;
        client_state[i]=0;
    }
    if( (server_fd = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created.\n");
    if( setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    printf("setsockopt success\n");
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("bind success\n");
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for connection on %d\n",PORT);
    addrlen = sizeof(address);
    int flag=0;
    while(1)
    {
        FD_ZERO(&readfds);
        
        FD_SET(server_fd, &readfds);
        
        max_sd = server_fd;
        for ( i = 0 ; i < max_client ; i++) 
        {        
            sd=client_list[i]; 
            if(client_list[i] > 0)
                FD_SET( sd , &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
        printf("activity working");
        if (FD_ISSET(server_fd, &readfds)) 
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            //printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            char *message="Welcome to IRC\n\tPress 1 to create new account.\n\tPress 2 to login.\n";
            if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
            {
                perror("send");
            }
              
            printf("Welcome message sent successfully\n");
              
            //add new socket to array of sockets
            for (i = 0; i < max_client; i++) 
            {
                //if position is empty
                if( client_list[i] == 0 )
                {
                    client_list[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                     
                    break;
                }
            }
        }
     //else its some IO operation on some other socket :)
        for (i = 0; i < max_client; i++) 
        {
            sd = client_list[i];
            // cout<<sd; 
            if (FD_ISSET( sd , &readfds)) 
            {
                std::memset(buffer, 0, sizeof buffer);
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                      
                    //Close the socket and mark as 0 in list for reuse
                    client_state[i]=0;
                    client_meta[i]=0;
                    close( sd );
                    client_list[i] = 0;
                }
                  
                //Echo back the message that came in
                else
                {
                    printf("here\n");
                    //set the string terminating NULL byte on the end of the data read
                    buffer[valread-2] = '\0';
                    //cout<<buffer<1<endl;ad
                    cout<<"Read"<<valread<<endl;
                    //cout<<valread<<endl; 
                    cout<<"Comparing with 1"<<strcmp(buffer,"1")<<endl;
                    cout<<"Comparing with 8"<<strcmp(buffer,"8")<<endl;
                    //send(sd , message , strlen(message) , 0 );
                    
                    if(!strcmp(buffer,"1") && client_state[i]<80)
                    {
                        client_state[i]=2;
                        char *message="\tEnter a new Username\n";
                        send(sd , message , strlen(message) , 0 );
                        //std::memset(buffer, 0, sizeof buffer);
                    }
                    else if(!strcmp(buffer,"2") && client_state[i]<80)
                    {
                        client_state[i]=4;
                        char *message="\tEnter the Username\n";
                        send(sd , message , strlen(message) , 0 );
                    }
                    else if(!strcmp(buffer,"8"))
                    {
                        client_state[i]=1;
                        char *message="\tWelcome to IRC\n\tPress 1 to create new account.\n\tPress 2 to login.\n";
                        send(sd , message , strlen(message) , 0 );
                        //std::memset(buffer, 0, sizeof buffer);
                    }
                    else if(!strcmp(buffer,"1") && client_state[i]>80)
                    {
                        for (int j = 0; j < 30; j++) 
                        {
                            //if position is empty
                            if( client_list[j] != 0 )
                            {         
                                if(client_state[j]>80)
                                {                      
                                    string a=client_details[j].first+"\n";
                                    //cout<<"who"<<a<<endl;
                                    char tab2[100];
                                    strcpy(tab2, a.c_str()); 
                                    send(sd , tab2 , strlen(tab2) , 0 );
                                }


                            }
                        }
                    }
                    else if(!strcmp(buffer,"2") && client_state[i]>80)
                    {
                        char *message="\tTo whom do you want to send the message? Enter his name\n"; 
                        send(sd , message , strlen(message) , 0 );
                        client_state[i]=98;
                    }
                    else if(!strcmp(buffer,"3") && client_state[i]>80)
                    {
                        char *message="\tEnter the name of the group\n"; 
                        send(sd , message , strlen(message) , 0 );
                        client_state[i]=94;
                    }
                    else if(!strcmp(buffer,"4") && client_state[i]>80)
                    {
                        char *message="\tEnter the name of the group\n"; 
                        send(sd , message , strlen(message) , 0 );
                        client_state[i]=93;   
                    }
                    else if(!strcmp(buffer,"5") && client_state[i]>80)
                    {
                        char *message="\tEnter the message\n"; 
                        send(sd , message , strlen(message) , 0 );
                        client_state[i]=92;   
                    }
                    else if(!strcmp(buffer,"6") && client_state[i]>80)
                    {
                        client_state[i]=96;
                    }
                    else if(!strcmp(buffer,"7") && client_state[i]>80)
                    {
                        int j3=0;
                        vector<pair<string,pair < string,string > > >::iterator it; 
                        for(it=messages.begin() ; it < messages.end(); it++,j3++) 
                        {
                            cout<<it->second.first;
                            string a=buffer;
                            if(it->second.first==client_details[i].first)
                            {
                                char tab2[1025];
                                it->second.second=it->second.second+":";
                                strcpy(tab2, it->second.second.c_str());
                                send(sd , tab2 , strlen(tab2) , 0 );
                                it->first="\t"+it->first;
                                strcpy(tab2, it->first.c_str()); 
                                send(sd , tab2 , strlen(tab2) , 0 );
                                char *a="\n";
                                send(sd , a , strlen(a) , 0 );
                                messages.erase(messages.begin()+j3);
                                it--;
                            }
                        }    
                    }
                    else
                    {
                        if(client_state[i]==2)
                        {
                            int flag=0;
                            vector<pair<string,string> >::iterator it; 
                            for(it=account.begin() ; it < account.end(); it++) 
                            {
                                string a=buffer;
                                if(it->first==a)
                                {
                                    flag=1;
                                    char *message="\tThis username is taken. Enter a unique username.\n"; 
                                    send(sd , message , strlen(message) , 0 );
                                    break;
                                }
                            }
                            if (!flag)
                            {
                                char *message="\tEnter a new Password\n"; 
                                send(sd , message , strlen(message) , 0 );
                                client_details[i]=make_pair(buffer,"");
                                client_state[i]=3;
                            }
                            
                            //std::memset(buffer, 0, sizeof buffer);
                        }
                        else if(client_state[i]==3)
                        {
                            char *message="\tAccount Created\n";
                            send(sd , message , strlen(message) , 0 );
                            client_details[i].second=buffer;
                            cout<<client_details[i].first<<client_details[i].second<<endl;
                            client_state[i]=99;
                            account.push_back(client_details[i]);
                            send(sd , instructions , strlen(instructions) , 0 );
                            //std::memset(buffer, 0, sizeof buffer);
                        }   
                        else if(client_state[i]==4)
                        {
                            int flag=0;
                            vector<pair<string,string> >::iterator it; 
                            for(it=account.begin() ; it < account.end(); it++) 
                            {
                                string a=buffer;
                                if(a==buffer)
                                {
                                    flag=1;
                                    client_details[i]=make_pair(it->first,it->second);
                                }
                            }
                            if(!flag)
                            {
                                char *message="\tEnter a valid username\n"; 
                                send(sd , message , strlen(message) , 0 );
                                break;
                            }
                            char *message="\tEnter the Password\n"; 
                            send(sd , message , strlen(message) , 0 );
                            client_state[i]=5;
                        }
                        else if(client_state[i]==5)
                        { 
                            int flag=0;  
                            string a=buffer;
                            if(a==client_details[i].second)
                                flag=1;
                            cout<<client_details[i].second<<endl;
                            if(flag)
                            {
                                char * message="\tLogged in\n";
                                send(sd , message , strlen(message) , 0 );
                                cout<<client_details[i].first<<client_details[i].second<<endl;
                                client_state[i]=99;
                                send(sd , instructions , strlen(instructions) , 0 );                                
                            }
                            else
                            {
                                char * message="\tIncorrect Password\n";
                                send(sd , message , strlen(message) , 0 );
                                client_state[i]=5;
                            }
                            //std::memset(buffer, 0, sizeof buffer);
                        }
                        else if(client_state[i]==98)
                        {   
                            int flag=0;int j2=0;
                            vector<pair<string,string> >::iterator it; 
                            for(it=account.begin() ; it < account.end(); it++,j2++) 
                            {
                                string a=buffer;
                                if(it->first==a)
                                {
                                    flag=1;
                                    client_meta[i]=(char *)(malloc(100*sizeof(char)));
                                    strcpy(client_meta[i],  it->first.c_str());
                                }
                            }
                            if(flag)
                            {
                                char * message="\tEnter the message\n";
                                send(sd , message , strlen(message) , 0 );
                                client_state[i]=97;
                            }
                            else
                            {
                                char * message="\tEnter a valid address\n";
                                send(sd , message , strlen(message) , 0 );
                            }

                        }
                        else if(client_state[i]==97)
                        {   
                            messages.push_back(make_pair(buffer,make_pair(client_meta[i],client_details[i].first)));
                            char * message="\tMessage Recieved\n";
                            send(sd , message , strlen(message) , 0 );
                            send(sd , instructions , strlen(instructions) , 0 );
                            client_state[i]=99;
                            free(client_meta[i]);
                        }
                        else if(client_state[i]==96)
                        {
                            cout<<"creating new file :"<<buffer<<endl;
                            fptr[i]  = open(buffer, O_WRONLY | O_CREAT);
                            client_state[i]=95;
                        }
                        else if(client_state[i]==95)
                        {
                            long my_net_id;
                            std::memcpy((void *)&my_net_id,(void *)buffer,sizeof(long));
                            size[i]=(size_t)ntohl(my_net_id);
                            cout<<valread<<sizeof(size_t)<<" "<<buffer<<endl;
                            cout<<my_net_id<<"No of bytes: "<<size[i]<<endl;
                            FD_ZERO(&readfds);
                            read_write_bs(client_list[i],fptr[i],512,(ssize_t)size[i]);
                            // transfer[i]=sendfile(fileno(fptr[i]), client_list[i], &offset, size[i]);
                            // if (transfer[i] == -1) 
                            // {
                            //     fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
                            //     exit(1);
                            // }
                            // if (transfer[i] != size[i]) 
                            // {
                            //     fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",transfer[i],size[i]);
                            //     exit(1);
                            // }
                            // else
                            // {
                            //     cout<<"File transfer complete"<<endl;
                            // }
                            close(fptr[i]);
                        }
                        else if(client_state[i]==94)
                        {
                            vector<pair < string, vector<string> > >::iterator it; 
                            int flag=0;
                            string a=buffer;
                            for(it=group.begin() ; it < group.end(); it++) 
                            { 
                                if(it->first==a)
                                {
                                    flag=1;
                                    char * message="\tGroup exists! Enter some other name\n";
                                    send(sd , message , strlen(message) , 0 );
                                }
                            }
                            if(!flag)
                            {
                                group.push_back(make_pair(a,vector<string>()));
                                group.back().second.push_back(client_details[i].first);
                                char * message="\tGroup Created!\n";
                                send(sd , message , strlen(message) , 0 );
                                client_state[i]=99;
                            } 
                        }
                        else if(client_state[i]==93)
                        {
                            vector<pair < string, vector<string> > >::iterator it; 
                            int flag2=0;
                            int flag=0;
                            for(it=group.begin() ; it < group.end(); it++) 
                            {
                                string a=buffer;
                                if(it->first==a)
                                {
                                    vector<string >::iterator it2; 
                                    for(it2=it->second.begin() ; it2 < it->second.end(); it2++) 
                                    {
                                        cout<<*it2<<endl;
                                        if(*it2==client_details[i].first)
                                        {
                                            flag2=1;
                                            flag=1;
                                            char * message="\tAlready in the group!\n";
                                            send(sd , message , strlen(message) , 0 );   
                                        }
                                    }
                                    if(!flag2)
                                    {
                                        flag=1;
                                        it->second.push_back(client_details[i].first);
                                        char * message="\tAdded to the group\n";
                                        send(sd , message , strlen(message) , 0 );
                                        client_state[i]=99;
                                    }
                                    
                                }
                                
                            }
                            if(!flag)
                            {
                                char * message="\tGroup does not exist!\n";
                                send(sd , message , strlen(message) , 0 );
                            }
                        }
                        else if(client_state[i]==92)
                        {
                            string a=buffer;
                            group_message[i]=(char *)malloc(1024*sizeof(char));
                            strcpy(group_message[i],  a.c_str());
                            char *message="\tEnter the name of the group\n"; 
                            send(sd , message , strlen(message) , 0 );
                            client_state[i]=91;
                        }
                        else if(client_state[i]==91)
                        {
                            vector<pair < string, vector<string> > >::iterator it; 
                            int flag2=0;
                            int flag=0;
                            string a=buffer;
                            string b=group_message[i];
                            for(it=group.begin() ; it < group.end(); it++) 
                            {
                                if(it->first==a)
                                {
                                    vector<string >::iterator it2; 
                                    for(it2=it->second.begin() ; it2 < it->second.end(); it2++) 
                                    {
                                        //cout<<"here"<<endl;
                                        string dest=*it2;
                                        messages.push_back(make_pair(b,make_pair(dest,client_details[i].first+" on Group "+a)));
                                    }
                                } 
                            }
                            char *message="\tMessage Sent!\n"; 
                            send(sd , message , strlen(message) , 0 );
                            client_state[i]=99;
                            free(group_message[i]);
                        }
                    }
                }
            }
        }
    }
    return 0;
}