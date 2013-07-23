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

#ifndef HIER_DATAFLOW_BLOCK_H
#define HIER_DATAFLOW_BLOCK_H

#include <vector>
#include <map>

class hierarchicalDataflowBlock;

struct hierarchicalDataConnection{
	int local_up_channel;
	int local_down_channel;
	hierarchicalDataflowBlock *remote;
};

/*
 * hierarchicalDataflowBlock implements the necessary functionality to connect separate independent classes together to
 *   provide a generic RX/TX dataflow path between each class in the hierarchy
 */
class hierarchicalDataflowBlock{
public:
	hierarchicalDataflowBlock(){};
	void addUpperLevel(hierarchicalDataflowBlock *in_block, int up_channel=0, int local_up_channel=0);
	virtual void dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0) = 0;
	void dataToUpperLevel(void *data, int num_bytes, int local_up_channel=0);
	void addLowerLevel(hierarchicalDataflowBlock *in_block, int down_channel=0, int local_down_channel=0);
	virtual void dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0) = 0;
	void dataToLowerLevel(void *data, int num_bytes, int local_down_channel=0);
	void removeUpperLevel(hierarchicalDataflowBlock *in_block);
	void removeLowerLevel(hierarchicalDataflowBlock *in_block);
	void notifyUpper(void *notification);
	virtual void notificationFromLower(void *notification){};
private:
	struct hierarchicalDataConnection {
		int local_channel;
		int remote_channel;
		hierarchicalDataflowBlock *remote;
	};

	typedef std::map<int, std::vector<hierarchicalDataConnection> >::iterator linkIterator;
	std::map<int, std::vector<hierarchicalDataConnection> > upper_level_links;
	std::map<int, std::vector<hierarchicalDataConnection> > lower_level_links;
};

#endif

