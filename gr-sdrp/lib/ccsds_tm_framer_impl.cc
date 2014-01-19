/* -*- c++ -*- */
/*
 * Copyright 2004,2006,2010,2012 Free Software Foundation, Inc.
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

#include "ccsds_tm_framer_impl.h"
#include "fec/viterbi27_port.h"
#include "fec/encode_rs_ccsds.h"
#include "fec/decode_rs_ccsds.h"
#include "fec/fixed.h"
#include <gnuradio/io_signature.h>
#include <cstdio>
#include <string>

namespace gr {
namespace sdrp {

inline void ccsds_tm_framer_impl::enter_search(){
	d_state = STATE_SYNC_SEARCH;
}

inline void ccsds_tm_framer_impl::enter_have_sync(){
	d_state = STATE_HAVE_SYNC;
	d_header = 0;
	d_headerbitlen_cnt = 0;
}

inline void ccsds_tm_framer_impl::enter_have_header(int payload_len, int whitener_offset){
	d_state = STATE_HAVE_HEADER;
	d_packetlen = payload_len;
	d_packet_whitener_offset = whitener_offset;
	d_packetlen_cnt = 0;
	d_packet_byte = 0;
	d_packet_byte_index = 0;
}

ccsds_tm_framer::sptr ccsds_tm_framer::make(unsigned packet_id, const std::string &tag_name){
	return gnuradio::get_initial_sptr
		(new ccsds_tm_framer_impl(packet_id, tag_name));
}

ccsds_tm_framer_impl::ccsds_tm_framer_impl(unsigned packet_id, const std::string &tag_name)
	: sync_block("ccsds_tm_framer",
			io_signature::make(1, 1, sizeof(char)),
			io_signature::make(0, 0, 0)),
	d_frame_len(223*8),
	d_r_mult(1), d_r_div(1),
	d_rs_e(16), d_rs_i(1), d_rs_q(0),
	d_turbo_k(8920),
	d_ldpc_k(7136),
	d_vp(NULL),
	d_num_times(0),
	d_frames_correct(0),
	d_frames_incorrect(0),
	d_packet_id(packet_id)
{
	d_correlate_key = pmt::string_to_symbol(tag_name);

	message_port_register_out(pmt::mp("tm_frame_out"));
	resetDecoder();

	//Set up the reed-solomon decoder
}

ccsds_tm_framer_impl::~ccsds_tm_framer_impl(){
}

void ccsds_tm_framer_impl::setFrameLength(unsigned int num_bits){
	if(num_bits > 0){
		d_frame_len = num_bits;
		resetDecoder();
	
		//Recreate the convolutional encoder
		delete_viterbi27_port(d_vp);
		d_vp = create_viterbi27_port(num_bits);
	}
}
      
void ccsds_tm_framer_impl::setCodeRate(unsigned int r_mult, unsigned int r_div){
	d_r_mult = r_mult;
	d_r_div = r_div;
	resetDecoder();
}

void ccsds_tm_framer_impl::setCodingMethod(std::string in_method){
	if(in_method == "CONV"){
		d_r_mult = 2;
		d_r_div = 1;
		d_coding_method = METHOD_CONV;
	}else if(in_method == "RS"){
		d_coding_method = METHOD_RS;
		d_r_mult = 255;
		d_r_div = 223;
	}else if(in_method == "CC")
		d_coding_method = METHOD_CC;
	else if(in_method == "Turbo")
		d_coding_method = METHOD_TURBO;
	else if(in_method == "LDPC")
		d_coding_method = METHOD_LDPC;
	else{
		d_r_mult = 1;
		d_r_div = 1;
		d_coding_method = METHOD_NONE;
	}
	resetDecoder();
}

void ccsds_tm_framer_impl::setCodingParameter(std::string param_name, std::string param_val){
	if(d_coding_method == METHOD_RS){
		if(param_name == "E")
			d_rs_e = (unsigned int)atoi(param_val.c_str());
		else if(param_name == "I")
			d_rs_i = (unsigned int)atoi(param_val.c_str());
		else if(param_name == "Q")
			d_rs_q = (unsigned int)atoi(param_val.c_str());
	} else if(d_coding_method == METHOD_TURBO){
		if(param_name == "K")
			d_turbo_k = (unsigned int)atoi(param_val.c_str());
	} else if(d_coding_method == METHOD_LDPC){
		if(param_name == "K")
			d_ldpc_k = (unsigned int)atoi(param_val.c_str());
	}

	resetDecoder();
}

void ccsds_tm_framer_impl::resetDecoder(){
	//Calculate the number of bits in the packet (including encoding...)
	d_tot_bits = d_frame_len * d_r_mult / d_r_div;
	if(d_coding_method == METHOD_RS)
		d_tot_bits *= d_rs_i;
	enter_search();
}

void ccsds_tm_framer_impl::performHardDecisions(std::vector<uint8_t> &packet_data){
	//Make hard decisions based on each bit depending on if it's greater than or less than zero
	uint8_t cur_byte = 0;
	for(unsigned ii=0; ii < d_symbol_hist.size(); ii++){
		unsigned cur_bit_pos = d_symbol_hist.size() - ii - 1;
		unsigned byte_pos = cur_bit_pos % 8;
		if(d_symbol_hist[ii])
			cur_byte |= 1 << byte_pos;
		if(byte_pos == 0){
			packet_data.push_back(cur_byte);
			cur_byte = 0;
		}
	}
}

int ccsds_tm_framer_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	std::vector<tag_t> tags;
	const uint64_t nread = nitems_read(0);
	const float *in = (const float *) input_items[0];
	int count=0;
	bool found_tag;

	while(count < noutput_items){
		switch(d_state) {

			case STATE_SYNC_SEARCH:    // Look for tags indicating beginning of pkt
				get_tags_in_range(tags, 0, nread+count, nread+noutput_items);
				found_tag = false;
				for(unsigned ii=0; ii < tags.size(); ii++){
					if(tags[ii].key == d_correlate_key){
						count = tags[ii].offset-nread;
						count++;
						enter_have_sync();
						std::cout << "entered have_sync " << d_num_times++ << " times" << std::endl;
						d_symbol_hist.clear();
						found_tag = true;
					}
				}
				if(!found_tag) count = noutput_items;
				break;

			case STATE_HAVE_SYNC:
				//Push soft bits into vector 
				while(d_symbol_hist.size() < d_tot_bits && count < noutput_items)
					d_symbol_hist.push_back(in[count++]);

				//If we're at the end of the packet, first try decoding it, then return to searching
				if(d_symbol_hist.size() >= d_tot_bits){
					bool valid_packet = false;
					std::vector<uint8_t> packet_data;
					if(d_coding_method == METHOD_NONE){
						//No coding required, push out all the bits regardless of whether they're right!
						performHardDecisions(packet_data);
						valid_packet = true;
					} else if(d_coding_method == METHOD_RS){
						//Need to pay attention to the interleaving depth here...
						//First of all, all decoding is based on hard decisions, so do that here...
						std::vector<uint8_t> hard_data;
						performHardDecisions(hard_data);

						//Then deinterleave if necessary
						std::vector<std::vector<uint8_t> > deinterleaved_data(d_rs_i);
						for(unsigned ii=0; ii < hard_data.size(); ii+=d_rs_i)
							for(unsigned jj=0; jj < d_rs_i; jj++)
								deinterleaved_data[jj].push_back(hard_data[ii+jj]);

						//Now decode each interleaved code separately
						valid_packet = true;
						int derrlocs[NROOTS];
						for(unsigned jj=0; jj < d_tot_bits/8/255; jj++){
							//Debug print received data out
							for(unsigned ii=0; ii < d_rs_i; ii++){
								std::cout << "Interleaved data set #" << ii << ": ";
								for(unsigned kk=0; kk < 223; kk++)
									printf("%02X", deinterleaved_data[ii][jj*255+kk]);
								std::cout << std::endl;
								std::cout << "RS check symbols:" << std::endl;
								for(unsigned kk=223; kk < 255; kk++)
									printf("%02X", deinterleaved_data[ii][jj*255+kk]);
								std::cout << std::endl;
								std::cout << "Expected RX check symbols:" << std::endl;
								std::vector<uint8_t> cur_packet_parity(NROOTS);
								encode_rs_ccsds(&(deinterleaved_data[ii][jj*255]), &cur_packet_parity[0], 0);
								for(unsigned kk=0; kk < NROOTS; kk++)
									printf("%02X", cur_packet_parity[kk]);
								std::cout << std::endl;
							}
	
							for(unsigned ii=0; ii < d_rs_i; ii++){
								memset(derrlocs,0,sizeof(derrlocs));
								int ret = decode_rs_ccsds(&(deinterleaved_data[ii][jj*255]), derrlocs, 0, 0);
								std::cout << "RX Interleave #" << ii << " check: " << ((ret == -1) ? "FAIL" : "SUCCESS") << std::endl;
								if(ret == -1)
									valid_packet = false;
							}
	
							for(unsigned ii=0; ii < 223; ii++)
								packet_data.push_back(deinterleaved_data[ii%d_rs_i][jj*255+ii]);
						}
					}
					if(valid_packet)
						d_frames_correct++;
					else
						d_frames_incorrect++;

					//Spit out a statistic reporting the number of correct and incorrect frames
					std::cout << "PACKET STATISTICS: " << d_frames_correct << " of " << d_frames_correct+d_frames_incorrect << " correct" << std::endl;
					

					//If there's a valid packet, put it into a message to send off upstream
					if(valid_packet){
						//Print out decoded message to screen for debugging purposes
						std::cout << "PACKET DATA: ";
						for(unsigned int ii=0; ii < packet_data.size(); ii++){
							printf("%02X",packet_data[ii]);
						}
						std::cout << std::endl;
					//	for(unsigned ii=0; ii < packet_data.size(); ii++)
					//		std::cout << (int)packet_data[ii] << ", ";

						pmt::pmt_t new_message_dict = pmt::make_dict();
						pmt::pmt_t key = pmt::from_long((long)(d_packet_id));
						pmt::pmt_t value = pmt::init_u8vector(packet_data.size(), (const uint8_t*)&packet_data[0]);
						new_message_dict = pmt::dict_add(new_message_dict, key, value);
						pmt::pmt_t new_message = pmt::cons(new_message_dict, pmt::PMT_NIL);
						message_port_pub(pmt::mp("tm_frame_out"), new_message);
						
					}
					enter_search();
				}
				break;

			default:
				assert(0);
		} // switch

	}   // while

	return noutput_items;
}

} /* namespace digital */
} /* namespace gr */
