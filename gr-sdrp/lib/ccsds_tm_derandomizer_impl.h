#ifndef INCLUDED_CCSDS_TM_DERANDOMIZER_IMPL_H
#define INCLUDED_CCSDS_TM_DERANDOMIZER_IMPL_H

#include <sdrp/ccsds_tm_derandomizer.h>

namespace gr {
namespace sdrp {

class SDRP_API ccsds_tm_derandomizer_impl : public ccsds_tm_derandomizer
{
private:

	int d_count;
	bool d_rand_en;
	pmt::pmt_t d_correlate_key;
	std::vector<uint8_t> d_pn;

public:
	ccsds_tm_derandomizer_impl(const std::string &tag_name);

	//Member functions callable from python
	virtual void setDerandomizerEn(bool rand_en);

	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);
};

} /* namespace sdrp */
} /* namespace gr */

#endif
