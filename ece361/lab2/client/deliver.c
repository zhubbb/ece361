/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   deliver.c
 * Author: zhang823
 *
 * Created on September 26, 2017, 4:16 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

/*
 * 
 */

void *get_socket_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int create_client_socket_connect_server(char* server_name, char* port){
    
    struct sockaddr_in server_address;
    struct addrinfo hints, *servinfo, *p;
    int sockfd;
   
    
    memset(&hints, 0, sizeof hints);
    
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    int error_type;
    if ((error_type=getaddrinfo(server_name, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "get addr info error: %s\n",error_type);
        return 1;
    }
    int connect_status=-1;
    for(p = servinfo; p != NULL&&connect_status==-1; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol) ;
        
        if ( sockfd == -1) {
            perror("Client: socket phase error");
            continue;
        }
        
        connect_status = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if (connect_status == -1) {
            close(sockfd);
            perror("Client: connect phase error");
            continue;
        }
        break;
    }
    
    if (connect_status == -1) {
        fprintf(stderr, "client: failed to connect\n");
        return -1;
    }
    
    char address_buffer[INET6_ADDRSTRLEN];
    
    inet_ntop(p->ai_family, get_socket_addr((struct sockaddr *)p->ai_addr),
            address_buffer, sizeof(address_buffer));
    printf("Client: I am connecting to %s\n", address_buffer);
    return sockfd;
}


int main(int argc, char** argv) {
    
    
    int sockfd;
    
   
  
    
    if (argc != 3) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    
   
   
    char* server_name = argv[1];
    char* port = argv[2];
  
    sockfd = create_client_socket_connect_server(server_name,port);
    if(sockfd==-1){
        return -1;
    }

#define maxbyte 256
    char buffer[maxbyte];
    char filename[maxbyte] ;
    char command_buffer[maxbyte];
    int number_bytes=0;
    
    
    
    
    printf("Please enter your the instruction: ");
 
   
    
    bzero(buffer,maxbyte);
    bzero(command_buffer,maxbyte);
    bzero(filename,maxbyte);
    
    scanf ("%s", command_buffer);
    scanf ("%s", filename);
     
    FILE * file;
    if ( !strcmp(command_buffer,"ftp") && (file = fopen(filename, "r")) ) {
        
        
        fclose(file);
        printf("file exists.\n");
      
        int write_status = send(sockfd,command_buffer,strlen(command_buffer),0);
    
        if (write_status < 0) 
             perror("ERROR writing to socket");
        bzero(buffer,256);
        bzero(command_buffer,maxbyte);
        bzero(filename,maxbyte);
    }
    
    else{
        
        printf("file is not found. exit.\n");
        close(sockfd);
        return 0;
        
    }
  
    
    
    //receiving message from server
    number_bytes = recv(sockfd, buffer, maxbyte-1, 0);
    if (number_bytes == -1) {
        perror("receive phase error.");
        exit(1);
    }

    buffer[number_bytes] = '\0';
    
    printf("client: received '%s'\n",buffer);
    printf("client: A file transfer can start.\n");

    close(sockfd);
        return (EXIT_SUCCESS);
}

