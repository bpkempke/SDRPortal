#ifndef STREAM_CONVERTER_H
#define STREAM_CONVERTER_H

#include "streamConversionHelper.h"
#include <algorithm>
#include <map>

class scratchspaceType{
public:
	void *scratchspace;
	int cur_capacity;
	int cur_size;

	scratchspaceType() : scratchspace(NULL), cur_capacity(0), cur_size(0) {};
};

struct fullConvType{
	convFunc cnv_to_common;
	convFunc cnv_from_common;
};

class streamConverter {
public:
	streamConverter(streamType common_type);
	void setConversionCommonType(streamType common_type);
	int convertToCommon(void *in_data, int num_bytes, streamType start_type, int num_prims_per_block);
	int convertFromCommon(void *in_data, int num_bytes, streamType result_type, int num_prims_per_block);
	void *getResultFromStreamType(streamType in_type);
private:
	int convertWorker(void *in_data, int num_bytes, streamType secondary_type, int num_prims_per_block, bool convert_to);

	//Scratchspace used in convertFrom()
	scratchspaceType t_scratch;

	//Current type points to the last type sent to convertTo()
	streamType common_type;

	//Map to different scratchspaces used in convertTo()
	std::map<streamType, scratchspaceType> scratchspace_map;
	std::map<streamType, fullConvType> conversion_map;
};

#endif

