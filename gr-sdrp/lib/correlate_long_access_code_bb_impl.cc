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

#include "correlate_long_access_code_bb_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/count_bits.h>
#include <stdexcept>
#include <cstdio>

namespace gr {
namespace sdrp {

#define VERBOSE 0

correlate_long_access_code_bb::sptr
	correlate_long_access_code_bb::make(const std::string &access_code, int threshold)
	{
		return gnuradio::get_initial_sptr
			(new correlate_long_access_code_bb_impl(access_code, threshold));
	}

correlate_long_access_code_bb_impl::correlate_long_access_code_bb_impl(
		const std::string &access_code, int threshold)
	: sync_block("correlate_long_access_code_bb",
			io_signature::make(1, 1, sizeof(char)),
			io_signature::make(1, 1, sizeof(char))),
	d_access_code(NULL), d_data_reg(NULL), d_mask(NULL), d_len_64(0),
	d_threshold(threshold)
{
	set_access_code(access_code);
}

correlate_long_access_code_bb_impl::~correlate_long_access_code_bb_impl()
{
}

bool correlate_long_access_code_bb_impl::set_access_code(
		const std::string &access_code)
{
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
		d_mask[ii/64] |= 1ULL << (ii % 64);
		if(access_code[ii] & 1)
			d_access_code[ii/64] |= 1ULL << (ii % 64);
	}

	return true;
}

void correlate_long_access_code_bb_impl::set_threshold(unsigned int new_threshold){
	d_threshold = new_threshold;
}

int correlate_long_access_code_bb_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const unsigned char *in = (const unsigned char*)input_items[0];
	unsigned char *out = (unsigned char*)output_items[0];

	for(int i = 0; i < noutput_items; i++) {
		//compute overflow
		for(unsigned int ii=d_len_64-1; ii >= 0; ii--){
			d_data_reg[ii] <<= 1;
			if(ii > 0 && (d_data_reg[ii-1] & 0x8000000000000000ULL))
				d_data_reg[ii] |= 1ULL;
		}
		if(in[i] & 1)
			d_data_reg[0] |= 1ULL;
		out[i] = in[i] & 1;

		// compute hamming distance between desired access code and current data
		unsigned long long wrong_bits = 0;
		unsigned int nwrong = 0;

		for(unsigned int ii=0; ii < d_len_64; ii++){
			wrong_bits = (d_data_reg[ii] ^ d_access_code[ii]) & d_mask[ii];
			nwrong += gr::blocks::count_bits64(wrong_bits);
		}

		if(nwrong < d_threshold)
			out[i] |= 2;
	}

	return noutput_items;
}

} /* namespace digital */
} /* namespace gr */
