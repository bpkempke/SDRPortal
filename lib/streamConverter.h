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

