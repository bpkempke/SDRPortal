namespace gr {
namespace sdrp {

enum combinationMethod {
	CM_AND, CM_OR, CM_XOR, CM_VOTE
};

struct PNComposite {
	combinationMethod cm;
	uint64_t xmit_time;
	uint64_t rx_time;
	double T;
	std::vector<std::vector<bool> > components;
	double range_freq;
	bool range_is_square;
	bool done;
	bool running;
};

bool compare_composite_start(const PNComposite &first, const PNComposite &second){
	if(first.xmit_time < second.xmit_time)
		return true;
	else
		return false;
}

} /* namespace sdrp */
} /* namespace gr */
