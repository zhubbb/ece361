/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: zhang823
 *
 * Created on September 26, 2017, 3:17 PM
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
/*
 * 
 */



// get sockaddr, IPv4 or IPv6:
void *get_socket_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
//    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int create_server_socket(int portnumber){
    struct sockaddr_in serv_addr;
     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        perror("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     
     
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portnumber);
        
  
    //bind socket to server address 
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) == -1) {
        close(sockfd);
        perror("server: bind");

        exit(-1);
    }
            
        
 

    #define queue_size 3
    if ( (listen(sockfd, queue_size)) == -1) {
        perror("listen");
        exit(1);
    } 
    
     return sockfd;
} 


struct packet{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char* filedata[1000];
};
#define packet_size 2000
int read_buffer(char* output, char*input){
	for(int i=0;i<packet_size;++i){
		if(input[i]!=':'){
			output[i]=input[i];
		}else{
			return i+1;
		}
	}

}
int main(int argc, char** argv) {


    
        int sockfd, new_client_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    
    struct sockaddr_in serv_addr, cli_addr;
    
    struct sockaddr_storage client_addr; // need to store connector's address information
    socklen_t address_len;

    char address_buffer[INET6_ADDRSTRLEN];


    
    
    if(argc<2){
        fprintf(stderr,"ERROR, no port provided\n");
    }
    
    
        
    int portnumber = atoi(argv[1]);
    
    sockfd=create_server_socket(portnumber);
    
       
    
    printf("Server: I am waiting for client connections.\n");
    
#define maxbyte 256
    while(1) {  // listener loop
        address_len = sizeof(client_addr);
        new_client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &address_len);
        if (new_client_fd == -1) {
            perror("Accept phase error.");
            continue;
        }
        inet_ntop(client_addr.ss_family,
            get_socket_addr((struct sockaddr *)&client_addr),
            address_buffer, sizeof(address_buffer));
        printf("Server: I got connection from %s\n", address_buffer);
        if (!fork()) { // this is the child process
            close(sockfd); // close the listener socket child doesn't need the listener
            char buffer[maxbyte];
            int read_bytes;
            read_bytes = recv(new_client_fd, buffer, maxbyte-1, 0);
            if (read_bytes == -1) {
                perror("Server receive phase error.");
                exit(1);
            }else{
                buffer[read_bytes] = '\0';
                printf("Server: I got message: %s\n", buffer);
            }

            
            char packet_buffer[packet_size];
	    bzero(packet_buffer,packet_size);
            if(!strcmp(buffer,"ftp")){
                 send(new_client_fd, "yes", 3, 0);
                  printf("Server: I sent message: %s\n", "yes");
                  char * pch;
                  
                  FILE *f = fopen("receiving", "wb");
                  struct packet packet_info;
                  packet_info.total_frag = -1;
		  char  name[packet_size];
		  bzero(name,packet_size);
                  while((packet_info.frag_no<packet_info.total_frag||packet_info.total_frag==-1)&&(recv(new_client_fd, packet_buffer,packet_size, 0)!=-1)){
                      
                      //printf("%s\n",packet_buffer);
                      int print = (packet_info.total_frag ==-1);
                       char temp_buffer[packet_size];
			bzero(temp_buffer,packet_size);
			int read_byte=0;
			read_byte=read_buffer(temp_buffer,packet_buffer+read_byte);
			//printf("%s\n",temp_buffer);
                        packet_info.total_frag = atoi(temp_buffer);
                   	bzero(temp_buffer,packet_size);
			
                        read_byte+=read_buffer(temp_buffer,packet_buffer+read_byte);
                        packet_info.frag_no = atoi(temp_buffer);
			bzero(temp_buffer,packet_size);
                        
                        
                       
			read_byte+=read_buffer(temp_buffer,packet_buffer+read_byte);
                        packet_info.size = atoi(temp_buffer);
			bzero(temp_buffer,packet_size);

                 
                       read_byte+=read_buffer(temp_buffer,packet_buffer+read_byte);
			
                        memcpy(name,temp_buffer,strlen(temp_buffer));
			packet_info.filename=name;
			bzero(temp_buffer,packet_size);

			if(print){
				printf("Server receiving file :%s\n",packet_info.filename);
				printf("Server receiving total_frag :%d\n",packet_info.total_frag);
				
			}

			printf("Server receiving packet no:%d\n",packet_info.frag_no);
                        if(packet_info.frag_no<packet_info.total_frag ){
                        	fwrite(packet_buffer+read_byte,1,1000,f);
                        }else{
                            fwrite(packet_buffer+read_byte,1,packet_info.size%1000,f);
                        }
                      
                  }
                  fclose(f);
                  rename("receiving", packet_info.filename);
                  if(packet_info.frag_no==packet_info.total_frag){
                      printf("Server received file :%s.\n",packet_info.filename);
                       send(new_client_fd, "ACK", 3, 0);
                  }else{
                      printf("receiving failed.\n");
                      send(new_client_fd, "NACK", 4, 0);
                  }
            }
            else{
                send(new_client_fd, "no", 2, 0);
            }
            
           
              
            close(new_client_fd);
            exit(0);
        }
        close(new_client_fd);  // parent doesn't need this
    }
    
    
    
    
    return (EXIT_SUCCESS);
}

