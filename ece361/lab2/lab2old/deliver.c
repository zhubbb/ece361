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

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv) {
    
    
    int sockfd, port_num;
    
    struct sockaddr_in server_address;
    struct addrinfo hints, *servinfo, *p;
    
    if (argc != 3) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    char* port = argv[2];
    int rv;
    
    if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol) ;
        
        if ( sockfd == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    
    char s[INET6_ADDRSTRLEN];
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
#define maxbyte 256
    char buffer[maxbyte];
    int number_bytes=0;
    
    
    printf("Please enter the message: ");
 
   
     
    
    
    
    bzero(buffer,maxbyte);
    char filename[maxbyte] ;
    bzero(filename,maxbyte);
//    fgets(buffer,maxbyte-1,stdin);
    scanf ("%s", buffer);
    scanf ("%s", filename);
    
//    char filename[maxbyte] ;
//    memcpy( filename, &buffer[4], maxbyte-4 );
     FILE * file;
     printf("%s\n",filename);
    if ( file = fopen(filename, "r")) {
        fclose(file);
        printf("file exists.\n");
        
        int n = write(sockfd,buffer,strlen(buffer));
    
        if (n < 0) 
             perror("ERROR writing to socket");
        bzero(buffer,256);
    }
    
    else{
        
        printf("file is not found. exit.\n");
        close(sockfd);
        return 0;
        
    }
  
    
    
//receiving message from server
number_bytes = recv(sockfd, buffer, maxbyte-1, 0);
if (number_bytes == -1) {
    perror("receive.");
    exit(1);
}

buffer[number_bytes] = '\0';
 
printf("client: received '%s'\n",buffer);

close(sockfd);
    return (EXIT_SUCCESS);
}

