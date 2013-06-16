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
#include "ws_source_c_impl.h"
#include "genericSocketInterface.h"

namespace gr {
  namespace sdrportal {

    ws_source_c::sptr
    ws_source_c::make(bool is_server, int portnum, std::string otw_format, std::string address)
    {
      return gnuradio::get_initial_sptr
        (new ws_source_c_impl(is_server, portnum, otw_format, address));
    }

    /*
     * The private constructor
     */
    ws_source_c_impl::ws_source_c_impl(bool is_server, int portnum, std::string otw_format, std::string address)
      : gr::sync_block("ws_source_c",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {}

    /*
     * Our virtual destructor.
     */
    ws_source_c_impl::~ws_source_c_impl()
    {
    }

    int
    ws_source_c_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        gr_complex *out = (gr_complex *) output_items[0];

        // Do <+signal processing+>

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace sdrportal */
} /* namespace gr */

