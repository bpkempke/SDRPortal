#ifndef PORTAL_COMMAND_SOCKET_H
#define PORTAL_COMMAND_SOCKET_H

#include "generic.h"
#include "hierarchicalDataflowBlock.h"
#include "genericSDRInterface.h"
#include "genericSocketInterface.h"

class portalCommandSocket : public hierarchicalDataflowBlock{
public:
	portalCommandSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_int);
	~portalCommandSocket();

	//Methods inherited from hierarchicalDataflowBlock
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0);
private:
	int cur_channel;
	socketType socket_type;
	genericSocketInterface *socket_int;
	genericSDRInterface *sdr_int;
};

#endif

