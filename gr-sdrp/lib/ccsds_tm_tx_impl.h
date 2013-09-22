/* -*- c++ -*- */
/*
 * Copyright 2005,2013 Free Software Foundation, Inc.
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

#ifndef INCLUDED_CCSDS_TM_TX_IMPL_H
#define INCLUDED_CCSDS_TM_TX_IMPL_H

#include <sdrp/ccsds_tm_tx.h>
#include <gnuradio/message.h>
#include <queue>
#include <vector>

namespace gr {
namespace sdrp {

class ccsds_tm_tx_impl : public ccsds_tm_tx {
private:
	size_t	 	d_itemsize;
	msg_queue::sptr	d_msgq;
	message::sptr	d_msg;
	unsigned		d_msg_offset;
	bool		d_eof;
	bool              d_tags;
	// FIXME: Is this adequate tagname length.
	std::string       d_lengthtagname;
	enum coding_method_t {METHOD_CONV, METHOD_RS, METHOD_CC, METHOD_TURBO, METHOD_LDPC, METHOD_NONE};
	std::queue<std::vector<uint8_t> > d_packet_queue;
	std::queue<uint8_t> d_historic_bits;
	void inTXMsg(pmt::pmt_t msg);
	void pushByte(uint8_t in_byte);
	std::vector<uint8_t> unpackBits(std::vector<uint8_t> in_packet);
	
	int d_frame_len;
	coding_method_t  d_coding_method;
	unsigned int d_r_mult, d_r_div;
	unsigned int d_rs_e, d_rs_i, d_rs_q;
	unsigned int d_turbo_k;
	unsigned int d_ldpc_k;
	std::vector<uint8_t> d_access_code;


public:
	ccsds_tm_tx_impl();

	~ccsds_tm_tx_impl();

	msg_queue::sptr msgq() const { return d_msgq; }

	virtual void setCodeRate(unsigned int r_mult, unsigned int r_div);
	virtual void setFrameLength(int in_frame_len);
	virtual void setCodingMethod(const std::string in_method);
	virtual void setCodingParameter(std::string param_name, std::string param_val);
	virtual void setAccessCode(std::string access_code);

	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);
};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_CCSDS_TM_TX_IMPL_H */
