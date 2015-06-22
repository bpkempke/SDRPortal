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

#ifndef INCLUDED_CCSDS_TM_TX_H
#define INCLUDED_CCSDS_TM_TX_H

#include <sdrp/api.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/msg_queue.h>

namespace gr {
namespace sdrp {

/*!
 * \brief Turn received messages into a stream
 * \ingroup message_tools_blk
 */
class SDRP_API ccsds_tm_tx : virtual public sync_block{
public:
	// gr::blocks::ccsds_tm_tx::sptr
	typedef boost::shared_ptr<ccsds_tm_tx> sptr;

	static sptr make(unsigned packet_id, unsigned timestamp_id, float out_amp, int num_hist, msg_queue::sptr py_msgq);

	virtual void setConvEn(bool conv_en) = 0;
	virtual void setInterpRatio(float in_ratio) = 0;
	virtual void setCodeRate(unsigned int r_mult, unsigned int r_div) = 0;
	virtual void setFrameLength(int in_frame_len) = 0;
	virtual void setCodingMethod(const std::string in_method) = 0;
	virtual void setCodingParameter(std::string param_name, std::string param_val) = 0;
	virtual void setAccessCode(std::string access_code) = 0;
	virtual void setIdleSequence(std::string idle_sequence) = 0;
	virtual void pauseTX(bool pause) = 0;


	virtual gr::msg_queue::sptr msgq() const = 0;
};

} /* namespace blocks */
} /* namespace gr */

#endif /* INCLUDED_CCSDS_TM_TX_H */
