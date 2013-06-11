#include "streamConverter.h"

template <typename T>
streamConverter<T>::streamConverter(){
	scratchspace = new int[512];
	scratchspace_size = 512*sizeof(int);
}

template <typename T>
int streamConverter<T>::convertTo(T *in_data, int num_bytes, primType result_type){
	//First count the number of actual elements
	int num_elements = num_bytes/sizeof(T);

	int resulting_bytes = num_bytes;
	
	//TODO: Any suggestions on how to not do so much code duplication here?
	switch(result_type){
		case DOUBLE:{
			double *d_scratchspace = (double *)scratchspace;
			if(num_elements*sizeof(double) > scratchspace_size){
				d_scratchspace = new double[num_elements*2];
				scratchspace_size = num_elements*2*sizeof(double);
			}
			for(int ii=0; ii < num_elements; ii++)
				d_scratchspace[ii] = (double)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(double);
			}
			break;

		case FLOAT:{
			float *d_scratchspace = (float *)scratchspace;
			if(num_elements*sizeof(float) > scratchspace_size){
				d_scratchspace = new float[num_elements*2];
				scratchspace_size = num_elements*2*sizeof(float);
			}
			for(int ii=0; ii < num_elements; ii++)
				d_scratchspace[ii] = (float)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(float);
			}
			break;

		case INT32:{
			int32_t *d_scratchspace = (int32_t *)scratchspace;
			if(num_elements*sizeof(int32_t) > scratchspace_size){
				d_scratchspace = new int32_t[num_elements*2];
				scratchspace_size = num_elements*2*sizeof(int32_t);
			}
			for(int ii=0; ii < num_elements; ii++)
				d_scratchspace[ii] = (int32_t)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(int32_t);
			}
			break;

		case INT16:{
			int16_t *d_scratchspace = (int16_t *)scratchspace;
			if(num_elements*sizeof(int16_t) > scratchspace_size){
				d_scratchspace = new int16_t[num_elements*2];
				scratchspace_size = num_elements*2*sizeof(int16_t);
			}
			for(int ii=0; ii < num_elements; ii++)
				d_scratchspace[ii] = (int16_t)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(int16_t);
			}
			break;

		case INT8:{
			int8_t *d_scratchspace = (int8_t *)scratchspace;
			if(num_elements*sizeof(int8_t) > scratchspace_size){
				d_scratchspace = new int8_t[num_elements*2];
				scratchspace_size = num_elements*2*sizeof(int8_t);
			}
			for(int ii=0; ii < num_elements; ii++)
				d_scratchspace[ii] = (int8_t)(in_data[ii]);
			resulting_bytes = num_elements*sizeof(int8_t);
			}
			break;
	}

	return resulting_bytes;
}

template <typename T>
int streamConverter<T>::convertFrom(void *in_data, int num_bytes, primType start_type){
	//First count the number of actual elements
	int num_elements = num_bytes/sizeOfType(start_type);

	int resulting_bytes = num_elements*sizeof(T);
	if(resulting_bytes > scratchspace_size){
		scratchspace = new T[num_elements*2];
		scratchspace_size = sizeof(T)*num_elements*2;
	}

	T *r_scratchspace = (T *)scratchspace;
	
	//TODO: Any suggestions on how to not do so much code duplication here?
	switch(start_type){
		case DOUBLE:
			for(int ii=0; ii < num_elements; ii++)
				r_scratchspace[ii] = (T)(((double*)in_data)[ii]);
			break;

		case FLOAT:
			for(int ii=0; ii < num_elements; ii++)
				r_scratchspace[ii] = (T)(((float*)in_data)[ii]);
			break;

		case INT32:
			for(int ii=0; ii < num_elements; ii++)
				r_scratchspace[ii] = (T)(((int32_t*)in_data)[ii]);
			break;

		case INT16:
			for(int ii=0; ii < num_elements; ii++)
				r_scratchspace[ii] = (T)(((int16_t*)in_data)[ii]);
			break;

		case INT8:
			for(int ii=0; ii < num_elements; ii++)
				r_scratchspace[ii] = (T)(((int8_t*)in_data)[ii]);
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
	return scratchspace;
}

