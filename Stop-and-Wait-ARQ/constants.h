#define DATA_SIZE 1024

/*
 * Packet types:
 * S - synchronization request
 * R - synchronization reply resonse
 * C - close connection response
 * W - Acknowledgement
 * D - data packets
 * */

typedef enum{
	S,
	R,
	W,
	D,
	A,
	C
}p_type;

#define PACKET_SIZE (sizeof(p_type) + sizeof(int) + sizeof(int) + DATA_SIZE)

#define SEQ_LOC sizeof(p_type)

#define DATA_SIZE_LOC (SEQ_LOC + sizeof(int))

#define DATA_LOC (DATA_SIZE_LOC + sizeof(int))

