/* Header file to define structures and memory to buffer and buffer to memory conversion. */


#include "constants.h"
#include <string.h>
#include <netinet/in.h>


// transmission packets stuctures
typedef struct packet{
	p_type type;
	int seq_num;
	int data_length;
	char data[DATA_SIZE];
	
}packet;


typedef struct Timeout{
	int *handler;
	char buffer[PACKET_SIZE];
	struct sockaddr_in *timer_sock;

}Timeout;


// Initialize buffer
void init_buffer(char * buffer, packet *p){
	memset(buffer , 0 , PACKET_SIZE);
	memset(&(p -> data), 0 , DATA_SIZE);
}


// insert values into packets 
void create_packet(packet *p, p_type type, int sn, int size, char * data){
	p -> type = type;
	p -> seq_num = sn;
	p -> data_length = size;
	strcpy(p -> data, data);
}


// put buffer data into memory
void packet_to_memory(char * buffer, packet *p){

        memcpy(buffer, &(p -> type), sizeof(p_type));
        memcpy(buffer + SEQ_LOC, &(p -> seq_num),sizeof(int));
        memcpy(buffer + DATA_SIZE_LOC, &(p -> data_length), sizeof(int));
	if(p -> seq_num == -1){

		memcpy(buffer + DATA_LOC, &(p -> data), strlen(p -> data));
	}
	else
	{
        	memcpy(buffer + DATA_LOC, &(p -> data), DATA_SIZE);
	}

}


// get buffer data from memory
void memory_to_packet(char * buffer, packet *p){

        memcpy(&(p -> type),buffer, sizeof(p_type));
        memcpy(&(p -> seq_num),buffer + SEQ_LOC,sizeof(int));
        memcpy(&(p -> data_length),buffer+DATA_SIZE_LOC,sizeof(int));
        memcpy(&(p -> data),buffer + DATA_LOC,DATA_SIZE);

}
