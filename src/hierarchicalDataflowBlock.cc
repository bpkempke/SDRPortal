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

#include <iostream>
#include "hierarchicalDataflowBlock.h"


void hierarchicalDataflowBlock::addUpperLevel(hierarchicalDataflowBlock *in_block, int remote_up_channel, int local_up_channel){

	//Add the requested hierarchy pointer to the upper_level_links matrix
	hierarchicalDataConnection new_connection;
	new_connection.local_channel = local_up_channel;
	new_connection.remote_channel = remote_up_channel;
	new_connection.remote = in_block;
	upper_level_links[local_up_channel].push_back(new_connection);
}


void hierarchicalDataflowBlock::dataToUpperLevel(void *data, int num_bytes, int local_up_channel){

	if(upper_level_links.size() > 0){
		linkIterator start_channel = (local_up_channel < 0) ? upper_level_links.begin() : upper_level_links.find(local_up_channel);
		linkIterator end_channel = (local_up_channel < 0) ? upper_level_links.end() : ++upper_level_links.find(local_up_channel);
		for(linkIterator it=start_channel; it != end_channel; it++){
			//Push the requested data to all of the higher-level blocks that reside on the requested channel
			for(unsigned int ii=0; ii < it->second.size(); ii++){
				hierarchicalDataConnection cur_conn = it->second[ii];
				cur_conn.remote->dataFromLowerLevel(data, num_bytes, cur_conn.remote_channel);
			}
		}
	}
}

void hierarchicalDataflowBlock::notifyUpper(void *in_notification){

	if(upper_level_links.size() > 0){
		for(linkIterator it=upper_level_links.begin(); it != upper_level_links.end(); it++){
			for(unsigned int jj=0; jj < it->second.size(); jj++){
				hierarchicalDataConnection cur_conn = it->second[jj];
				cur_conn.remote->notificationFromLower(in_notification);
			}
		}
	}
}

void hierarchicalDataflowBlock::addLowerLevel(hierarchicalDataflowBlock *in_block, int remote_down_channel, int local_down_channel){

	//Add the requested hierarchy pointer to the lower_level_links matrix
	hierarchicalDataConnection new_connection;
	new_connection.local_channel = local_down_channel;
	new_connection.remote_channel = remote_down_channel;
	new_connection.remote = in_block;

	//Make sure there's space reserved first
	lower_level_links[local_down_channel].push_back(new_connection);
}

void hierarchicalDataflowBlock::dataToLowerLevel(void *data, int num_bytes, int local_down_channel){

	if(lower_level_links.size() > 0){
		linkIterator start_channel = (local_down_channel < 0) ? lower_level_links.begin() : lower_level_links.find(local_down_channel);
		linkIterator end_channel = (local_down_channel < 0) ? lower_level_links.end() : ++lower_level_links.find(local_down_channel);
		for(linkIterator it=start_channel; it != end_channel; it++){
			//Push the requested data to all of the higher-level blocks that reside on the requested channel
			for(unsigned int ii=0; ii < it->second.size(); ii++){
				hierarchicalDataConnection cur_conn = it->second[ii];
				cur_conn.remote->dataFromUpperLevel(data, num_bytes, cur_conn.remote_channel);
			}
		}
	}
}

void hierarchicalDataflowBlock::removeUpperLevel(hierarchicalDataflowBlock *in_block){
	std::cout << "removing upper level" << std::endl;

	//Iterate over all of the different uplink channels
	for(linkIterator it=upper_level_links.begin(); it != upper_level_links.end(); it++){

		//Then just do a linear search for the block to delete, since this shouldn't happen often
		for(unsigned int jj=0; jj < it->second.size(); jj++){
			if(it->second[jj].remote == in_block){
				it->second.erase(it->second.begin()+jj);
				jj--;
			}
		}
	}
}

void hierarchicalDataflowBlock::removeLowerLevel(hierarchicalDataflowBlock *in_block){
	std::cout << "removing lower level" << std::endl;

	//Iterate over all of the different uplink channels
	for(linkIterator it=lower_level_links.begin(); it != lower_level_links.end(); it++){

		//Then just do a linear search for the block to delete, since this shouldn't happen often
		for(unsigned int jj=0; jj < it->second.size(); jj++){
			if(it->second[jj].remote == in_block){
				it->second.erase(it->second.begin()+jj);
				jj--;
			}
		}
	}
}

