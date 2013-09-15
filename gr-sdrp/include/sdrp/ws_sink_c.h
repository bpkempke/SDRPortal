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


#ifndef INCLUDED_SDRP_WS_SINK_C_H
#define INCLUDED_SDRP_WS_SINK_C_H

#include <sdrp/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace sdrp {

    /*!
     * \brief <+description of block+>
     * \ingroup sdrp
     *
     */
    class SDRP_API ws_sink_c : virtual public sync_block
    {
    public:
       typedef boost::shared_ptr<ws_sink_c> sptr;

       /*!
        * \brief Return a shared_ptr to a new instance of sdrp::ws_sink_c.
        *
        * To avoid accidental use of raw pointers, sdrp::ws_sink_c's
        * constructor is in a private implementation
        * class. sdrp::ws_sink_c::make is the public interface for
        * creating new instances.
        */
       static sptr make(bool is_server, int portnum, std::string otw_format, std::string address="");
    };

  } // namespace sdrp
} // namespace gr

#endif /* INCLUDED_SDRP_WS_SINK_C_H */

