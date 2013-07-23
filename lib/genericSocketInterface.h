/*    SDRPortal - A generic web-based interface for SDRs
 *    Copyright (C) 2013 Ben Kempke (bpkempke@umich.edu)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GENERIC_SOCKET_INTERFACE_H
#define GENERIC_SOCKET_INTERFACE_H

#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <netinet/in.h>
#include "generic.h"
#include "hierarchicalDataflowBlock.h"
#include "streamConverter.h"

enum destType{UPSTREAM, DOWNSTREAM};

struct messageType{
	char *buffer;//TODO: Should this be void* instead???
	int num_bytes;
	int socket_channel;
	streamType stream_type;
	destType message_dest;
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

class socketThread : public hierarchicalDataflowBlock{
public:
	socketThread(int in_fp, pthread_mutex_t *in_mutex, socketInterpreter *in_interp, int in_uid);
	void *socketReader();
	void socketWriter(char *buffer, int buffer_length);

	int getUID(){return uid;};

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){};
private:
	socketInterpreter *interp;
	pthread_mutex_t *shared_mutex;
	int socket_fp;
	int uid;
};

class genericSocketInterface: public hierarchicalDataflowBlock{
public:
	genericSocketInterface(socketType in_socket_type, int portnum, int max_connections=0);
	~genericSocketInterface();
	socketType getSocketType(){return socket_type;};
	void *connectionListenerThread();
	int getPortNum();

	//Virtual functions inherited from abstract base class fdInterface
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);
	virtual void notificationFromLower(void *notification);

protected:
	socketType socket_type;
	int socket_fp;

private:
	int socket_portnum;
	int max_connections;
	int cur_uid;
	int initSocket(int portnum);
	std::vector<socketInterpreter*> client_parsers;
	pthread_t conn_listener;
	pthread_mutex_t mutex;
};

class tcpSocketInterpreter : public socketInterpreter{
public:
	//For now, should be ok to just inherit the default socketInterpreter functionality
	tcpSocketInterpreter(){};
	//std::vector<messageType> parseDownstreamMessage(messageType in_message);
	//std::vector<messageType> parseUpstreamMessage(messageType in_message);
};

class udpSocketInterpreter : public socketInterpreter{
public:
	//For now, should be ok to just inherit the default socketInterpreter functionality
	udpSocketInterpreter(){};
	//std::vector<messageType> parseDownstreamMessage(messageType in_message);
	//std::vector<messageType> parseUpstreamMessage(messageType in_message);
};

class wsSocketInterpreter : public socketInterpreter{
public:
	wsSocketInterpreter(bool is_binary);
	std::vector<messageType> parseDownstreamMessage(messageType in_message);
	std::vector<messageType> parseUpstreamMessage(messageType in_message);
private:
	bool connection_established;
	bool is_binary;
	std::string message_parser;
};

#endif

