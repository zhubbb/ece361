#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

void main(int argc, char** argv) {

       FILE* file=fopen("cat.bmp", "r");
  
if(file==NULL){
	printf("sss");
}
    
        int file_size= 0;
        fseek(file, 0L, SEEK_END);
        file_size = ftell(file);
         fseek(file, 0L, SEEK_SET);

        size_t read_size=0;
        char file_buffer[1000];

        char* packet_buffer = malloc(1200);
        bzero(packet_buffer,1200);
        int packet_no=1;

        int total_frag= file_size/1000;
        int write_byte=0;
	char number_buffer[4];
	//bzero(number_buffer,4);
	if(file_size%1000){
		total_frag+=1;
	}

//printf("Client sending %d \n",total_frag);
	//sprintf(number_buffer,"%d",total_frag);
	//fclose(file);
}
