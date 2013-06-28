#ifndef STREAM_CONVERTER_H
#define STREAM_CONVERTER_H

#include "generic.h"
#include <algorithm>
#include <map>

class scratchspaceType{
public:
	void *scratchspace;
	int cur_capacity;
	int cur_size;

	scratchspaceType() : scratchspace(NULL), cur_capacity(0), cur_size(0) {};
};

template <typename T>
class streamConverter {
public:
	streamConverter();
	int convertTo(T *in_data, int num_bytes, primType result_type);
	int convertFrom(void *in_data, int num_bytes, primType start_type);
	void *getResult();
private:
	int sizeOfType(primType in_type);

	//Scratchspace used in convertFrom()
	scratchspaceType t_scratch;

	//Current type points to the last type sent to convertTo()
	primType cur_type;

	//Map to different scratchspaces used in convertTo()
	std::map<primType, scratchspaceType> scratchspace_map;
};

template <typename T>
streamConverter<T>::streamConverter(){
}

template <typename TO, typename FROM>
int convertWorker(scratchspaceType &in_scratch, FROM *in_from_array, int num_bytes){
	//First count the number of actual elements
	in_scratch.cur_size = num_bytes/sizeof(FROM);

	//Resize the desired array if necessary
	if(in_scratch.cur_size > in_scratch.cur_capacity){
		delete [] (TO*)in_scratch.scratchspace;
		in_scratch.scratchspace = new TO[in_scratch.cur_size*2];
		in_scratch.cur_capacity = in_scratch.cur_size*2;
	}

	//Do a standard copy and type conversion
	std::copy(in_from_array, in_from_array+in_scratch.cur_size, (TO*)(in_scratch.scratchspace));

	//Return the number of resulting bytes
	return in_scratch.cur_size*sizeof(TO);
}

template <typename T>
int streamConverter<T>::convertTo(T *in_data, int num_bytes, primType result_type){
	int resulting_bytes;
	cur_type = result_type;
	
	switch(result_type){
		case DOUBLE:
			resulting_bytes = convertWorker<double, T>(scratchspace_map[result_type], in_data, num_bytes);
			break;

		case FLOAT:
			resulting_bytes = convertWorker<float, T>(scratchspace_map[result_type], in_data, num_bytes);
			break;

		case INT32:
			resulting_bytes = convertWorker<int32_t, T>(scratchspace_map[result_type], in_data, num_bytes);
			break;

		case INT16:
			resulting_bytes = convertWorker<int16_t, T>(scratchspace_map[result_type], in_data, num_bytes);
			break;

		case INT8:
			resulting_bytes = convertWorker<int8_t, T>(scratchspace_map[result_type], in_data, num_bytes);
			break;

		default:
			//SHOULD NOT GET HERE!
			break;
	}

	return resulting_bytes;
}

template <typename T>
int streamConverter<T>::convertFrom(void *in_data, int num_bytes, primType start_type){
	//First count the number of actual elements
	int num_elements = num_bytes/sizeOfType(start_type);

	int resulting_bytes = num_elements*sizeof(T);
	if(num_elements > t_scratch.cur_capacity){
		delete [] t_scratch.scratchspace;
		t_scratch.scratchspace = new T[num_elements*2];
		t_scratch.cur_capacity = num_elements*2;
	}

	switch(start_type){
		case DOUBLE:
			std::copy((double*)in_data, ((double*)in_data) + num_elements, t_scratch.scratchspace);
			break;

		case FLOAT:
			std::copy((float*)in_data, ((float*)in_data) + num_elements, t_scratch.scratchspace);
			break;

		case INT32:
			std::copy((int32_t*)in_data, ((int32_t*)in_data) + num_elements, t_scratch.scratchspace);
			break;

		case INT16:
			std::copy((int16_t*)in_data, ((int16_t*)in_data) + num_elements, t_scratch.scratchspace);
			break;

		case INT8:
			std::copy((int8_t*)in_data, ((int8_t*)in_data) + num_elements, t_scratch.scratchspace);
			break;
	}

	return resulting_bytes;
}

template <typename T>
int streamConverter<T>::sizeOfType(primType in_type){
	if(in_type == DOUBLE)
		return sizeof(double);
	else if(in_type == FLOAT)
		return sizeof(float);
	else if(in_type == INT32)
		return sizeof(int32_t);
	else if(in_type == INT16)
		return sizeof(int16_t);
	else if(in_type == INT8)
		return sizeof(int8_t);
}

template <typename T>
void *streamConverter<T>::getResult(){
	return scratchspace_map[cur_type].scratchspace;
}

#endif

