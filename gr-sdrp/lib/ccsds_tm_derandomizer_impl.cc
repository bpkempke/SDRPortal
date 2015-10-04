#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ccsds_tm_derandomizer_impl.h"
#include <gnuradio/io_signature.h>
#include <cstdio>

namespace gr {
namespace sdrp {

ccsds_tm_derandomizer::sptr ccsds_tm_derandomizer::make(const std::string &tag_name){
	return gnuradio::get_initial_sptr(new ccsds_tm_derandomizer_impl(tag_name));
}

ccsds_tm_derandomizer_impl::ccsds_tm_derandomizer_impl(const std::string &tag_name)
	: sync_block("ccsds_tm_derandomizer",
			io_signature::make (1, 1, sizeof(float)),
			io_signature::make (1, 1, sizeof(float)))
{
	d_correlate_key = pmt::string_to_symbol(tag_name);

	//Generate pseudo-randomizer (it might make more sense to put this in its own block instead)
	uint8_t cur_pn = 0xFF;
	for(int ii=0; ii < 255; ii++){
		uint8_t x7 = (cur_pn & 0x80) > 0;
		uint8_t x5 = (cur_pn & 0x20) > 0;
		uint8_t x3 = (cur_pn & 0x08) > 0;
		uint8_t x0 = (cur_pn & 0x01) > 0;
		d_pn.push_back(cur_pn & 0x01);
		uint8_t fb = x7 ^ x5 ^ x3 ^ x0;
		cur_pn >>= 1;
		if(fb) cur_pn |= 0x80;
	}

}

//Enable/disable de-randomizer functionality
void ccsds_tm_derandomizer_impl::setDerandomizerEn(bool rand_en){
	d_rand_en = rand_en;
}

int ccsds_tm_derandomizer_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const float *in = (const float *)input_items[0];
	float *out = (float *)output_items[0];

	int noutput_ret = 0;
	uint64_t new_count = d_count;

	std::vector<tag_t> tags;
	const uint64_t nread = this->nitems_read(0);
	this->get_tags_in_range(tags, 0, nread, nread+noutput_items);
	for(int ii=0; ii < tags.size(); ii++){
		if(tags[ii].key == d_correlate_key){
			std::cout << "FOUND TAG" << std::endl;
			new_count = tags[ii].offset+1;
		}
	}

	for (int i = 0; i < noutput_items; i++) {
		if(d_rand_en && nread+i > new_count)
			out[noutput_ret++] = (d_pn[(nread+i-new_count)%255]) ? -in[i] : in[i];
		else if(d_rand_en)
			out[noutput_ret++] = (d_pn[(nread+i-d_count)%255]) ? -in[i] : in[i];
		else
			out[noutput_ret++] = in[i];
	}

	d_count = new_count;

	return noutput_ret;
}

} /* namespace sdrp */
}/* namespace gr */
