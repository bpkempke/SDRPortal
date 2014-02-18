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

#ifndef INCLUDED_SDRP_SQUARE_DATA_TRACKER_FF_H
#define INCLUDED_SDRP_SQUARE_DATA_TRACKER_FF_H

#include <sdrp/api.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/msg_queue.h>

namespace gr {
namespace sdrp {

class SDRP_API square_data_tracker_ff : virtual public gr::sync_block {
public:
	typedef boost::shared_ptr<square_data_tracker_ff> sptr;

	static sptr make(double freq, double phase_loop_bw, double freq_loop_bw);

	virtual void setFreq(double freq) = 0;
	virtual void setPhaseLoopBW(double loop_bw) = 0;
	virtual void setFreqLoopBW(double loop_bw) = 0;
};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_SQUARE_DATA_TRACKER_FF_H */