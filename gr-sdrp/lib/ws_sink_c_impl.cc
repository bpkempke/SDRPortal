/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ws_sink_c_impl.h"
#include <gnuradio/io_signature.h>
#include <string>
#include <iostream>

namespace gr {
  namespace sdrp {

    ws_sink_c::sptr
    ws_sink_c::make(bool is_server, int portnum, std::string otw_format, std::string address)
    {
      return gnuradio::get_initial_sptr
        (new ws_sink_c_impl(is_server, portnum, otw_format, address));
    }

    /*
     * The private constructor
     */
    ws_sink_c_impl::ws_sink_c_impl(bool is_server, int portnum, std::string otw_format, std::string address)
      : sync_block("ws_sink_c",
			io_signature::make(0, 1, sizeof(gr_complex)),
		      io_signature::make(0, 0, 0)), stream_conv(STREAM_FLOAT)
    {
    	if(is_server){
		sock_int = new genericSocketInterface(SOCKET_WS_BINARY, portnum);
		sock_int->addUpperLevel(this);
		this->addLowerLevel(sock_int);
	}else{
		//TODO: Add in functionality for WS client socket
	}

	if(otw_format == "DOUBLE")
		result_type = STREAM_DOUBLE;
	else if(otw_format == "FLOAT")
		result_type = STREAM_FLOAT;
	else if(otw_format == "INT32")
		result_type = STREAM_INT32_T;
	else if(otw_format == "INT16")
		result_type = STREAM_INT16_T;
	else if(otw_format == "INT8")
		result_type = STREAM_INT8_T;

	//Message ports to/from socket interface
	message_port_register_in(pmt::mp("ws_pdu_in"));
	set_msg_handler(pmt::mp("ws_pdu_in"), boost::bind(&ws_sink_c_impl::inPacketMsg, this, _1));
}

/*
 * Our virtual destructor.
 */
ws_sink_c_impl::~ws_sink_c_impl()
{
}

void ws_sink_c_impl::inPacketMsg(pmt::pmt_t msg){
	//This should be a message containing a variety of key-value dictionary pairs
	uint8_t *buf;
	uint32_t total_buf_length = 4;
	
	// grab the components of the pdu message
	pmt::pmt_t meta(pmt::car(msg)); // make sure this is NIL || Dict ?
	pmt::pmt_t vect(pmt::cdr(msg)); // make sure this is a vector?

	//We are only interested in the metadata
	//  There should be nothing contained in the cdr
	if(!pmt::eq(meta, pmt::PMT_NIL)) {
		pmt::pmt_t keys = pmt::dict_keys(meta);
		pmt::pmt_t vals = pmt::dict_values(meta);
		size_t nitems = pmt::length(keys);
		for(size_t i = 0; i < nitems; i++) {
			total_buf_length += 8;
			total_buf_length += pmt::length(pmt::nth(i, vals));
		}
	}

	//Allocate the buffer now that we know how long it's going to be
	buf = new uint8_t[total_buf_length];

	//Now loop through everything again, populating the 
	memcpy(&buf[0], &total_buf_length, 4);
	uint32_t cur_buf_idx = 4;
	if(!pmt::eq(meta, pmt::PMT_NIL)) {
		pmt::pmt_t keys = pmt::dict_keys(meta);
		pmt::pmt_t vals = pmt::dict_values(meta);
		size_t nitems = pmt::length(keys);
		for(size_t i = 0; i < nitems; i++) {
			uint32_t this_id = (uint32_t)(pmt::to_long(pmt::nth(i, keys)));
			uint32_t this_length = (uint32_t)(pmt::length(pmt::nth(i, vals)));
			memcpy(&buf[cur_buf_idx], &this_id, 4);
			memcpy(&buf[cur_buf_idx+4], &this_length, 4);
			size_t offset = 0;
			memcpy(&buf[cur_buf_idx+8], pmt::uniform_vector_elements(pmt::nth(i, vals),offset), pmt::length(pmt::nth(i, vals)));
			cur_buf_idx += 8 + this_length;
		}
	}

	//Send everything out to the socket
	messageType resulting_message;
	resulting_message.buffer = (char*)buf;
	resulting_message.num_bytes = total_buf_length;
	resulting_message.socket_channel = -1;
	dataToLowerLevel(&resulting_message, 1);
}


int ws_sink_c_impl::work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items)
{
	const float *in = (const float *) input_items[0];

	//Convert everything to the requested result type
	stream_conv.convertFromCommon((float*)in, noutput_items*sizeof(gr_complex), result_type, noutput_items);

	//	std::cerr << "noutput_items = " << noutput_items << " in[1000] = " << in[1000] << std::endl;

	//Send everything out to the socket
	messageType resulting_message;
	resulting_message.buffer = (char*)stream_conv.getResultFromStreamType(STREAM_FLOAT);
	resulting_message.num_bytes = noutput_items*sizeof(float)*2;
	resulting_message.socket_channel = -1;
	dataToLowerLevel(&resulting_message, 1);

	// Tell runtime system how many output items we produced.
	return noutput_items;

}

} /* namespace sdrp */
} /* namespace gr */

