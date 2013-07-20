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

