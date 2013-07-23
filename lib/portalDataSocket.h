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

#ifndef PORTAL_DATA_H
#define PORTAL_DATA_H

#include "hierarchicalDataflowBlock.h"
#include "genericSocketInterface.h"
#include "streamConverter.h"
#include "generic.h"

class genericSDRInterface;

class portalDataSocket : public hierarchicalDataflowBlock{
public:
	portalDataSocket(socketType in_socket_type, int socket_num);
	~portalDataSocket();

	int getPortNum();
	void setUID(int in_uid);
	int getUID();

	streamType getStreamType(){return stream_type;};
	void setStreamType(streamType in_stream_type){stream_type = in_stream_type;};

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);
private:
	int uid;
	genericSocketInterface *socket_int;
	streamType stream_type;
};

#endif

