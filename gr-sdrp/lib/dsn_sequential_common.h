#ifndef INCLUDED_SDRP_DSN_SEQUENTIAL_COMMON_H
#define INCLUDEd_SDRP_DSN_SEQUENTIAL_COMMON_H

namespace gr {
namespace sdrp {

//Refer to DSN document for terminology used in this block:
// deepspace.jpl.nasa.gov/dsndocs/810-005/203/203C.pdf
enum sequenceState {
	SEQ_T1, SEQ_T1_POST, SEQ_T2_PRE, SEQ_T2, SEQ_T2_POST
};

struct sequenceType {
	double f0;
	double downlink_freq;
	uint64_t XMIT;
	uint64_t RXTIME;
	uint64_t T1;
	uint64_t T2;
	int range_clk_component;
	int chop_component;
	int end_component;
	unsigned int interp_factor;
	bool range_is_square;
	bool done;
	bool running;
	sequenceState state;
};

inline bool compare_sequence_start(const sequenceType &first, const sequenceType &second){
	if(first.XMIT < second.XMIT)
		return true;
	else
		return false;
}

} /* namespace sdrp */
} /* namespace gr */

#endif
