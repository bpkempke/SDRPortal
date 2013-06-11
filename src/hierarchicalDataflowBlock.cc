
#include "hierarchicalDataflowBlock.h"

hierarchicalDataflowBlock::hierarchicalDataflowBlock(int num_down_channels, int num_up_channels){

	//Initialize higher and lower-level links based on the number of channels passed in
	upper_level_links.reserve(num_up_channels);
	lower_level_links.reserve(num_down_channels);
}

void hierarchicalDataflowBlock::addUpperLevel(hierarchicalDataflowBlock *in_block, int remote_up_channel, int local_up_channel){

	//Add the requested hierarchy pointer to the upper_level_links matrix
	hierarchicalDataConnection new_connection;
	new_connection.local_channel = local_up_channel;
	new_connection.remote_channel = remote_up_channel;
	new_connection.remote = in_block;

	upper_level_links[local_up_channel].push_back(new_connection);
}


void hierarchicalDataflowBlock::dataToUpperLevel(void *data, int num_bytes, int local_up_channel){

	//Push the requested data to all of the higher-level blocks that reside on the requested channel
	for(int ii=0; ii < upper_level_links[local_up_channel].size(); ii++){
		hierarchicalDataConnection cur_conn = upper_level_links[local_up_channel][ii];
		cur_conn.remote->dataFromLowerLevel(data, num_bytes, cur_conn.remote_channel);
	}
}

void hierarchicalDataflowBlock::addLowerLevel(hierarchicalDataflowBlock *in_block, int remote_down_channel, int local_down_channel){

	//Add the requested hierarchy pointer to the lower_level_links matrix
	hierarchicalDataConnection new_connection;
	new_connection.local_channel = local_down_channel;
	new_connection.remote_channel = remote_down_channel;
	new_connection.remote = in_block;

	lower_level_links[local_down_channel].push_back(new_connection);
}

void hierarchicalDataflowBlock::dataToLowerLevel(void *data, int num_bytes, int local_down_channel){

	//Push the requested data to all of the higher-level blocks that reside on the requested channel
	for(int ii=0; ii < lower_level_links[local_down_channel].size(); ii++){
		hierarchicalDataConnection cur_conn = lower_level_links[local_down_channel][ii];
		cur_conn.remote->dataFromUpperLevel(data, num_bytes, cur_conn.remote_channel);
	}
}

void hierarchicalDataflowBlock::removeUpperLevel(hierarchicalDataflowBlock *in_block){

	//Iterate over all of the different uplink channels
	for(int ii=0; ii < upper_level_links.size(); ii++){

		//Then just do a linear search for the block to delete, since this shouldn't happen often
		for(int jj=0; jj < upper_level_links[ii].size(); jj++){
			if(upper_level_links[ii][jj].remote == in_block){
				upper_level_links[ii].erase(upper_level_links[ii].begin()+jj);
				jj--;
			}
		}
	}
}

void hierarchicalDataflowBlock::removeLowerLevel(hierarchicalDataflowBlock *in_block){

	//Iterate over all of the different uplink channels
	for(int ii=0; ii < lower_level_links.size(); ii++){

		//Then just do a linear search for the block to delete, since this shouldn't happen often
		for(int jj=0; jj < lower_level_links[ii].size(); jj++){
			if(lower_level_links[ii][jj].remote == in_block){
				lower_level_links[ii].erase(lower_level_links[ii].begin()+jj);
				jj--;
			}
		}
	}
}

