/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
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

#include "ccsds_tm_conv_decoder_impl.h"
#include <gnuradio/io_signature.h>
#include <cstdio>

#define NUM_HISTORY 64

namespace gr {
namespace sdrp {

ccsds_tm_conv_decoder::sptr ccsds_tm_conv_decoder::make(const std::string &tag_name){
	return gnuradio::get_initial_sptr(new ccsds_tm_conv_decoder_impl(tag_name));
}

ccsds_tm_conv_decoder_impl::ccsds_tm_conv_decoder_impl(const std::string &tag_name)
	: block("ccsds_tm_conv_decoder",
			io_signature::make (1, 1, sizeof(float)),
			io_signature::make (1, 1, sizeof(char)))
{
	d_correlate_key = pmt::string_to_symbol(tag_name);

	//Start off with convolutional decoding disabled
	d_conv_en = false;

	resetViterbi();

	//Create a circular queue for history because GNURadio doesn't handle tags right
	d_history = new float[NUM_HISTORY];
	d_history_idx = 0;

	nwritten = 0;

	d_negate = false;
}

//Enable/disable Convolutional decoding functionality
void ccsds_tm_conv_decoder_impl::setConvEn(bool conv_en){
	d_conv_en = conv_en;
}

void ccsds_tm_conv_decoder_impl::resetViterbi(){
	d_count = 0;
	float RATE = 0.5;
	float ebn0 = 12.0;
	float esn0 = RATE*pow(10.0, ebn0/10.0);
	gen_met(d_mettab, 100, esn0, 0.0, 256);
	viterbi_chunks_init(d_state0);
	viterbi_chunks_init(d_state1);
}

void ccsds_tm_conv_decoder_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required){
	if(d_conv_en)
		ninput_items_required[0] = noutput_items*2; //Rate 1/2 code
	else
		ninput_items_required[0] = noutput_items; //Either uncoded or coded after binary slicing
}

int ccsds_tm_conv_decoder_impl::general_work(int noutput_items,
		gr_vector_int &ninput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const float *in = (const float *)input_items[0];
	unsigned char *out = (unsigned char *)output_items[0];

	int noutput_ret = 0;
	int consume_count = ninput_items[0];

	//Handle corner case of inconvenient mode switch timing
	if(d_conv_en && (consume_count > (noutput_items-8)*2))
		consume_count = (noutput_items-8)*2;
	else if(!d_conv_en && (consume_count > noutput_items))
		consume_count = noutput_items;

	//Re-index all tags passing through appropriately
	std::vector<tag_t> tags;
	const uint64_t nread = this->nitems_read(0);
	this->get_tags_in_range(tags, 0, nread, nread+consume_count);
	for(int ii=0; ii < tags.size(); ii++){
		//First check for any negation events that may cause issues
		if(tags[ii].key == d_correlate_key){
			bool data = pmt::to_bool(tags[ii].value);
			if(data != d_negate){
				d_negate = data;
				std::cout << "Found data negation. d_negate = " << d_negate << std::endl;
				if(d_conv_en){
					//TODO: Will resetViterbi cause issues down the line with lining up tag indices?
					resetViterbi();//Reset the viterbi decoder due to change in symbol polarity
					//Re-populate Viterbi decoder with historic data
					int d_count_temp = 0;
					int temp_idx = d_history_idx-NUM_HISTORY/2-(d_count % 16);
					if(temp_idx < 0) temp_idx += NUM_HISTORY;
					while(temp_idx != d_history_idx){
						// Translate and clip [-1.0..1.0] to [28..228]
						float sample = (d_negate) ? -d_history[temp_idx]*100.0+128.0 : d_history[temp_idx]*100.0+128.0;
						if (sample > 255.0)
							sample = 255.0;
						else if (sample < 0.0)
							sample = 0.0;
						unsigned char sym = (unsigned char)(floor(sample));

						//sym == 128 means erased, so let's avoid that
						sym = (sym == 128) ? 129 : sym;

						d_viterbi_in[d_count_temp % 4] = sym;
						if ((d_count_temp % 4) == 3) {
							// Every fourth symbol, perform butterfly operation
							viterbi_butterfly2(d_viterbi_in, d_mettab, d_state0, d_state1);

							// Every sixteenth symbol, read out a byte
							if (d_count_temp % 16 == 11) {
								unsigned char cur_byte;
								viterbi_get_output(d_state0, &cur_byte);
							}
						}

				

						//Increment idx for next sample
						temp_idx++;
						if(temp_idx == NUM_HISTORY) temp_idx = 0;
					}
				}
			}
		}

		remove_item_tag(0,tags[ii]);
		if(d_conv_en)
			tags[ii].offset = nwritten+(tags[ii].offset-nread)/2+40;
		else
			tags[ii].offset += nwritten-nread;
		add_item_tag(0,tags[ii]);
	}

	for (int i = 0; i < consume_count; i++) {
		d_history[d_history_idx++] = in[i];
		if(d_history_idx == NUM_HISTORY)
			d_history_idx = 0;
		if(d_conv_en){
			// Translate and clip [-1.0..1.0] to [28..228]
			float sample = (d_negate) ? -in[i]*100.0+128.0 : in[i]*100.0+128.0;
			if (sample > 255.0)
				sample = 255.0;
			else if (sample < 0.0)
				sample = 0.0;
			unsigned char sym = (unsigned char)(floor(sample));

			//sym == 128 means erased, so let's avoid that
			sym = (sym == 128) ? 129 : sym;

			d_viterbi_in[d_count % 4] = sym;
			if ((d_count % 4) == 3) {
				// Every fourth symbol, perform butterfly operation
				viterbi_butterfly2(d_viterbi_in, d_mettab, d_state0, d_state1);

				// Every sixteenth symbol, read out a byte
				if (d_count % 16 == 11) {
					unsigned char cur_byte;
					viterbi_get_output(d_state0, &cur_byte);
					for(int ii=0; ii<8; ii++)
						out[noutput_ret++] = (cur_byte & (0x80 >> ii)) ? 1 : 0;
				}
			}

			d_count++;
		} else {
			//Otherwise just do a straight binary slice
			out[noutput_ret++] = (d_negate) ? (in[i] < 0.0) : (in[i] > 0.0);
		}
	}

	//Always consume all items from input no matter what.
	//TODO: How does this work with history?
	consume(0,consume_count);

	//Variable number of output symbols in the case of convolutional encoding
	nwritten += noutput_ret;
	return noutput_ret;
}

} /* namespace sdrp */
}/* namespace gr */
