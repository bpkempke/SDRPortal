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

#ifndef INCLUDED_SDRP_SWEEP_GENERATOR_CC_IMPL_H
#define INCLUDED_SDRP_SWEEP_GENERATOR_CC_IMPL_H

#include <sdrp/sweep_generator_cc.h>

namespace gr {
namespace sdrp {

class sweep_generator_cc_impl : public sweep_generator_cc
{
private:
	double d_time_step;
	double d_cur_time;
	double d_freq;
	double d_phase;
	int d_cur_profile_idx;
	std::vector<double> d_profile_times, d_profile_freqs;

protected:

public:
	sweep_generator_cc_impl(double samples_per_second);
	~sweep_generator_cc_impl(){};

	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);

	virtual void setProfile(std::vector<double> profile_times, std::vector<double> profile_freqs);
	virtual void sweep();
};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_SWEEP_GENERATOR_CC_IMPL_H */
