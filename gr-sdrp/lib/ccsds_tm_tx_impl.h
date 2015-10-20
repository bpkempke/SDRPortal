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
#include <deque>

namespace gr {
namespace sdrp {

class ccsds_tm_tx_impl : public ccsds_tm_tx {
private:
	unsigned d_packet_id, d_timestamp_id;

	size_t	 	d_itemsize;
	unsigned		d_msg_offset;
	bool		d_print;
	bool              d_tags;
	bool d_pause;
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
	std::vector<uint8_t> d_idle_sequence;
	
	float d_out_amp, d_frac_step;
	int d_num_hist;
	float *d_sinc_lookup;
	float d_frac_pos;
	msg_queue::sptr d_msgq;
	message::sptr d_msg;
	std::deque<float> sample_queue;
	unsigned char d_enc_state;
	bool d_conv_en;

public:
	ccsds_tm_tx_impl(unsigned packet_id, unsigned timestamp_id, float out_amp, int num_hist, msg_queue::sptr py_msgq);

	~ccsds_tm_tx_impl();

	msg_queue::sptr msgq() const { return d_msgq; }

	virtual void setConvEn(bool conv_en);
	virtual void setInterpRatio(float in_ratio);
	virtual void setCodeRate(unsigned int r_mult, unsigned int r_div);
	virtual void setFrameLength(int in_frame_len);
	virtual void setCodingMethod(const std::string in_method);
	virtual void setCodingParameter(std::string param_name, std::string param_val);
	virtual void setAccessCode(std::string access_code);
	virtual void setIdleSequence(std::string idle_sequence);
	virtual void pauseTX(bool pause);

	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);
};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_CCSDS_TM_TX_IMPL_H */
