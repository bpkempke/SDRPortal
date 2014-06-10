/* -*- c++ -*- */
/*
 * Copyright 2004,2006,2010-2012 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "correlate_soft_access_tag_ff_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/count_bits.h>
#include "fec/fec.h"
#include <stdexcept>
#include <cstdio>

/********************************
 * Convolutional Encoding Logic *
 ********************************/
#define	POLYA	0x4f
#define	POLYB	0x6d

namespace gr {
namespace sdrp {

#define VERBOSE 0

correlate_soft_access_tag_ff::sptr
	correlate_soft_access_tag_ff::make(const std::string &access_code, int threshold, const std::string &tag_name)
	{
		return gnuradio::get_initial_sptr
			(new correlate_soft_access_tag_ff_impl(access_code, threshold, tag_name));
	}

correlate_soft_access_tag_ff_impl::correlate_soft_access_tag_ff_impl(
		const std::string &access_code, int threshold, const std::string &tag_name)
	: sync_block("correlate_soft_access_tag_ff",
			io_signature::make(1, 1, sizeof(float)),
			io_signature::make(1, 1, sizeof(float))),
	d_access_code(NULL), d_data_reg(NULL), d_mask(NULL), d_len_64(0), d_conv_en(false),
	d_cur_access_code(""), d_threshold(threshold)
{
	//Check to make sure there IS an access code first
	if(access_code.length() == 0)
		throw std::invalid_argument("correlate_soft_access_tag_ff: Access code length must be >0!");

	set_access_code(access_code);

	std::stringstream str;
	str << name() << unique_id();
	d_me = pmt::string_to_symbol(str.str());
	d_key = pmt::string_to_symbol(tag_name);

	d_negate = false;
	
	//Set up fast parity lookup table
	partab_init();
}

correlate_soft_access_tag_ff_impl::~correlate_soft_access_tag_ff_impl()
{
}

void correlate_soft_access_tag_ff_impl::set_conv_en(bool conv_en){
	d_conv_en = conv_en;
	set_access_code(d_cur_access_code);
}

bool correlate_soft_access_tag_ff_impl::set_access_code(std::string access_code)
{
	d_cur_access_code = access_code;
	if(d_conv_en){
		//Start from scratch with string
		access_code = "";

		//If doing conv encoding, encode to bitstream and chop first and last 16 bits off since they won't be fixed
		unsigned char d_enc_state = 0;
		unsigned char new_access_code_char = 0;
		for(int ii=0; ii < d_cur_access_code.size(); ii++){
    			d_enc_state = (d_enc_state << 1) | (d_cur_access_code[ii] & 1);
			access_code.push_back((parityb(d_enc_state & POLYA)) ? 0x31 : 0x30);
			access_code.push_back((1-parityb(d_enc_state & POLYB)) ? 0x31 : 0x30);
		}

		//First two and last two bytes are constant...
		access_code.erase(0,16);
		access_code.erase(access_code.size()-16,16);
	}

	//Print to stdout to verify...
	/*printf("access_code = ");
	for(int ii=0; ii < access_code.size(); ii++){
		printf("%02X",access_code[ii]);
	}
	printf("\n");*/

	unsigned len = access_code.length();	// # of bytes in string
	d_len_64 = (len+63)/64;

	//Delete arrays and start anew
	delete [] d_access_code;
	delete [] d_data_reg;
	delete [] d_mask;

	d_access_code = new unsigned long long[d_len_64];
	d_data_reg = new unsigned long long[d_len_64];
	d_mask = new unsigned long long[d_len_64];

	for(unsigned ii=0; ii < d_len_64; ii++){
		d_access_code[ii] = 0ULL;
		d_data_reg[ii] = 0ULL;
		d_mask[ii] = 0ULL;
	}

	for(unsigned ii=0; ii < len; ii++){
		d_mask[(len-ii-1)/64] |= 1ULL << ((len-ii-1) % 64);
		if(access_code[ii] & 1)
			d_access_code[(len-ii-1)/64] |= 1ULL << ((len-ii-1) % 64);
	}

	for(unsigned ii=0; ii < d_len_64; ii++){
		printf("%llX\n",d_access_code[ii]);
	}

	return true;
}

void correlate_soft_access_tag_ff_impl::set_threshold(unsigned int new_threshold){
	d_threshold = new_threshold;
}

int correlate_soft_access_tag_ff_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const float *in = (float*)input_items[0];
	float *out = (float*)output_items[0];

	uint64_t abs_out_sample_cnt = nitems_written(0);

	for(int i = 0; i < noutput_items; i++) {
		//compute overflow
		for(int ii=d_len_64-1; ii >= 0; ii--){
			d_data_reg[ii] <<= 1;
			if(ii > 0 && (d_data_reg[ii-1] & 0x8000000000000000ULL))
				d_data_reg[ii] |= 1ULL;
		}
		if(in[i] > 0)
			d_data_reg[0] |= 1ULL;
		out[i] = in[i];

		// compute hamming distance between desired access code and current data
		unsigned long long wrong_bits = 0;
		unsigned int nwrong = 0;
		unsigned int nwrong_n = 0;

		for(unsigned int ii=0; ii < d_len_64; ii++){
			wrong_bits = (d_data_reg[ii] ^ d_access_code[ii]) & d_mask[ii];
			nwrong += gr::blocks::count_bits64(wrong_bits);

			//Also calculate negated bitcount in case constellation is off
			wrong_bits = (~(d_data_reg[ii] ^ d_access_code[ii])) & d_mask[ii];
			nwrong_n += gr::blocks::count_bits64(wrong_bits);
		}

		if(nwrong <= d_threshold || nwrong_n <= d_threshold){
			if(nwrong_n <= d_threshold)
				d_negate = true;
			else
				d_negate = false;

			uint64_t offset = (d_conv_en) ? 80 : 0;
			add_item_tag(0, //stream ID
				abs_out_sample_cnt + i + offset, //sample
				d_key,      //frame info
				pmt::from_bool(d_negate), //data (unused)
				d_me        //block src id
				);
		}
	}

	return noutput_items;
}

} /* namespace digital */
} /* namespace gr */
