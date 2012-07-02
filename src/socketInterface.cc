#include "socketInterface.h"
#include "base64.h"
#include "sha1.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <netinet/in.h>

//#define DEBUG

/*
 * Function definitions for the socketInterface class
 */
using namespace std;
/*
 * Function definitions for the socketInterface class
 */

socketInterface::socketInterface(socketIntType in_socket_type){
	this_uplink = NULL;
	socket_type = in_socket_type;
}

void socketInterface::bind(int in_fp){
	socket_fp = in_fp;
}

int socketInterface::getSockFP(){
	return socket_fp;
}

void socketInterface::closeFP(){
	close(socket_fp);
}

void socketInterface::fdBytesReceived(char *buffer, int num_bytes, fdInterface *from_interface)
{
	//This function packetizes and then distributes incoming bytes
	//TODO: this should probably be done faster than a linear search, but with only a few clients, it should be fine
	int client_index = 0;
	for(int i = 0; i < clients.size(); i++){
		if(from_interface == clients[i]){
			client_index = i;
			break;
		}
	}
	messageType new_message = {buffer, num_bytes, UPSTREAM};
	vector<messageType> new_packets = client_parsers[client_index]->parseDownstreamMessage(new_message);
	dispatchMessages(new_packets, client_index);
}

void socketInterface::registerDownstreamInterface(fdInterface *in_thread){
	static int sec_id_counter = 0;

	//Add this client id to the table in order to keep track of it for packetization
	clients.push_back(in_thread);
	in_thread->secondary_id = sec_id_counter++;
	cout << "NEW DOWNSTREAM INTERFACE!!" << endl;
	if(socket_type == SOCKET_TCP)
		client_parsers.push_back(new tcpSocketInterpreter());
	else if(socket_type == SOCKET_UDP)
		client_parsers.push_back(new udpSocketInterpreter());
	else if(socket_type == SOCKET_WS)
		client_parsers.push_back(new wsSocketInterpreter());
}

void socketInterface::deleteDownstreamInterface(fdInterface *in_thread){
	for(int i = 0; i < clients.size(); i++){
		if(clients[i] == in_thread){
			clients.erase(clients.begin() + i);
			client_parsers.erase(client_parsers.begin() + i);
			printf("deleted client\n");
			//delete in_thread;
	        }
	}
}

void socketInterface::registerUpstreamInterface(fdInterface *up_int){
	this_uplink = up_int;
	this_uplink->registerDownstreamInterface(this);
}

void socketInterface::dataFromUpstream(char *message, int num_bytes, fdInterface *from_interface){
	messageType in_message = {message, num_bytes, DOWNSTREAM};
	for(int ii = 0; ii < clients.size(); ii++){
		vector<messageType> new_messages = client_parsers[ii]->parseUpstreamMessage(in_message);
		dispatchMessages(new_messages, ii);
	}
}

void socketInterface::dispatchMessages(vector<messageType> in_messages, int client_idx){
	for(int i = 0; i < in_messages.size(); i++)
	{
		if(in_messages[i].message_dest == DOWNSTREAM){
			clients[client_idx]->dataFromUpstream(in_messages[i].buffer, in_messages[i].num_bytes, this);
		} else if(in_messages[i].message_dest == UPSTREAM){
			this_uplink->fdBytesReceived(in_messages[i].buffer, in_messages[i].num_bytes, this, client_idx);
		}
	}
}
/*
 * Function definitions for the tcpSocketInterpreter class
 */

//tcpSocketInterpreter::tcpSocketInterpreter(){
//
//}
//
//void tcpSocketInterpreter::serviceIncomingPacket(lithiumPacket in_packet){
//}

/*
 * Function definitions for the udpSocketInterpreter class
 */

//udpSocketInterpreter::udpSocketInterpreter(){
//
//}
//
//void udpSocketInterpreter::serviceIncomingPacket(lithiumPacket in_packet){
//}

/*
 * Function definitions for the wsSocketInterpreter class
 */
wsSocketInterpreter::wsSocketInterpreter(){
	connection_established = false;
	message_parser = "";
}

vector<messageType> wsSocketInterpreter::parseDownstreamMessage(messageType in_message)
{
	char *buffer = in_message.buffer;
	int num_bytes = in_message.num_bytes;
	vector<messageType> out_messages;
	messageType new_out_message;
	int end_idx;

	//Keep track of historic data coming in from the socket
	message_parser.append(buffer, num_bytes);

	//If we've received the end of a client handshake, go ahead and process the request
	if(!connection_established && (end_idx = message_parser.find("\r\n\r\n")) != -1){
		cout << message_parser.length() - end_idx << endl;
		//Parsing results
		bool ws_isws = false;
		string ws_key, ws_protocol, ws_dir;

		//Parser
		stringstream strstr(message_parser);
		string temp_str, last_str;
		int ii=0;
		while(strstr >> temp_str){
			//Process keys and values
			if(last_str.compare("GET") == 0)
				ws_dir = temp_str;
			if(last_str.compare("Upgrade:") == 0)
				ws_isws = true;
			if(last_str.compare("Sec-WebSocket-Key:") == 0)
				ws_key = temp_str;
			if(last_str.compare("Sec-WebSocket-Protocol:") == 0)
				ws_protocol = temp_str;

			//Save temp_str for next time
			last_str = temp_str;
		}

		//Get rid of the stuff we just processed from the message_parser string
		message_parser.erase(0,end_idx+4);

		//Check to see if we got a web socket request
		if(ws_isws){
			//Now do stuff with the request
			cout << "ws_dir = " << ws_dir << endl;
			cout << "ws_key = " << ws_key << endl;
			cout << "ws_protocol = " << ws_protocol << endl << endl;

			//Compute the correct reply
			unsigned char sha1_hash[20];
			string sha1_result(ws_key);
			sha1_result += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			sha1::calc(sha1_result.c_str(), sha1_result.length(), sha1_hash);
			sha1_result = base64_encode(sha1_hash, 20);

			//Assemble the reply
			static string reply_string;
			reply_string = "HTTP/1.1 101 Switching Protocols\r\n";
			reply_string += "Upgrade: websocket\r\n";
			reply_string += "Connection: Upgrade\r\n";
			reply_string += "Sec-WebSocket-Accept: ";
			reply_string += sha1_result;
			reply_string += "\r\n";
			reply_string += "Sec-WebSocket-Protocol: ";
			reply_string += ws_protocol;
			reply_string += "\r\n\r\n";

			cout << reply_string;

			//Send the reply
			new_out_message.buffer = (char*)reply_string.c_str();
			new_out_message.num_bytes = reply_string.length();
			new_out_message.message_dest = DOWNSTREAM;
			out_messages.push_back(new_out_message);

			connection_established = true;

			////Try to send a data message now
			//static unsigned char message[7] = {0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f};
			//new_out_message.buffer = (char*)message;
			//new_out_message.num_bytes = 7;
			//new_out_message.message_dest = DOWNSTREAM;
			//for(int ii=0; ii < 10; ii++){
			//	out_messages.push_back(new_out_message);
			//}
		}
	}


	//Parse incoming data messages for possible uplink
	while(connection_established && message_parser.length() > 2){

		//Start by extacting the payload length
		int payload_len = message_parser[1] & 0x7F;
		int message_start_idx = 6;
		if(payload_len == 126){
			uint16_t len_temp;
			memcpy(&len_temp, &message_parser[2], 2);
			len_temp = ntohs(len_temp);
			payload_len = (int)len_temp;
			message_start_idx += 2;
		}else if(payload_len == 127){
			uint32_t len_temp;
			memcpy(&len_temp, &message_parser[6], 4);
			len_temp = ntohl(len_temp);
			payload_len = (int)len_temp;
			message_start_idx += 8;
		}

		//Check to see if we have the whole message now!
		if(message_parser.length() < message_start_idx + payload_len) return out_messages;

		//cout << "I think that payload_len=" << payload_len << " end message_start_idx=" << message_start_idx << endl;
		//cout << "message_parser[0] = " << (unsigned int)(unsigned char)message_parser[1] << endl;

		//We have to decode the message by XORing with the mask
		int message_mask_idx = message_start_idx - 4;
		for(int ii=0; ii < payload_len; ii++){
			message_parser[ii+message_start_idx] = message_parser[ii+message_start_idx] ^ message_parser[(ii%4)+message_mask_idx];
		}

		//If so, then let's extract the message...
		new_out_message.buffer = &message_parser[message_start_idx];
		new_out_message.num_bytes = payload_len;
		new_out_message.message_dest = UPSTREAM;
		out_messages.push_back(new_out_message);
		//cout << "RECEIVED WS MESSAGE: " << message_parser << endl;

		//And get rid of it from the parsing string
		message_parser.erase(0,payload_len + message_start_idx);
	}

	return out_messages;
}

vector<messageType> wsSocketInterpreter::parseUpstreamMessage(messageType in_message){
	//Construct a valid RFC 6455 message from the incoming message

	//First, figure out how big our resulting message is going to be
	bool ml_16 = in_message.num_bytes > 125 && in_message.num_bytes < 65535;
	bool ml_64 = in_message.num_bytes > 65535;
	int message_length = in_message.num_bytes+2;
	if(ml_16) message_length += 2;
	else if(ml_64) message_length += 8;
	char *message = new char[message_length];

	//First byte: FIN (always true in our case) and opcode (0x02 = binary)
	message[0] = 0x82;

	//Next byte: payload length (0-125 if small message, 126 if length can fit in 16 bits, 127 if length can fit in 64 bits)
	//Subsequent bytes: Length (if needed), then the actual message
	if(!(ml_16 || ml_64)){
		message[1] = (char)in_message.num_bytes;
		memcpy(&message[2],in_message.buffer,in_message.num_bytes);
	}else if(ml_16){
		message[1] = 126;
		uint16_t ml_temp = htons((uint16_t)in_message.num_bytes);
		memcpy(&message[2],&ml_temp,2);
		memcpy(&message[4],in_message.buffer,in_message.num_bytes);
	}else if(ml_64){
		message[1] = 127;
		uint32_t ml_temp = htonl((uint32_t)in_message.num_bytes);
		memset(&message[2],0,4);
		memcpy(&message[6],&ml_temp,4);
		memcpy(&message[10],in_message.buffer,in_message.num_bytes);
	}

	messageType new_message = {message, message_length, DOWNSTREAM};
	vector<messageType> ret_vector(1,new_message);
	return ret_vector;
}
/*
 * Function definitions for the socketThread class
 */

//This function just acts as a proxy in order to run POSIX threads through a member function
static void *socketReaderProxy(void *in_socket_thread){
	static_cast<socketThread*>(in_socket_thread)->socketReader();
}

socketThread::socketThread(int in_fp, fdInterface *in_sock, pthread_mutex_t *in_mutex, bool in_is_datagram_socket, struct sockaddr_in *in_addr){
	is_datagram_socket = in_is_datagram_socket;

	socket_fp = in_fp;
	sock_int = in_sock;
	shared_mutex = in_mutex;
	in_sock->registerDownstreamInterface(this);

	udp_addr = *in_addr;

	//Instantiate the thread which reads data from the socket
	//NOTE: We only need this thread if we're not doing datagrams
	if(!is_datagram_socket){
		pthread_t new_socket_thread;	
		pthread_create(&new_socket_thread, NULL, socketReaderProxy, this);
		pthread_detach(new_socket_thread);
	}
}

void *socketThread::socketReader(){
	struct sockaddr_in from;
	socklen_t fromlen;
	char buffer[256];

	#ifdef DEBUG
	printf("Got to the socket Reader function...\n");
	#endif
	while(1){
		int n;
		n = read(socket_fp, buffer, sizeof(buffer));
		if(n <= 0) break;

		//Semaphore here to protect shared data structures
		if(shared_mutex)
			pthread_mutex_lock(shared_mutex);
		receivedData(buffer, n);
		if(shared_mutex)
			pthread_mutex_unlock(shared_mutex);
		for(int ii = 0; ii < n; ii++){
			//printf("%c",buffer[ii]);
		}
		fflush(stdout);
		#ifdef DEBUG
		printf("Got some data here......\n");
		#endif
	}

	#ifdef DEBUG
	printf("exiting...\n");
	#endif
	close(socket_fp);
	sock_int->deleteDownstreamInterface(this);
}

void socketThread::receivedData(char *message, int num_bytes){
	sock_int->fdBytesReceived(message, num_bytes, this);
}

void socketThread::dataFromUpstream(char *message, int num_bytes, fdInterface *from_interface){
	socketWriter((unsigned char *)message, num_bytes);
}

void socketThread::socketWriter(unsigned char *buffer, int buffer_length){
	if(socket_fp){
		//TODO: this needs to go to a UDP socket sometimes!
		int n;
		if(is_datagram_socket)
			n = sendto(socket_fp, buffer, buffer_length, 0, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr_in));
		else
	        	n = write(socket_fp, buffer, buffer_length);
		if(n <= 0){
			//TODO: Commit hara kiri...
			printf("socket not open anymore...tried writing %s bytes...\n",buffer);
		}
	}
}

