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


struct lab3message {
unsigned int type;
unsigned int size;
unsigned char source[255];
unsigned char data[255];
};

fd_set readfds_set;

int read_buffer(char* output, char*input){

	for(int i=0;i<255;++i){
		if(input[i]!=' '&&input[i]!='\n'&&input[i]!= '\0'){
			output[i]=input[i];
		}else{
                        output[i]='\0';
			return i+1;
		}
	}

}

void trim(char* s){
    
    int index = strlen(s)-1;
//    printf("%d\n",index);
    while(index>=0&&(s[index]=='\t'||s[index]==' '||s[index]=='\n')){
        s[index]=0;
        index--;
    }
}

//fgets(comment, sizeof comment, stdin);

void *get_socket_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int login(int sockfd,char* user_name, char* passward){
    
    int write_status,read_status;
    char packet_buffer[255];
    char ack_buffer[255];
    bzero(packet_buffer,255);
    strncpy(packet_buffer, "login", 255);
    write_status = send(sockfd,packet_buffer,255,0);
    write_status = send(sockfd,user_name,255,0);	
    write_status = send(sockfd,passward,255,0);	
    read_status = recv(sockfd, ack_buffer, 255, 0);
    if(ack_buffer[0]=='a'){
        printf("login successful.\n");
        return 1;
    }else{
        printf("login failed.\n");
        return 0;
        
    }
    
}

int create_client_socket_connect_server(char* server_name, char* port){
    

    struct sockaddr_in server_address;
    struct addrinfo hints, *servinfo, *p;
    int sockfd;
   
    
    memset(&hints, 0, sizeof hints);
    
//    printf("%s\n",port);
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    int error_type;
    if ((error_type=getaddrinfo(server_name, port, &hints, &servinfo)) != 0) {
        printf("get addr info error\n");
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

int main(int argc, char** argv) {
    
    
    int sockfd;
    
   
  
    
//    if (argc != 3) {
//        fprintf(stderr,"usage: client hostname\n");
//        exit(1);
//    }
    
   
   
    
   

#define maxbyte 256
#define maxline (10*maxbyte)
    char buffer[maxline];
    char username[maxbyte] ;
     char password[maxbyte] ;
    char server_name[maxbyte];
    char port[maxbyte];
    char session_name[maxbyte] ;
    char * line ;
    size_t line_len;
    char command_buffer[maxbyte];
    char ack_buffer[maxbyte];
    int number_bytes=0;
    
    int connected = 0 ;
    
    bzero(buffer,sizeof(buffer));
    bzero(command_buffer,maxbyte);
    bzero(password,maxbyte);
    bzero(username,maxbyte);
    bzero(server_name,maxbyte);
    bzero(port,maxbyte);
    bzero(session_name,maxbyte);
    bzero(ack_buffer,maxbyte);
    int maxfd = 0;
    printf("Please enter your the instruction: \n");
    
    
    while(1){
        
        
        
        
        bzero(buffer,sizeof(buffer));
        if(!connected){
            
            int read_byte=0;
            getline( &line, &line_len, stdin);
        
            read_byte+=read_buffer(command_buffer,line+read_byte);

        
      
        
//               scanf ("%s", command_buffer);
        
//            printf("%s\n",command_buffer); 
            if ( !strcmp(command_buffer,"/login") ) {


    //               scanf ("%s %s %s %s", username,password,server_name,port);
              read_byte+=read_buffer(username,line+read_byte);
              read_byte+=read_buffer(password,line+read_byte);
              read_byte+=read_buffer(server_name,line+read_byte);
              read_byte+=read_buffer(port,line+read_byte);   
//               printf("%s\n",server_name); 
//                printf("%s\n",port); 
              
              sockfd = create_client_socket_connect_server(server_name,port);
              if(sockfd==-1){
                  continue;
              }

              connected= login(sockfd, username, password);


              
                
                
            }else if(!strcmp(command_buffer,"/quit")){
                return 0;

            }else{
                printf("need to login first\n");
            }
        }else{

                        //set select 
              FD_ZERO(&readfds_set);
              FD_SET(0,&readfds_set);
              FD_SET(sockfd,&readfds_set);
              maxfd = sockfd+1;

		
              
            int r = select(maxfd, &readfds_set, NULL, NULL, NULL); 
            
            if (r == -1){
                return 0;
            }
            else if (r==0){
                continue;
            }
            //if sockfd is ready //print message
            if(FD_ISSET(sockfd,&readfds_set)){
                int r = recv(sockfd, buffer, 255, 0);
                if(r<=0){
                    close(sockfd);
                    return 0;
                }else{
                    trim(buffer); 
                    printf("%s\n",buffer);
                }
                
                
            }
            //if stdin is ready if(FD_ISSET(0,&readfds_set))
            else if(FD_ISSET(0,&readfds_set)){
                int read_byte = 0 ;
                getline(&line, &line_len, stdin);
                if(line[0]=='/'){
                    read_byte+=read_buffer(command_buffer,line+read_byte);

                    if(!strcmp(command_buffer,"/logout")){
                        send(sockfd, "logout", 255, 0);
                        send(sockfd, username, 255, 0);
                        printf("logout successful.\n");
          
                        close(sockfd);
                        connected = 0;
                        
                    }else if(!strcmp(command_buffer,"/quit")){
                        printf("need to logout first.\n");
                        
            

                    }
                    else if(!strcmp(command_buffer,"/createsession")){
                        printf("creating session\n");
                         bzero(session_name,maxbyte);
                        read_byte+=read_buffer(session_name,line+read_byte);
                      
                        send(sockfd, "create_session", 255, 0 );
                        
                        send(sockfd, username, 255, 0);
                        send(sockfd, session_name, 255, 0);

                        bzero(ack_buffer,255);
                        recv(sockfd, ack_buffer, 255, 0);
                        if(ack_buffer[0]=='a'){
                            printf("create successful.\n");
                        }else{
                            printf("create failed.\n");
                            
                        }
                       
                    }
                    else if(!strcmp(command_buffer,"/joinsession")){
                        bzero(session_name,maxbyte);
                        read_byte+=read_buffer(session_name,line+read_byte);
                        
                        send(sockfd, "join_session", 255, 0 );
                        
                        send(sockfd, username, 255, 0);
//                        printf("%s \n",session_name);
                        send(sockfd, session_name, 255, 0);
                        
                         bzero(ack_buffer,255);
                        recv(sockfd, ack_buffer, 255, 0);
                        if(ack_buffer[0]=='a'){
                            printf("join successful.\n");
                        }else{
                            printf("join failed.\n");
//                            close(sockfd);
                            
                        }
                        
                        
                       
                    }else if(!strcmp(command_buffer,"/leavesession")){
                       
                        send(sockfd, "leave_session", 255, 0 );
                        
                        send(sockfd, username, 255, 0);
                        
                        bzero(ack_buffer,255);
                        recv(sockfd, ack_buffer, 255, 0);
                        if(ack_buffer[0]=='a'){
                            printf("leave successful.\n");
                        }else{
                            printf("leave failed.\n");
                            
                        }
                       
                    }else if(!strcmp(command_buffer,"/list")){
                        
                        send(sockfd, "list", 255, 0 );
            

                    }else if(!strcmp(command_buffer,"/invite")){
                        char invitee[255];
                        bzero(invitee,255);
                        read_byte+=read_buffer(invitee,line+read_byte);
                        send(sockfd, "invite", 255, 0 );
                        send(sockfd, username, 255, 0);
                        send(sockfd, invitee, 255, 0);

                    }else if(!strcmp(command_buffer,"/accept")){
                        
                        send(sockfd, "accept", 255, 0 );
                        send(sockfd, username, 255, 0);

                    }else if(!strcmp(command_buffer,"/reject")){
                        
                        send(sockfd, "reject", 255, 0 );
                        send(sockfd, username, 255, 0);

                    }else{
                        printf("invalid command.\n");
                    }
                }else{
                    send(sockfd, "message", 255, 0 );
                    send(sockfd, username, 255,0);
                    send(sockfd, line, 255, 0);
                    
                }
                
                
            }
            
            
            
        }
    
        
    }
     
  
        close(sockfd);
        return (EXIT_SUCCESS);
        
        
        
        
    
    
   
  
    
    
   
}

/*
 
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
 */