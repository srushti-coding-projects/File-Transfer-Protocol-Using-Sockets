/*
 * Client code that requests file from the server and acknowledges data packets and sends to server.*/


#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include "Packet.h"
#include <fcntl.h>
#include <iostream>
#include <fstream>

using namespace std;


int main (int argc, char *argv[])  // server address, port number and file path is sent from the command line arguments
{
        //declarations
	int client_handler, filename_size, recv_len, handshake=0,prev_seq = 0;
        struct sockaddr_in sock_info;
	socklen_t sock_info_len = sizeof(sock_info);
	char filename[50], buffer[PACKET_SIZE], output_file[50], ack_buffer[PACKET_SIZE];
	packet syn, data_pack, data_ack,syn_ack,ack;

	
	// check all arguments are provided
	if (argc < 3){
                cout<<"Arguments needed.";
                exit(1);
        }

	// create client socket	
	client_handler = socket(AF_INET,SOCK_DGRAM,0);		
        if(client_handler == -1){
                cout<<"Creation of client socket failed";
                exit(1);
        }
        
	// server information in the socket address
	sock_info.sin_family = AF_INET;			
        sock_info.sin_port = htons(atoi(argv[2]));
        sock_info.sin_addr.s_addr = htonl(INADDR_ANY);

        int i=0,j=0,k=0;
	char c[50];
	
	// parse file name from file path in the argument
	strcpy(filename,argv[3]);
	filename[strlen(filename) + 1] = '\0';

	i = strlen(filename) - 1;

        while(filename[i] != '/'){
                output_file[j] = filename[i];
                i--;
                j++;
        }
	output_file[j] = '\0';
        
        for(i = strlen(output_file) - 1; i >= 0; i--){
                c[k] = output_file[i];
                k++;
        }
        c[k]='\0';
	
	// Establish a 3-way handshake
	
	filename_size = strlen(c);

	init_buffer(buffer, &syn);
	
	create_packet(&syn , S, -1, filename_size,c);	
	
	packet_to_memory(buffer,&syn);
	
	// send SYN request to the server along with filename
	sendto(client_handler, buffer, PACKET_SIZE,0, (const struct sockaddr *) &sock_info,sizeof(sock_info));
	
	memory_to_packet(buffer, &syn);
	
	if(syn.type == S){
            recv_len = recvfrom(client_handler, buffer, PACKET_SIZE, 0, (struct sockaddr *)&sock_info, &sock_info_len);
	    memory_to_packet(buffer, &syn_ack);

        }
	if(syn_ack.type == C){
		cout<<"\nHandshake Failed.";
		return 0;
	}
	if(syn_ack.type == R && syn_ack.data_length > 0){
		ack.type = W;
		packet_to_memory(buffer, &ack);
		sendto(client_handler, buffer, PACKET_SIZE,0, (const struct sockaddr *) &sock_info,sizeof(sock_info));
		handshake = 1;
	}

	cout<<"\n\nHandshake Established."<<handshake;

	
	if(handshake == 1){
	
		strcat(c,".out");
        	ofstream fp;
        	fp.open("client_server.txt");
	
	
		//create file to write
		int recv_fp = open(c, O_WRONLY | O_CREAT | O_APPEND , 0644);
		
		while(1){
		
			// receive data packets
			recv_len = recvfrom(client_handler, buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&sock_info, &sock_info_len);
			memory_to_packet(buffer,&data_pack);
			

			if(data_pack.type == C){
				break;
			}


			// write data in file		
                	if(prev_seq+1 == data_pack.seq_num){
                   		write(recv_fp,data_pack.data,data_pack.data_length);
                   		cout<<"\nclient recieved packet "<<data_pack.seq_num;	
                   		fp<<data_pack.seq_num<<"\n";
                	}


                	//send ack packet
                	data_ack.type = A;
                   	data_ack.seq_num = data_pack.seq_num;
                   	packet_to_memory(ack_buffer,&data_ack);
                   	sendto(client_handler,ack_buffer, PACKET_SIZE,MSG_CONFIRM, (const struct sockaddr *) &sock_info,sizeof(sock_info));
		   	prev_seq = data_pack.seq_num;
                
		
		}// while
	
		cout<<"\n\n File Recieved by client.";
		
		close(recv_fp);
        
		close(client_handler);
	
        	fp.close();

	}//if
	else{
		close(client_handler);
	}
	close(client_handler);

return 0;
}
