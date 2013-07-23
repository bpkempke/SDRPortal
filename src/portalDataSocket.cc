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

#include <vector>
#include "genericSDRInterface.h"
#include "portalDataSocket.h"

portalDataSocket::portalDataSocket(socketType in_socket_type, int socket_num){
	stream_type = STREAM_INT8_T;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);

	//Link upper and lower 
	socket_int->addUpperLevel(this);
	this->addLowerLevel(socket_int);
}

portalDataSocket::~portalDataSocket(){
	delete socket_int;
}

int portalDataSocket::getPortNum(){
	return socket_int->getPortNum();
}

void portalDataSocket::setUID(int in_uid){
	uid = in_uid;
}

int portalDataSocket::getUID(){
	return uid;
}

void portalDataSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel){
	//Data coming in from the SDR

	messageType new_message;
	new_message.buffer = (char*)data;
	new_message.num_bytes = num_bytes;
	new_message.socket_channel = -1;

	dataToLowerLevel(&new_message, 1);
}

void portalDataSocket::dataFromLowerLevel(void *data, int num_messages, int local_down_channel){
	//Data coming in from the socket

	//First, we need to make sure data is casted correctly (data is a pointer to a vector of messages)
	messageType *in_messages = static_cast<messageType*>(data);

	//Traverse all incoming messages and forward all IQ data on to the SDR
	for(int ii=0; ii < num_messages; ii++){
		in_messages[ii].socket_channel = uid;
		in_messages[ii].stream_type = stream_type;
		dataToUpperLevel(&in_messages[ii], 1);
	}
}

