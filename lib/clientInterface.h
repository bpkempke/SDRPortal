#ifndef CLIENTINTERFACE_H
#define CLIENTINTERFACE_H
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#define BEACON_SIZE 166//size of output_beacon_data in beacon_client, needed in .h so can share with command_client

#define MAT_COMMAND_BUFF_LENGTH 550//size of mat_command_buff array in commmand_client, needed in .h so can share with scheduler

#define DOWNLINK_45S_WAIT_PERIOD 7

#define STATE_IDLE 0
#define STATE_GET_LENGTH1 1
#define STATE_GET_LENGTH2 2
#define STATE_GET_MESSAGE 3

int openClientSocketConnection(int portnum);
void sendPacket(int sockfd, unsigned char *out_packet, int packet_length, char packet_type);

struct socketPacket{
	int packet_type;
	std::vector<unsigned char> data;
};

class packetParser{
	public:
		packetParser(){state = STATE_IDLE;};	
		std::vector<struct socketPacket > parse(char *buffer, int num_bytes);

	private:
		int state;
		
};

#endif //CLIENTINTERFACE_H
