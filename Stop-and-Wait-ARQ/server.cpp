/* 
 * Implementation of Stop-and-Wait ARQ flow control protocol using UDP sockets for transferring files.
 * In this code, I have implemented a reliable file transfer protocol on a simulated unreliable channel.
 * */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include "Packet.h"
#include <iostream>
#include <fstream>
#include <mutex>
#include <pthread.h>

using namespace std;

/*
 * Packet types:
 * S - synchronization request
 * R - synchronization reply resonse
 * C - close connection response
 * W - Acknowledgement 
 * D - data packets
 * */

long int get_file_size(char file_name[]) /* This functions returns the size of the inputted file in bytes */
{
    // opening the file in read mode
    FILE* fp = fopen(file_name, "r");

    // checking if the file exist or not
    if (fp == NULL) {
        printf("File Not Found!\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);

    // calculating the size of the file
    long int res = ftell(fp);

    // closing the file
    fclose(fp);

    return res;
}

void send_method (Timeout *p, int i){ /* function sends data packets to client */
	
	sendto(*(p->handler), p->buffer, PACKET_SIZE,MSG_CONFIRM, (const struct sockaddr *) p->timer_sock,sizeof(*(p->timer_sock)));
 }


int main (int argc, char *argv[])  // port number of server is given through arguments
{
        int  handshake = 0, prev_seq, server_handler,seq_count = 0,recv_len,read_length,resend = 0;
	long int filesize;
        char ack_buffer[PACKET_SIZE], buffer[PACKET_SIZE];
	
	// structure for socket address information
	struct sockaddr_in sock_info;
	
	// declare diiferent packets for three way handshake : SYN, SYN-ACK, ACK
	packet syn, data_packet, data_ack, syn_ack, ack;
	Timeout t;

	// initialize buffer with zero 
	memset(buffer, 0 ,PACKET_SIZE);

        // length of socket address structure
	socklen_t sock_info_len= sizeof(sock_info);
      
	// check if command line arguments are given
        if (argc < 1){
                printf("Arguments needed.");
                exit(1);
        }

	// create a UDP socket 
        server_handler = socket(AF_INET,SOCK_DGRAM,0);
        if(server_handler == -1){
                printf("Creation of server socket failed");
                exit(1);
        }
        sock_info.sin_family = AF_INET;
        sock_info.sin_port = htons(atoi(argv[1]));
        sock_info.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind socket to the address and port in socket address object
	if(bind(server_handler, (struct sockaddr *)&sock_info, sizeof(sock_info)) < 0){
                printf("Bind failed.");
                exit(1);
        }
	
	// attach server socket to timer object
	t.handler = &server_handler;
	t.timer_sock = &sock_info;	



	// Part I: Establish Three way establish
	// blocking receive - waits to accept request from client
	recv_len = recvfrom(server_handler, buffer, PACKET_SIZE,0, (struct sockaddr *)&sock_info, &sock_info_len);
	if (recv_len > 0){
		// put received information into character array from memory
		memory_to_packet(buffer, &syn);
                        
        }
	// check if the request is SYN - synchronization request
	if(syn.type == S){
		// get file size
		filesize = get_file_size(syn.data);
		
		if(filesize == -1){ // check if file opening returns -1
			
			// send close packet because error occured while opening file
			// handshake cannot be established
			syn_ack.type = C;
			packet_to_memory(buffer, &syn_ack);
                	sendto(server_handler, buffer, PACKET_SIZE,0, (const struct sockaddr *) &sock_info,sizeof(sock_info));
			cout<<"\nHandshake failed. File not found.";
			return 0;

		}
		else{
			// on successful file size calculation: return reply packet with filesize
			syn_ack.type = R;
                	syn_ack.data_length = filesize;
		}
	 	
		// put request packet in memory and send	
		packet_to_memory(buffer, &syn_ack);
		sendto(server_handler, buffer, PACKET_SIZE,0, (const struct sockaddr *) &sock_info,sizeof(sock_info));
	}

	// wait to recieve acknowledgment
	recv_len = recvfrom(server_handler, buffer, PACKET_SIZE,0, (struct sockaddr *)&sock_info, &sock_info_len);
	if (recv_len > 0){

                        memory_to_packet(buffer, &ack);
                        
			// if ack is received set handshake flag
			if(ack.type == W){
				handshake = 1;
			}

        }
	cout <<"\n\nHandshake Established.."<<handshake;

        // timeval structure to define timeout values to implement non-blocking receive
	struct timeval read_timeout;
	read_timeout.tv_sec = 0;
        read_timeout.tv_usec = 400000;
        
	// for send and recieve timeouts
	setsockopt(server_handler, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));	
        
	if (setsockopt(server_handler, SOL_SOCKET, SO_RCVTIMEO,&read_timeout,sizeof(read_timeout)) < 0) {
       		cout<<"error";
        }

	// Sending file into smaller data packets
	if(handshake == 1){

		int send_fp = open(syn.data, O_RDONLY);

		// check file opening error
		if (send_fp < 0){

			cout<<"Error while reading the file to send.";
		}

              
		while(1){ // read file contents and send data packets to client
			
			if(resend == 0){ // continue reading file        
				
				read_length = read(send_fp, data_packet.data, DATA_SIZE);
                        }
                               
			resend = 0;
				
			if(read_length == 0 ){ // check if file is completed
                                	
				// server sends close connection packet
                                data_packet.type = C;
                                data_packet.seq_num = seq_count + 1;
                                packet_to_memory(t.buffer, &data_packet);
			                
	                       	send_method(&t,0);

			        // waits to receive the acknowledgement of last packet from client
				recv_len = recvfrom(server_handler,ack_buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&sock_info, &sock_info_len);
			                
				memory_to_packet(ack_buffer, &data_ack);
                                
				// check sent and recieved sequence number of packet	
				if (recv_len > 0 || data_ack.seq_num == data_packet.seq_num){
			        	
					// stop sending
				       	cout<<"\n\n File reading and sending completed.";
					break;
                                         
				}
				else{
                                      	
					// if last packet is dropped. resend the packet
                                        resend =1;
                                        seq_count = seq_count - 1;

                                }
                                
                       	} // end if 
                              
			
			// continue sending file contents. increment sequence number for every packet
			seq_count = seq_count + 1;

                        data_packet.type = D;
                        data_packet.data_length = read_length;
                        data_packet.seq_num = seq_count;

			//prepare packet to send if timeout occurs
			
			packet_to_memory(t.buffer,&data_packet);
			
			// send packet
			send_method(&t,0);
		
			// wait to receive acknowledgement
			recv_len = recvfrom(server_handler,ack_buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&sock_info, &sock_info_len);
			
			memory_to_packet(ack_buffer, &data_ack);
                        
			/*If data packets or acknowlegements are dropped then resend the packets. Timer starts while 
			 * waiting to receive acknowledgments. If ack is not received within time then resend the same 
			 * data packet  */

			if (recv_len<= 0 || data_ack.seq_num != data_packet.seq_num) {
				
                           	resend =1;
                          	seq_count = seq_count - 1;
                        }
			else{
			   
			   	cout<<"\nSending packet = "<<data_packet.seq_num<<"Receiving ACK ="<<data_ack.seq_num;
			   
                	}      
		}// end of while
		
			cout<<"\n\nFile Sent to Client. Closing file and socket.";
	   		close(send_fp);
			close(server_handler);
       
	}// if of hanshake
	else{
		cout<<"\n\nError Occured: Connection Terminated!!";
		close(server_handler);
	}
        return 0;
}
