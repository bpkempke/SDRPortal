#ifndef SHELL_PORTAL_H
#define SHELL_PORTAL_H

#include "generic.h"
#include "hierarchicalDataflowBlock.h"
#include "genericSocketInterface.h"

struct cmdListenerArgs{
	shellPortal *shell_portal_ptr;
	int down_channel;
	FILE *command_fp;
	pthread_t *thread_ptr;
};

class shellPortal : public hierarchicalDataflowBlock{
public:
	shellPortal(socketType in_socket_type, int socket_num);
	~shellPortal();

	void deleteListener(cmdListenerArgs *in_arg);

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
