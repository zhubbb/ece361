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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
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

struct packet{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char* filedata[1000];
};

#define packet_size 2000
void send_file(FILE* file,char* filename,int sockfd){
   
       //file=fopen(filename, "rb");

  	char ack[5];
	bzero(ack,5);
    
        int file_size= 0;
        fseek(file, 0L, SEEK_END);
        file_size = ftell(file);
         fseek(file, 0L, SEEK_SET);

        size_t read_size=0;
        char file_buffer[1000];

        char* packet_buffer = malloc(packet_size);
        bzero(packet_buffer,packet_size);
        int packet_no=1;

        int total_frag= file_size/1000;
        
	char number_buffer[packet_size];
	bzero(number_buffer,packet_size);
	if(file_size%1000){
		total_frag+=1;
	}


	
        
       
        
        printf("Client sending file  %s with size %d\n",filename,file_size);
        
        
        while ((read_size = fread(file_buffer,1, 1000, file)) > 0){
            	int write_byte=0;
            	sprintf(number_buffer,"%d",total_frag); // print integer into string 

		memcpy(packet_buffer+write_byte,number_buffer,strlen(number_buffer));
		write_byte += strlen(number_buffer);
		packet_buffer[write_byte]=':';
		write_byte++;
		bzero(number_buffer,packet_size);
           
           	printf("Client sending file packet %d\n",packet_no);
		sprintf(number_buffer,"%d",packet_no);
 		
		memcpy(packet_buffer+write_byte,number_buffer,strlen(number_buffer));
		write_byte += strlen(number_buffer);
		packet_buffer[write_byte]=':';
		write_byte++;
		bzero(number_buffer,packet_size);


           	sprintf(number_buffer,"%d",read_size);
		memcpy(packet_buffer+write_byte,number_buffer,strlen(number_buffer));
		write_byte += strlen(number_buffer);
		packet_buffer[write_byte]=':';
		write_byte++;
		bzero(number_buffer,packet_size);
           
            
           
            
            
            	memcpy(packet_buffer+write_byte,filename,strlen(filename));
		write_byte += strlen(filename);
		packet_buffer[write_byte]=':';
		write_byte++;
		
             
//            fwrite(file_buffer, 1, read_size, stdout);
            	memcpy(packet_buffer+write_byte,&file_buffer,read_size);
            
		//printf("%d\n",read_size+write_byte);
		
		//if timeout, resent the packet
		int read_status=-1;
		int write_status=-1;
		int y =0 ;
		char ack_buffer[255];
		int resent = 1;
		while(resent){
			bzero(ack,5);
			bzero(ack_buffer,255);
		   	write_status = send(sockfd,packet_buffer,packet_size,0);
			
			read_status = recv(sockfd, ack_buffer, 255, 0);
			int i = 0;
			while(ack_buffer[i]!=':' && ack_buffer[i]!='\0'){ 
				ack[i]=ack_buffer[i];
				i++;
			}
			char c[255];
			memcpy(c,&ack_buffer[i+1],255-i);
			int ack_number = atoi( c);
			
 			
			if(read_status < 0 && errno == EAGAIN){
				printf("Timeout reached. Resending packet: %d.\n",packet_no);
				resent = 1;
			}
			else if(strcmp(ack,"ACK")==0 && ack_number == packet_no){
				printf("ACK received for packet: %d.\n\n",packet_no);
				resent = 0;
			}
			else{
				printf("NACK received. Resending packet: %d.\n",packet_no);
				resent = 1;
			}
		}
		
             packet_no++;
            
             
        
        
             
        }
	
        printf("Client sent file.\n");
        fclose(file);
	free(packet_buffer);
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
    file = fopen(filename, "r");
    if ( !strcmp(command_buffer,"ftp") && file ) {
        
        
       // fclose(file);
        printf("file exists.\n");
      
        
        
        double msec = 0;
        clock_t timer_start=clock();
        int write_status = send(sockfd,command_buffer,strlen(command_buffer),0);
    
        if (write_status < 0) 
             perror("ERROR writing to socket");
//        bzero(buffer,256);
//        bzero(command_buffer,maxbyte);
//        bzero(filename,maxbyte);
         //receiving message from server
        number_bytes = recv(sockfd, buffer, maxbyte-1, 0);
        clock_t timer_diff = clock()-timer_start;
        
        msec=(double)timer_diff*1000/CLOCKS_PER_SEC;
        
        if (number_bytes == -1) {
            perror("receive phase error.");
            exit(1);
        }

        buffer[number_bytes] = '\0';

        printf("client: received '%s'\n",buffer);
        printf("Round Time trip is %.4f msec\n",msec);
	
	//set timeout base on RTT*10
	struct timeval timeout;
	timeout.tv_sec = (int)(msec*10/1000.0);  
	timeout.tv_usec = (int)(msec*10*1000)%1000000;  
	
	printf("Client set timeout as %d seconds, %d useconds\n",timeout.tv_sec,timeout.tv_usec);
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char *)&timeout,sizeof(timeout)) < 0) {
    		perror("Error");
	}


        printf("client: A file transfer can start.\n");
        
        
         
        
        send_file(file,filename,sockfd);
        
        //number_bytes = recv(sockfd, buffer, maxbyte-1, 0);
        //buffer[number_bytes] = '\0';
        //printf("client: received '%s'\n",buffer);
        
        close(sockfd);
        return (EXIT_SUCCESS);
        
        
        
        
    }
    
    else{
        
        printf("file is not found. exit.\n");
        close(sockfd);
        return 0;
        
    }
  
    
    
   
}

