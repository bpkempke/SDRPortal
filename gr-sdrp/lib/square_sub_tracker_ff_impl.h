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

#ifndef INCLUDED_SDRP_SQUARE_SUB_TRACKER_FF_IMPL_H
#define INCLUDED_SDRP_SQUARE_SUB_TRACKER_FF_IMPL_H

#include <sdrp/square_sub_tracker_ff.h>

namespace gr {
namespace sdrp {

class square_sub_tracker_ff_impl : public square_sub_tracker_ff
{
private:
	double d_freq;
	double d_phase;
	double d_freq_loop_bw;
	double d_phase_loop_bw;

protected:

public:
	square_sub_tracker_ff_impl(double freq, double phase_loop_bw, double freq_loop_bw);
	~square_sub_tracker_ff_impl(){};

	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);

	virtual void setFreq(double freq);
	virtual void setPhaseLoopBW(double loop_bw);
	virtual void setFreqLoopBW(double loop_bw);
};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_SQUARE_SUB_TRACKER_FF_IMPL_H */
