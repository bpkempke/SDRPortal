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

ccsds_tm_framer::sptr ccsds_tm_framer::make(msg_queue::sptr target_queue){
	return gnuradio::get_initial_sptr
		(new ccsds_tm_framer_impl(target_queue));
}

ccsds_tm_framer_impl::ccsds_tm_framer_impl(msg_queue::sptr target_queue)
	: sync_block("ccsds_tm_framer",
			io_signature::make(1, 1, sizeof(unsigned char)),
			io_signature::make(0, 0, 0)),
	d_target_queue(target_queue)
{
	enter_search();
}

ccsds_tm_framer_impl::~ccsds_tm_framer_impl(){
}

int ccsds_tm_framer_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const unsigned char *in = (const unsigned char *) input_items[0];
	int count=0;

	while(count < noutput_items){
		switch(d_state) {

			case STATE_SYNC_SEARCH:    // Look for flag indicating beginning of pkt
				while(count < noutput_items) {
					if(in[count] & 0x2){  // Found it, set up for header decode
						enter_have_sync();
						break;
					}
					count++;
				}
				break;

			case STATE_HAVE_SYNC:
				while(count < noutput_items) {	// Shift bits one at a time into header
					d_header = (d_header << 1) | (in[count++] & 0x1);
					if(++d_headerbitlen_cnt == HEADERBITLEN) {

						// we have a full header, check to see if it has been received properly
						if(header_ok()) {
							int payload_len;
							int whitener_offset;
							header_payload(&payload_len, &whitener_offset);
							enter_have_header(payload_len, whitener_offset);

							if(d_packetlen == 0) {	    // check for zero-length payload
								// build a zero-length message
								// NOTE: passing header field as arg1 is not scalable
								message::sptr msg =
									message::make(0, d_packet_whitener_offset, 0, 0);

								d_target_queue->insert_tail(msg);		// send it
								msg.reset();  				// free it up

								enter_search();
							}
						}
						else
							enter_search();				// bad header
						break;					// we're in a new state
					}
				}
				break;

			case STATE_HAVE_HEADER:
				while(count < noutput_items) {   // shift bits into bytes of packet one at a time
					d_packet_byte = (d_packet_byte << 1) | (in[count++] & 0x1);
					if(d_packet_byte_index++ == 7) {	  	// byte is full so move to next byte
						d_packet[d_packetlen_cnt++] = d_packet_byte;
						d_packet_byte_index = 0;

						if(d_packetlen_cnt == d_packetlen) {	// packet is filled
							// build a message
							// NOTE: passing header field as arg1 is not scalable
							message::sptr msg =
								message::make(0, d_packet_whitener_offset, 0, d_packetlen_cnt);
							memcpy(msg->msg(), d_packet, d_packetlen_cnt);

							d_target_queue->insert_tail(msg);	// send it
							msg.reset();  				// free it up

							enter_search();
							break;
						}
					}
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
