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

#include <gnuradio/io_signature.h>
#include <string>
#include "ws_sink_c_impl.h"

namespace gr {
namespace sdrportal {

ws_sink_c::sptr	ws_sink_c::make(bool is_server, int portnum, std::string otw_format, std::string address) {
		return gnuradio::get_initial_sptr
			(new ws_sink_c_impl(is_server, portnum, otw_format, address));
}

/*
 * The private constructor
 */
ws_sink_c_impl::ws_sink_c_impl(bool is_server, int portnum, std::string otw_format, std::string address)
	: gr::sync_block("ws_sink_c",
			gr::io_signature::make(1, 1, sizeof(gr_complex)),
			gr::io_signature::make(0, 0, 0)), hierarchicalDataflowBlock(1,1){
	if(is_server){
		sock_int = new genericSocketInterface(SOCKET_WS, portnum);
		sock_int->addUpperLevel(this);
		this->addLowerLevel(sock_int);
	}else{
		//TODO: Add in functionality for WS client socket
	}

	if(otw_format == "DOUBLE")
		result_type = DOUBLE;
	else if(otw_format == "FLOAT")
		result_type = FLOAT;
	else if(otw_format == "INT32")
		result_type = INT32;
	else if(otw_format == "INT16")
		result_type = INT16;
	else if(otw_format == "INT8")
		result_type = INT8;
}

/*
 * Our virtual destructor.
 */
ws_sink_c_impl::~ws_sink_c_impl(){
}
	
int ws_sink_c_impl::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items){

	const float *in = (const float *) input_items[0];

	//Convert everything to the requested result type
	stream_conv.convertTo((float*)in, noutput_items*2, result_type);

	//Send everything out to the socket
	messageType resulting_message;
	resulting_message.buffer = (char*)stream_conv.getResult();
	resulting_message.num_bytes = noutput_items*sizeof(float)*2;
	dataToLowerLevel(stream_conv.getResult(), 1);

	// Tell runtime system how many output items we produced.
	return noutput_items;
}

} /* namespace sdrportal */
} /* namespace gr */

