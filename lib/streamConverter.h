#ifndef STREAM_CONVERTER_H
#define STREAM_CONVERTER_H

#include "generic.h"

template <typename T>
class streamConverter {
public:
	streamConverter();
	int convertTo(T *in_data, int num_bytes, primType result_type);
	int convertFrom(void *in_data, int num_bytes, primType start_type);
	void *getResult();
private:
	int sizeOfType(primType in_type);
	T *scratchspace;
	int scratchspace_size;
	double *d_scratchspace;
	float *f_scratchspace;
	int32_t *i32_scratchspace;
	int16_t *i16_scratchspace;
	int8_t *i8_scratchspace;
	void *cur_scratchspace;
	int d_size, f_size, i32_size, i16_size, i8_size;
};

template <typename T>
streamConverter<T>::streamConverter(){
	d_size = f_size = i32_size = i16_size = i8_size = scratchspace_size = 0;
	scratchspace = NULL;
	d_scratchspace = NULL;
	f_scratchspace = NULL;
	i32_scratchspace = NULL;
	i16_scratchspace = NULL;
	i8_scratchspace = NULL;
}

template <typename T>
int streamConverter<T>::convertTo(T *in_data, int num_bytes, primType result_type){
	//First count the number of actual elements
	int num_elements = num_bytes/sizeof(T);

	int resulting_bytes = num_bytes;
	
	//TODO: Any suggestions on how to not do so much code duplication here?
	switch(result_type){
		case DOUBLE:{
			if(num_elements > d_size){
				delete [] d_scratchspace;
				d_scratchspace = new double[num_elements*2];
				d_size = num_elements*2;
			}
			for(int ii=0; ii < num_elements; ii++)
				d_scratchspace[ii] = (double)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(double);
			cur_scratchspace = d_scratchspace;
			}
			break;

		case FLOAT:{
			if(num_elements > f_size){
				delete [] f_scratchspace;
				f_scratchspace = new float[num_elements*2];
				f_size = num_elements*2;
			}
			for(int ii=0; ii < num_elements; ii++)
				f_scratchspace[ii] = (float)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(float);
			cur_scratchspace = f_scratchspace;
			}
			break;

		case INT32:{
			if(num_elements > i32_size){
				delete [] i32_scratchspace;
				i32_scratchspace = new int32_t[num_elements*2];
				i32_size = num_elements*2;
			}
			for(int ii=0; ii < num_elements; ii++)
				i32_scratchspace[ii] = (int32_t)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(int32_t);
			cur_scratchspace = i32_scratchspace;
			}
			break;

		case INT16:{
			if(num_elements > i16_size){
				delete [] i16_scratchspace;
				i16_scratchspace = new int16_t[num_elements*2];
				i16_size = num_elements*2;
			}
			for(int ii=0; ii < num_elements; ii++)
				i16_scratchspace[ii] = (int16_t)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(int16_t);
			cur_scratchspace = i16_scratchspace;
			}
			break;

		case INT8:{
			if(num_elements > i8_size){
				delete [] i8_scratchspace;
				i8_scratchspace = new int8_t[num_elements*2];
				i8_size = num_elements*2;
			}
			for(int ii=0; ii < num_elements; ii++)
				i8_scratchspace[ii] = (int8_t)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(int8_t);
			cur_scratchspace = i8_scratchspace;
			}
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

	int resulting_bytes = num_elements;
	if(resulting_bytes > scratchspace_size){
		delete [] scratchspace;
		scratchspace = new T[num_elements*2];
		scratchspace_size = num_elements*2;
	}

	//TODO: Any suggestions on how to not do so much code duplication here?
	switch(start_type){
		case DOUBLE:
			for(int ii=0; ii < num_elements; ii++)
				scratchspace[ii] = (T)(((double*)in_data)[ii]);
			break;

		case FLOAT:
			for(int ii=0; ii < num_elements; ii++)
				scratchspace[ii] = (T)(((float*)in_data)[ii]);
			break;

		case INT32:
			for(int ii=0; ii < num_elements; ii++)
				scratchspace[ii] = (T)(((int32_t*)in_data)[ii]);
			break;

		case INT16:
			for(int ii=0; ii < num_elements; ii++)
				scratchspace[ii] = (T)(((int16_t*)in_data)[ii]);
			break;

		case INT8:
			for(int ii=0; ii < num_elements; ii++)
				scratchspace[ii] = (T)(((int8_t*)in_data)[ii]);
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
	return cur_scratchspace;
}

#endif

