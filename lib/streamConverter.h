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
	void *scratchspace;
	int scratchspace_size;
};

#endif

