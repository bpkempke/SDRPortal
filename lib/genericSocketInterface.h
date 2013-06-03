#ifndef GENERIC_SOCKET_INTERFACE_H
#define GENERIC_SOCKET_INTERFACE_H

#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <netinet/in.h>

enum socketIntType{
	SOCKET_TCP,
	SOCKET_UDP,
	SOCKET_WS
};

class genericSocketInterface: public hierarchicalDataflowBlock{
public:
	genericSocketInterface(socketIntType in_socket_type, int portnum, int max_connections=0);
	~genericSocketInterface();
	void dispatchMessages(std::vector<messageType> in_message, int client_idx);
	socketIntType getSocketType(){return socket_type;};
	void *connectionListenerThread();

	//Virtual functions inherited from abstract base class fdInterface
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);

protected:
	socketIntType socket_type;
	int socket_fp;
	std::vector<socketThread*> clients;
	fdInterface *this_uplink;

private:
	int socket_portnum;
	int max_connections;
	void initSocket();
	std::vector<socketInterpreter*> client_parsers;
	pthread_t conn_listener;
	pthread_mutex_t mutex;
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

class socketThread : public hierarchicalDataflowBlock{
public:
	socketThread(int in_fp, pthread_mutex_t *in_mutex, socketInterpreter *in_interp);
	void *socketReader();
	void socketWriter(unsigned char *buffer, int buffer_length);

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){};
private:
	socketInterpreter *interp;
	pthread_mutex_t *shared_mutex;
	int socket_fp;
};

#endif

