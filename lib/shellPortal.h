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

#ifndef SHELL_PORTAL_H
#define SHELL_PORTAL_H

#include "generic.h"
#include "hierarchicalDataflowBlock.h"
#include "genericSocketInterface.h"
#include <vector>

class shellPortal;

struct cmdListenerArgs{
	shellPortal *shell_portal_ptr;
	int down_channel;
	pid_t pid;
	FILE *out_fp;
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
	virtual void notificationFromLower(void *notification);
private:
	int cur_channel;
	socketType socket_type;
	genericSocketInterface *socket_int;
	std::vector<cmdListenerArgs*> command_threads;
};

#endif
