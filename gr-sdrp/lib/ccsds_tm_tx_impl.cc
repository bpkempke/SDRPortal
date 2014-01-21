/* -*- c++ -*- */
/*
 * Copyright 2005,2010,2013 Free Software Foundation, Inc.
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

#include "ccsds_tm_tx_impl.h"
#include "fec/viterbi27_port.h"
#include "fec/encode_rs_ccsds.h"
#include "fec/fixed.h"
#include "fec/fec.h"
#include <gnuradio/io_signature.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <cmath>

#define Pi 3.14159265358979323846
#define INTERP 10000

/********************************
 * Convolutional Encoding Logic *
 ********************************/
#define	POLYA	0x6d
#define	POLYB	0x4f

void
encode(unsigned char *symbols,
       unsigned char *data,
       unsigned int nbits,
       unsigned char &encstate)
{
  while(nbits-- != 0){
    encstate = (encstate << 1) | (*data);
    *symbols++ = parityb(encstate & POLYA);
    *symbols++ = ~parityb(encstate & POLYB);
    data++;
  }
}

namespace gr {
namespace sdrp {

pthread_mutex_t packet_queue_mutex;
pthread_cond_t packet_queue_cv;

ccsds_tm_tx::sptr ccsds_tm_tx::make(float out_amp, int num_hist, msg_queue::sptr py_msgq){
	return gnuradio::get_initial_sptr(new ccsds_tm_tx_impl(out_amp, num_hist, py_msgq));
}

ccsds_tm_tx_impl::ccsds_tm_tx_impl(float out_amp, int num_hist, msg_queue::sptr py_msgq)
	: sync_block("ccsds_tm_tx",
		io_signature::make(0, 0, 0),
		io_signature::make(1, 1, sizeof(gr_complex))), d_msgq(py_msgq){

	//Incoming messages consist of arrays of uint8_t's corresponding to the desired data bytes
	message_port_register_in(pmt::mp("ccsds_tx_msg_in"));
	set_msg_handler(pmt::mp("ccsds_tx_msg_in"), boost::bind(&ccsds_tm_tx_impl::inTXMsg, this, _1));

	//Default to uncoded modulation with frame length of 8920
	d_coding_method = METHOD_NONE;
	d_frame_len = 8920;
	d_out_amp = out_amp;
	d_num_hist = num_hist;
	d_sinc_lookup = NULL;
	d_frac_pos = 0.0;
	d_enc_state = 0;

	//Set up fast parity lookup table
	partab_init();
}

ccsds_tm_tx_impl::~ccsds_tm_tx_impl(){
}

void ccsds_tm_tx_impl::setConvEn(bool conv_en){
	d_conv_en = conv_en;
}

void ccsds_tm_tx_impl::setInterpRatio(float in_ratio){
	d_frac_step = 1.0/in_ratio;

	//Clear out the deque of samples and ready it again
	sample_queue.assign(d_num_hist*2+1,d_out_amp);

	//Calculate sinc function for later use
	if(d_sinc_lookup != NULL)
		delete [] d_sinc_lookup;

	int sinc_size = INTERP*(d_num_hist*2+1);
	d_sinc_lookup = new float[sinc_size];
	for(int ii=0; ii < sinc_size; ii++){
		float cur_position = (float)(ii)/INTERP-d_num_hist;
		d_sinc_lookup[ii] = sin(Pi*cur_position)/(Pi*cur_position);
		if(cur_position == 0.0)
			d_sinc_lookup[ii] = 1.0;
	}
}

void ccsds_tm_tx_impl::inTXMsg(pmt::pmt_t msg){
	//This should be purely a message containing uint8's
	pmt::pmt_t msg_vector = pmt::cdr(msg);
	size_t msg_len = pmt::length(msg_vector);
	size_t offset(0);
	uint8_t *msg_array = (uint8_t*)(pmt::uniform_vector_elements(msg_vector, offset));

	std::cout << "GOT A MESSAGE... msg_len = " << msg_len << std::endl;

	//Make a vector and push it on to the queue of vectors to push out
	std::vector<uint8_t> new_packet;
	new_packet.assign(msg_array, msg_array+msg_len);
	pthread_mutex_lock(&packet_queue_mutex);
	d_packet_queue.push(new_packet);
	pthread_cond_signal(&packet_queue_cv);
	pthread_mutex_unlock(&packet_queue_mutex);
}

void ccsds_tm_tx_impl::setCodeRate(unsigned int r_mult, unsigned int r_div){
	d_r_mult = r_mult;
	d_r_div = r_div;
	//TODO: Should this modify d_fram_len as well???
}

void ccsds_tm_tx_impl::setFrameLength(int in_frame_len){
	d_frame_len = in_frame_len;
}

void ccsds_tm_tx_impl::setCodingMethod(const std::string in_method){
	if(in_method == "CONV"){
		std::cout << "setting coding method to CONV" << std::endl;
		d_coding_method = METHOD_CONV;
	}else if(in_method == "RS"){
		std::cout << "Setting coding method to RS" << std::endl;
		d_coding_method = METHOD_RS;
		d_frame_len = 223*8;
		d_r_mult = 255;
		d_r_div = 223;
	}else if(in_method == "CC")
		d_coding_method = METHOD_CC;
	else if(in_method == "Turbo")
		d_coding_method = METHOD_TURBO;
	else if(in_method == "LDPC")
		d_coding_method = METHOD_LDPC;
	else {
		std::cout << "Setting coding method to None" << std::endl;
		d_coding_method = METHOD_NONE;
	}
}

void ccsds_tm_tx_impl::setCodingParameter(std::string param_name, std::string param_val){
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
}

void ccsds_tm_tx_impl::setIdleSequence(std::string idle_sequence){

	//Convert everything into uint8_t and store into allocated vector
	d_idle_sequence.clear();
	for(int ii=0; ii < idle_sequence.size(); ii++)
		d_idle_sequence.push_back((uint8_t)idle_sequence[ii]);
}

void ccsds_tm_tx_impl::setAccessCode(std::string access_code){

	//Convert everything into uint8_t and store into allocated vector
	d_access_code.clear();
	for(int ii=0; ii < access_code.size(); ii++)
		d_access_code.push_back((uint8_t)access_code[ii]);
}

void ccsds_tm_tx_impl::pushByte(uint8_t in_byte){
//	printf("%02X (", in_byte);
	//Assuming MSB-first
	for(int ii=0; ii < 8; ii++){
		uint8_t cur_bit = (in_byte & (1 << (7-ii))) ? 1 : 0;
//		std::cout << cur_bit+0;
		d_historic_bits.push(cur_bit);
	}
//	printf(")\n");
}

std::vector<uint8_t> ccsds_tm_tx_impl::unpackBits(std::vector<uint8_t> in_packet){
	std::vector<uint8_t> return_bits;
	for(unsigned int ii=0; ii < in_packet.size(); ii++){
		for(int jj=0; jj < 8; jj++){
			uint8_t cur_bit = (in_packet[ii] & (1 << (7-ii))) ? 1 : 0;
			return_bits.push_back(cur_bit);
		}
	}
	return return_bits;
}

int ccsds_tm_tx_impl::work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items){
	gr_complex *out = (gr_complex*)output_items[0];
	gr_complex out_sample;
	int nn = 0;

	while(nn < noutput_items) {
		//Update fractional position no matter what
		d_frac_pos += d_frac_step;
		if(d_frac_pos >= 1.0){
			d_frac_pos--;

			//Pop sample off front of deque
			sample_queue.pop_front();

			//Check to see if there's any packets which need to be enqueued...
			if(d_msg = d_msgq->delete_head_nowait()){
				std::vector<uint8_t> cur_packet(d_msg->msg(),d_msg->msg()+d_msg->length());
				d_packet_queue.push(cur_packet);
				d_print = true;
			}
			if(d_packet_queue.size() > 0){
				std::vector<uint8_t> cur_packet = d_packet_queue.front();
				d_packet_queue.pop();
	
				//Put in logic here to unpack bits and encode in TM format......
				for(unsigned int ii=0; ii < d_access_code.size(); ii++)
					pushByte(d_access_code[ii]);
	
				//Pad cur_packet with X's until it's the size we want
				for(unsigned int ii=cur_packet.size(); ii < d_frame_len/8; ii++)
					cur_packet.push_back((uint8_t)(0x55));
	
				//Encoding varies depending on the coding method used...
				if(d_coding_method == METHOD_NONE){
					//Nothing else needs to be done in the no-encoding case
					for(unsigned int ii=0; ii < cur_packet.size(); ii++)
						pushByte(cur_packet[ii]);
				} else if(d_coding_method == METHOD_CONV){
					//Encode using r=1/2 k=7 CCSDS code
					std::vector<uint8_t> payload_bits = unpackBits(cur_packet);
					std::vector<uint8_t> encoded_bits((payload_bits.size()+6)*2);
					encode_viterbi27_port(&payload_bits[0], payload_bits.size(), &encoded_bits[0]);
					for(unsigned int ii=0; ii < encoded_bits.size(); ii++)
						d_historic_bits.push(encoded_bits[ii]);
					
				} else if(d_coding_method == METHOD_RS){
					//Get the appropriate parity symbols for the data (E=16)
					std::vector<uint8_t> cur_packet_parity(NROOTS);
					encode_rs_ccsds(&cur_packet[0], &cur_packet_parity[0], 0);
					for(unsigned int ii=0; ii < cur_packet.size(); ii++)
						pushByte(cur_packet[ii]);
					for(unsigned int ii=0; ii < NROOTS; ii++)
						pushByte(cur_packet_parity[ii]);
				} else {
					//TODO: Implement the remaining coding methods...
				}

			}

			//Push 'zero' sample to back of deque if it's too small
			if(sample_queue.size() < d_num_hist*2+1){
				for(unsigned int ii=0; ii < d_idle_sequence.size(); ii++){
					pushByte(d_idle_sequence[ii]);
					//std::cout << (int)(d_idle_sequence[ii]);
				}
			}

			//Push all bits from d_historic_bits onto the sample_queue using BPSK...
			while(d_historic_bits.size() > 0){
				//If doing conv encoding, there are two bits output for every data bit
				if(d_conv_en){
    					d_enc_state = (d_enc_state << 1) | d_historic_bits.front();
 					sample_queue.push_back((parityb(d_enc_state & POLYA)) ? d_out_amp : -d_out_amp);
					//sample_queue.push_back(!(parityb(d_enc_state & POLYB)) ? d_out_amp : -d_out_amp);
					sample_queue.push_back((parityb(d_enc_state & POLYB)) ? d_out_amp : -d_out_amp);
				} else
					sample_queue.push_back((d_historic_bits.front()) ? d_out_amp : -d_out_amp);
				d_historic_bits.pop();
			}
		}
		
		//Compute output sample based on neighboring samples
		out[nn] = 0;
		float square_pos = d_frac_pos/d_frac_step;
		square_pos -= (int)square_pos;
		if(square_pos < 0 || square_pos >= 1.0)
			std::cout << "square_pos = " << square_pos << std::endl;
		int cur_sinc_pos = (int)(square_pos*INTERP);
		for(int ii=0; ii < d_num_hist*2+1; ii++){
			out[nn] += sample_queue[d_num_hist*2-ii*d_frac_step+d_frac_pos]*d_sinc_lookup[cur_sinc_pos];
			cur_sinc_pos += INTERP;
		}
//		if(d_print)
//			std::cout << out[nn] << std::endl;

		//Increment sample position and keep going...
		nn++;
	}


	return nn;
}

} /* namespace sdrp */
} /* namespace gr */
