/* -*- c++ -*- */
/*
 * Copyright 2005,2006,2012 Free Software Foundation, Inc.
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

#ifndef INCLUDED_SDRP_CCSDS_TM_FRAMER_IMPL_H
#define INCLUDED_SDRP_CCSDS_TM_FRAMER_IMPL_H

#include <sdrp/ccsds_tm_framer.h>

namespace gr {
  namespace sdrp {

    class ccsds_tm_framer_impl : public ccsds_tm_framer
    {
    private:
      void performHardDecisions(std::vector<uint8_t> &packet_data);
      enum state_t {STATE_SYNC_SEARCH, STATE_HAVE_SYNC, STATE_HAVE_HEADER};
      enum coding_method_t {METHOD_CONV, METHOD_RS, METHOD_CC, METHOD_TURBO, METHOD_LDPC, METHOD_NONE};

      static const int MAX_PKT_LEN    = 4096;
      static const int HEADERBITLEN   = 32;

      msg_queue::sptr  d_target_queue;	    // where to send the packet when received
      state_t          d_state;
      unsigned int     d_header;	      // header bits
      int	       d_headerbitlen_cnt;    // how many so far

      unsigned char    d_packet[MAX_PKT_LEN]; // assembled payload
      unsigned char    d_packet_byte;	      // byte being assembled
      int	       d_packet_byte_index;   // which bit of d_packet_byte we're working on
      int	       d_packetlen;		   // length of packet
      int              d_packet_whitener_offset;   // offset into whitener string to use
      int	       d_packetlen_cnt;	           // how many so far

      int d_frame_len;
      int d_frames_correct, d_frames_incorrect;
      coding_method_t  d_coding_method;
      unsigned int d_r_mult, d_r_div;
      unsigned int d_rs_e, d_rs_i, d_rs_q;
      unsigned int d_turbo_k;
      unsigned int d_ldpc_k;
      unsigned int d_tot_bits;
      pmt::pmt_t d_correlate_key;
      unsigned int d_packet_id;
      std::vector<float> d_symbol_hist;
      unsigned long d_num_times;

      void *d_vp;

    protected:
      void enter_search();
      void enter_have_sync();
      void enter_have_header(int payload_len, int whitener_offset);

      bool header_ok()
      {
	// confirm that two copies of header info are identical
	return ((d_header >> 16) ^ (d_header & 0xffff)) == 0;
      }

      void header_payload(int *len, int *offset)
      {
	// header consists of two 16-bit shorts in network byte order
	// payload length is lower 12 bits
	// whitener offset is upper 4 bits
	*len = (d_header >> 16) & 0x0fff;
	*offset = (d_header >> 28) & 0x000f;
      }

      void resetDecoder();

    public:
      ccsds_tm_framer_impl(unsigned packet_id, const std::string &tag_name);
      ~ccsds_tm_framer_impl();

      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);

      virtual void setFrameLength(unsigned int num_bits);
      virtual void setCodeRate(unsigned int r_mult, unsigned int r_div);
      virtual void setCodingMethod(std::string in_method);
      virtual void setCodingParameter(std::string param_name, std::string param_val);
    };

  } /* namespace digital */
} /* namespace gr */

#endif /* INCLUDED_GR_FRAMER_SINK_1_IMPL_H */
