#include "clientInterface.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

int openClientSocketConnection(int portnum){
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	//Prepare the socket for opening
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("Error: Couldn't prepare the socket for opening\n");
		return -1; //TODO: appropriate error here
	}

//	server = gethostbyname("localhost");
//	if(server == NULL){
//		printf("Error: Couldn't stat localhost\n");
//		return -1; //TODO: appropriate error here
//	}

	//Clear out the serv_addr struture and populate with the new correct information
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
//	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	char *sa = (char *)&serv_addr.sin_addr.s_addr;
	sa[0] = 127;
	sa[1] = 0;
	sa[2] = 0;
	sa[3] = 1;
	serv_addr.sin_port = htons(portnum);

	//Try connecting to the specified port number
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("Error: Couldn't connect to the specified port! Is there a server there?\n");
		return -1; //TODO: appropriate error here
	}

	//Now that we have an appropriately created socket, return the file pointer to it
	return sockfd;
}

/*Original Version*/
void sendPacket(int sockfd, unsigned char *out_packet, int packet_length, char packet_type){
	char lsb = packet_length & 0xff;
	char msb = (packet_length & 0xff00) >> 8;

	write(sockfd, &packet_type, 1);
	write(sockfd, &lsb, 1); //Socket communications are little endian
	write(sockfd, &msb, 1);
	write(sockfd, out_packet, packet_length);
}
/*End original version*/

/*Version which can split up packets that are too large for the comm protocol to handle
void sendPacket(int sockfd, unsigned char *out_packet, int packet_length, char packet_type){
	
	char lsb = packet_length & 0xff;
	char msb = (packet_length & 0xff00) >> 8;

	write(sockfd, &packet_type, 1);
	write(sockfd, &lsb, 1); //Socket communications are little endian
	write(sockfd, &msb, 1);

	int max_packet_length = 100;	

	//send full packets
	int num_full_packets = packet_length/max_packet_length;
	for(int i =0;i<num_full_packets;i++)
		write(sockfd, out_packet+max_packet_length*i, max_packet_length);

	//send remainder
	int remaining_length = packet_length%max_packet_length;
	write(sockfd, out_packet+max_packet_length*num_full_packets, remaining_length);	

}
/*End edited version*/

vector<struct socketPacket > packetParser::parse(char *buffer, int num_bytes){
	static struct socketPacket packet_data;
	static unsigned int length = 0;;
	vector<struct socketPacket > return_packets;
	//State machine waits for start of transmission 
	for(int i = 0; i < num_bytes; i++){
		unsigned char cur_char = buffer[i];
		switch(this->state){
			//Waiting for a start of transmission character
			//TODO: this could be taken out, and just leave the length, for now it's staying
			case STATE_IDLE:
				packet_data.packet_type = cur_char;
				if(cur_char == 'D' || cur_char == 'C' || cur_char == 'T' || cur_char == 'a' || cur_char == 'c')
					this->state = STATE_GET_LENGTH1;			
				length = 0;
			break;

			case STATE_GET_LENGTH1:
				length = (unsigned int)cur_char;
				this->state++;
			break;

			case STATE_GET_LENGTH2:
				length |= ((unsigned int)cur_char) << 8;
				packet_data.data.clear();
				if(length == 0){
					return_packets.push_back(packet_data);
					this->state = STATE_IDLE;
				}else
					this->state++;
			break;

			case STATE_GET_MESSAGE:
				packet_data.data.push_back((char)cur_char);
				if(packet_data.data.size() == length){
					return_packets.push_back(packet_data);
					this->state = STATE_IDLE;
				}
			break;
		}
	}

	return return_packets;
}


