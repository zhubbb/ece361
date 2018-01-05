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
struct lab3message {
unsigned int type;
unsigned int size;
unsigned char source[255];
unsigned char data[255];
};


struct client{
    char uid[255];
     int socketfd;
     int session_id;
    struct client* next;
    int invite_status;
};

struct session{
    char session_name[255];
    int in_use;
    int id;
    struct client* client_list;
};

struct info{
    char username[255];
    char password[255];
};

//data section to store hard-coded user information
#define user_num 3
struct info userinfo[user_num] = { {"aaa","123"}, {"bbb","123"}, {"ccc","123"} };



struct client_link{
    struct client* c;
    struct client_link* next;
};


#define max_session 10
struct session sessions[max_session] = {"\0",0,-1,NULL};
struct client_link active_users = {NULL,NULL};

void exit_session(struct client* c);

struct session* get_session(char * session_id){
    for(int i = 0 ;i < max_session; i++){
        if(sessions[i].in_use && !strcmp(sessions[i].session_name, session_id)){
            return &sessions[i];
        }
    }
    return NULL;
}

void join_session(struct client* c){
    struct session* s;
    if(c->session_id<0){
        return;
    }
    s = &sessions[c->session_id];
    struct client* head = (s->client_list);
    s->client_list = c;
    c->next = head;
    
}
void exit_session(struct client* c){
    struct session* s;
    if(c->session_id<0){
        return;
    }
    s = &sessions[c->session_id];
    struct client* current = (s->client_list);
    struct client* previous = NULL;
    for( ; current != NULL ; current = current->next, previous =  current ){
        if(!strcmp(current->uid,c->uid) ){
            if(previous == NULL){
                s->client_list = c->next;
            }else{
                previous->next = c->next;
            }
        }
    }
    c->session_id=-1;
    if(s->client_list==NULL){
        s->in_use = 0;
    }
}

void activate_user(struct client* c){
    exit_session(c);
    struct client_link * next = active_users.next;
    struct client_link* new_link = malloc(sizeof(struct client_link));
    new_link->c = c;
    active_users.next = new_link;
    new_link->next = next;
}
void deactivate_user(struct client* c){
    struct client_link* previous = &active_users;
    struct client_link* current = active_users.next;
    for(; current != NULL ; current = current->next){
        if(!strcmp(c->uid,(current->c->uid))){
            previous->next = current->next;
            free(current->c);
            free(current);
        }
    }
}
int create_session(char * session_name){
    if(get_session(session_name)!=NULL){
        return -1;
    }
    for(int i = 0 ;i <max_session;i++){
        if(!sessions[i].in_use){
            sessions[i].in_use  = 1;
            bzero(sessions[i].session_name,255);
            strcpy((sessions[i].session_name),session_name) ;
            sessions[i].id = i;
           
            return i;
        }
    }
    return -1;
    
}
struct client* get_client(char * username);
int  create_client(int socketfd, char * username){
    if(get_client(username)==NULL){
        struct client* c = malloc(sizeof(struct client)); 
        bzero(c->uid,255);
        strcpy(c->uid,username);
        c->socketfd = socketfd;
        c->session_id = -1;
        c->invite_status=-1;
        activate_user(c);
        return 1;
    }
    return 0;
}

struct client* get_client(char * username){
    
    struct client_link* current =(active_users.next);
    for(; current != NULL ; current = current->next){
        if(!strcmp(username,(current->c->uid))){
            return current->c;
        }
    }
    return NULL;
    
    
}
void list(int socketfd){
    send(socketfd, "User list:", 255, 0);
    struct client_link* current = active_users.next;
    
    for(; current != NULL ; current = current->next){
        send(socketfd, current->c->uid , 255, 0);
//        send(socketfd, sessions[(current->c->session_id)].session_name , 255, 0);
    }
    send( socketfd,"-----------",255,0);
    send(socketfd, "Session list:", 255, 0);
    for(int i = 0 ;i <max_session;i++){
        if(sessions[i].in_use){
            send(socketfd,sessions[i].session_name,255,0) ;
        }
    }
    send( socketfd,"-----------",255,0);
}

void broadcast(struct client* c, char* message){
    if(c->session_id<0){
        return ;
    }
    
    struct session* s = &sessions[c->session_id];
    
    struct client* current = (struct client*)(s->client_list);
    for( ; current != NULL ; current = current->next){
        if(strcmp(c->uid,current->uid)){
            send(current->socketfd, message , 255, 0);
        }
    }
}

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

	for(int i=0;i<255;++i){
		if(input[i]!=' '&&input[i]!='\n'&&input[i]!= '\0'){
			output[i]=input[i];
		}else{
                        output[i]='\0';
			return i+1;
		}
	}

}

int main(int argc, char** argv) {
    
    fd_set master;
    fd_set read_fds;
    
    
    FD_ZERO(&master); 
    FD_ZERO(&read_fds);
    
    
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
    FD_SET(sockfd, &master);
    int fdmax = sockfd;
       
    
    printf("Server: I am waiting for client connections with socket %d.\n",sockfd);
    
#define maxbyte 256
    while(1) {  // listener loop
        int read = 1;
        read_fds = master;
        
         if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
         }
        for(int i=0; i<=fdmax; i++) {
            if (FD_ISSET(i,&read_fds)){
             if(i==sockfd){//create mew socket if it is listener that is ready
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
                FD_SET(new_client_fd, &master);
                if (new_client_fd > fdmax) { 
                    fdmax = new_client_fd;
                }
                

             }else{
                 //dealing with client socket
                 char buffer[maxbyte];
                bzero(buffer,maxbyte);
                int read_bytes;
                read_bytes = recv(i, buffer, 255, 0);
                if (read_bytes == 0) {
                    goto logout;
                }
                if (read_bytes == -1) {
                    perror("Server receive phase error.\n");
                    exit(1);
                }else{
                    buffer[read_bytes] = '\0';
                    printf("Server: I got message: %s\n", buffer);
                }


                char packet_buffer[packet_size];
                bzero(packet_buffer,packet_size);



                 char  username[255];
                bzero(username,255);
                char session[255];
                bzero(session,255);
                if(!strcmp(buffer,"login")){
    //                  send(new_client_fd, "yes", 3, 0);
    //                  printf("Server: I sent message: %s\n", "yes");

                      FILE *f ; 
                      struct packet packet_info;

                      char  password[255];
                      bzero(password,255);
                      recv(i, username, 255, 0);
                      read = recv(i, password, 255, 0);
                      if(read<=0) {goto logout;}
                      int found =0;
                      for(int i  = 0 ; i < user_num; i++ ){

                          if(!strcmp(username,userinfo[i].username)){

                              if(!strcmp(password,userinfo[i].password)){
                                found = 1;     
                              }
                          }
                      }
                      if(found&& create_client( i,username)){
//                          read_buffer(username,username);
                          
                          send(i, "ack", 255, 0);
                      }else{
                          send(i, "nack", 255, 0);
                      }




                }else if(!strcmp(buffer,"create_session")){
               
                    bzero(username,255);
                    recv(i, username, 255, 0);
//                     read_buffer(username,username);
                    bzero(session,255);
                    read=recv(i, session, 255, 0);
                    if(read<=0) {goto logout;}
//                     read_buffer(session,session);
                    int session_id = create_session(session);
//                     printf("%s %d\n",(sessions[session_id].session_name), (sessions[session_id].in_use));
                    struct client * c = get_client(username);
                    exit_session(c);
                    c->session_id = session_id;
                    join_session(c);
                    if(session_id>=0){
                        send(i, "ack", 255, 0);
                    }else{
                        send(i, "nack", 255, 0);
                    }
                    
                }else if(!strcmp(buffer,"join_session")){
                    recv(i, username, 255, 0);
                    read = recv(i, session, 255, 0);
                    if(read<=0) {goto logout;}
                    struct client * c = get_client(username);
                    struct session* s =get_session(session);
                    if(s!=NULL){
                        exit_session(c);
                        c->session_id = get_session(session)->id;
                        join_session(c);
                        send(i, "ack", 255, 0);
                    }
                    else{
                        send(i, "nack", 255, 0);
                    }
                }else if(!strcmp(buffer,"leave_session")){
                    read= recv(i, username, 255, 0);
                    if(read<=0) {goto logout;}
                    struct client * c = get_client(username);
                   
                    exit_session(c);
                    c->session_id = -1;
                    if(c->session_id==-1){
                        send(i, "ack", 255, 0);
                    }
                    else{
                        send(i, "nack", 255, 0);
                    }
                }else if(!strcmp(buffer,"message")){
                    char  message[255];
                    bzero(message,255);
                    recv(i, username, 255, 0);
                    read = recv(i, message, 255, 0);
                    if(read<=0) {goto logout;}
                    struct client * c = get_client(username);
                    printf("message is :%s\n",message);
                    char m[255];
                    bzero(m,255);
                    strcat(m,username);
                    strcat(m,": ");
                    strcat(m,message);
                    broadcast(c,m);
                    //send(new_client_fd, "braocast ack", 255, 0);

                }else if(!strcmp(buffer,"list")){
                    list(i);
                }else if(!strcmp(buffer,"invite")){
                    char invitee[255];
                    bzero(invitee,255);
                    recv(i, username, 255, 0); 
                    recv(i, invitee, 255, 0); 
                    struct client * c1 = get_client(username);
                    if(c1->session_id>=0 && invitee[0]!='\0'){
                        struct client * c2 = get_client(invitee);
                        char message[255];
                        bzero(message,255);
                        sprintf(message,"You are invited by %s to join a session.(Reply /accept or /reject)\n",username);
                        send(c2->socketfd, message , 255, 0);
                        c2->invite_status = c1->session_id;
                    }
                    
                    
                }else if(!strcmp(buffer,"accept")){
                    recv(i, username, 255, 0); 
                    struct client * c = get_client(username);
                    if(c->invite_status>=0){
                        exit_session(c);
                        c->session_id= c->invite_status;
                        c->invite_status = -1;
                        
                        join_session(c);
                        char message[255];
                        bzero(message,255);
                        sprintf(message,"User %s joined the session.\n",username);
                        broadcast(c,message);
                        send(i, "You accept the invite." , 255, 0);
                        
                    }
                }else if(!strcmp(buffer,"reject")){
                    recv(i, username, 255, 0); 
                    struct client * c = get_client(username);
                    if(c->invite_status>=0){
                        int session_id = c->session_id;
                        c->session_id=c->invite_status;
                        char message[255];
                        bzero(message,255);
                        sprintf(message,"User %s reject the invitation.\n",username);
                        broadcast(c,message);
                        c->invite_status = -1;
                        c->session_id = session_id;
                        send(i, "You reject the invite." , 255, 0);
                    }
                }else if(!strcmp(buffer,"logout")){
                    recv(i, username, 255, 0);                    
                    struct client * c = get_client(username);
                    deactivate_user(c);
                    logout:
                    close(i);
                    FD_CLR(i, &master); 
                }
                 
             }
            }
            
            
        }
       
        
        
        
//        if (!fork()) { // this is the child process
//            close(sockfd); // close the listener socket child doesn't need the listener
//            int done =0;
//            while(!done){
//                
//               
//            
//            }
//            
//              
//            close(new_client_fd);
//            exit(0);
//        }
//        close(new_client_fd);  // parent doesn't need this
    }
    
    
    
    
    return (EXIT_SUCCESS);
}

/*
  while((acked_seq_number<packet_info.total_frag||packet_info.total_frag==-1)&&(recv(new_client_fd, packet_buffer,packet_size, 0)>0)){
                      
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

			if(packet_info.frag_no==1&&acked_seq_number==0){
				f = fopen(packet_info.filename, "wb");
			}
			if(print){
				printf("Server receiving file :%s\n",packet_info.filename);
				printf("Server receiving total_frag :%d\n",packet_info.total_frag);
				
			}

			printf("Server receiving packet no:%d\n",packet_info.frag_no);
			if(debug){
				sleep(1);
				debug=0;
			}
			
			char send_message[255];
			bzero(send_message,255);
			sprintf(send_message,"ACK:%d",packet_info.frag_no);
			//printf("strlen:%d\n",strlen(send_message));

			send(new_client_fd, send_message, strlen(send_message), 0);
			printf("Server sent:%s\n",send_message);
			printf("Server ACK packet no:%d\n\n",packet_info.frag_no);
			//drop dup packet
			if(packet_info.frag_no == acked_seq_number+1){
				acked_seq_number ++;
			
		                if(packet_info.frag_no<packet_info.total_frag ){
		                	fwrite(packet_buffer+read_byte,1,1000,f);
		                }else{
		                    fwrite(packet_buffer+read_byte,1,packet_info.size%1000,f);
		                }
			}
    }
 */