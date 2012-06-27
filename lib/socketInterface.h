#ifndef SOCKET_INTERFACE_H
#define SOCKET_INTERFACE_H

#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "clientInterface.h"

class socketThread;
class lithiumInterface;
struct lithiumPacket{
	int data_length;
	int cmd_type;
	std::vector<unsigned char> data;
};

//Telemetry reception structure
struct UHF_telem{
	uint16_t num_CMD;
	uint16_t processor_temp;
	uint16_t rf_front_temp;
	float rssi;
	uint16_t PLL_lock;
	uint32_t bytes_received;
	uint32_t bytes_transmitted;
};

enum socketIntType{
	SOCKET_TCP,
	SOCKET_UDP,
	SOCKET_WS
};

enum messageDirType{
	UPSTREAM,
	DOWNSTREAM
};

//TODO: pass this around all the time instead of a buffer, num_bytes pair...
struct messageType{
	char *buffer;
	int num_bytes;
	messageDirType message_dest;
};

/*
 * fdInterface class
 *
 * Abstract base class for all interfaces communicating with a file descriptor
 */
//TODO: This should really be pure-virtual....
class fdInterface{
public:
	virtual void fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_interface){};
	virtual void registerDownstreamInterface(fdInterface *in_thread){};
	virtual void deleteDownstreamInterface(fdInterface *in_thread){};
	virtual void registerUpstreamInterface(fdInterface *up_int){};
	virtual void dataFromUpstream(char *data, int num_bytes, fdInterface *from_interface){};
	int secondary_id;
	
	//Overloaded functions for fdInterfaces which could have multiple independant connections
	virtual void fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_interface, int secondary_id){};
	virtual void dataFromUpstream(char *data, int num_bytes, fdInterface *from_interface, int secondary_id){};
};

class socketInterpreter{
public:
	virtual std::vector<messageType> parseDownstreamMessage(messageType in_message){
		return std::vector<messageType>(1,in_message);
	};
	virtual std::vector<messageType> parseUpstreamMessage(messageType in_message){
		return std::vector<messageType>(1,in_message);	
	};
};

class tcpSocketInterpreter : public socketInterpreter{
public:
	//For now, should be ok to just inherit the default socketInterpreter functionality
	//std::vector<messageType> parseDownstreamMessage(messageType in_message);
	//std::vector<messageType> parseUpstreamMessage(messageType in_message);
};

class udpSocketInterpreter : public socketInterpreter{
public:
	//For now, should be ok to just inherit the default socketInterpreter functionality
	//std::vector<messageType> parseDownstreamMessage(messageType in_message);
	//std::vector<messageType> parseUpstreamMessage(messageType in_message);
};

class wsSocketInterpreter : public socketInterpreter{
public:
	wsSocketInterpreter();
	std::vector<messageType> parseDownstreamMessage(messageType in_message);
	std::vector<messageType> parseUpstreamMessage(messageType in_message);
private:
	bool connection_established;
	std::string message_parser;
};

class socketInterface: public fdInterface{
	public:
		socketInterface(socketIntType in_socket_type);
		void bind(int in_fp);
		int getSockFP();
		void closeFP();
		void dispatchMessages(std::vector<messageType> in_message, int client_idx);

		//Virtual functions inherited from abstract base class fdInterface
		void fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_thread);
		void registerDownstreamInterface(fdInterface *in_thread);
		void deleteDownstreamInterface(fdInterface *in_thread);
		void registerUpstreamInterface(fdInterface *up_int);
		void dataFromUpstream(char *data, int num_bytes, fdInterface *from_interface);
		void uplinkData(char *data, int num_bytes){};

	protected:
		socketIntType socket_type;
		int socket_fp;
		std::vector<fdInterface*> clients;
		fdInterface *this_uplink;

	private:
		std::vector<socketInterpreter*> client_parsers;
		
};

class socketThread : public fdInterface{
	public:
		socketThread(int in_fp, fdInterface *in_sock, pthread_mutex_t *in_mutex);
		void *socketReader();
		void socketWriter(unsigned char *buffer, int buffer_length);
		pthread_mutex_t *shared_mutex;

		//Stuff inherited from fdInterface class
		void dataFromUpstream(char *message, int num_bytes, fdInterface *from_interface);
	private:
		fdInterface *sock_int;
		
		int socket_fp;
		int socket_id;
};

#endif

