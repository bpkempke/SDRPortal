#include <algorithm>
#include <map>
#include "streamConverter.h"

streamConverter::streamConverter(streamType common_type){
	this->common_type = common_type;
}

void streamConverter::setConversionCommonType(streamType common_type){
	this->common_type = common_type;

	//Update conversion function map
	for(std::map<streamType, fullConvType>::iterator it=conversion_map.begin(); it != conversion_map.end(); it++){
		conversion_map[it->first].cnv_to_common = getConversionFunc(it->first, common_type);
		conversion_map[it->first].cnv_from_common = getConversionFunc(common_type, it->first);
	}
}

int streamConverter::convertToCommon(void *in_data, int num_bytes, streamType start_type, int num_prims_per_block){
	return convertWorker(in_data, num_bytes, start_type, num_prims_per_block, true);;
}

int streamConverter::convertFromCommon(void *in_data, int num_bytes, streamType result_type, int num_prims_per_block){
	return convertWorker(in_data, num_bytes, result_type, num_prims_per_block, false);;
}

int streamConverter::convertWorker(void *in_data, int num_bytes, streamType secondary_type, int num_prims_per_block, bool convert_to){
	int stream_t_len = (convert_to) ? getStreamTypeLength(secondary_type) : getStreamTypeLength(common_type);

	//Only process the number of bytes which can be formed into a block
	int num_translated_blocks = num_bytes/num_prims_per_block/stream_t_len;
	int num_consumed_bytes = num_translated_blocks*num_prims_per_block*stream_t_len;

	//Have to convert to char* in order to do arithmetic on end pointer
	char *end_pointer = (char*)in_data;

	//Make sure there's a scratchspace available
	if(scratchspace_map.find(secondary_type) == scratchspace_map.end() || scratchspace_map[secondary_type].cur_capacity < num_consumed_bytes){

		delete [] (char*)(scratchspace_map[secondary_type].scratchspace);
		scratchspace_map[secondary_type].scratchspace = new char[num_consumed_bytes];
		scratchspace_map[secondary_type].cur_capacity = num_consumed_bytes;

		//Set up the conversion functions if necessary
		if(conversion_map.find(secondary_type) == conversion_map.end()){
			conversion_map[secondary_type].cnv_to_common = getConversionFunc(secondary_type, common_type);
			conversion_map[secondary_type].cnv_from_common = getConversionFunc(common_type, secondary_type);
		}
	}
	
	//Call the actual conversion function
	if(convert_to)
		conversion_map[secondary_type].cnv_to_common(in_data, (void*)(end_pointer+num_consumed_bytes), scratchspace_map[secondary_type].scratchspace);
	else
		conversion_map[secondary_type].cnv_from_common(in_data, (void*)(end_pointer+num_consumed_bytes), scratchspace_map[secondary_type].scratchspace);

	return num_consumed_bytes;
}

void *streamConverter::getResultFromStreamType(streamType in_type){
	return scratchspace_map[in_type].scratchspace;
}
