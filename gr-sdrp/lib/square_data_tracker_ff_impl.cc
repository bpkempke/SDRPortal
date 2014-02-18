/* -*- c++ -*- */
/*
 * Copyright 2004,2006,2010,2012 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "square_data_tracker_ff_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <math.h>
#include <gnuradio/math.h>

#define M_TWOPI 2.0f*M_PI

namespace gr {
namespace sdrp {

square_data_tracker_ff::sptr square_data_tracker_ff::make(double freq, double phase_loop_bw, double freq_loop_bw){
	return gnuradio::get_initial_sptr
		(new square_data_tracker_ff_impl(freq, phase_loop_bw, freq_loop_bw));
}

square_data_tracker_ff_impl::square_data_tracker_ff_impl(double freq, double phase_loop_bw, double freq_loop_bw)
	: sync_block("square_data_tracker_ff",
			io_signature::make(1, 1, sizeof(float)),
			io_signature::make(1, 1, sizeof(float)))
{	
	setFreq(freq);
	setPhaseLoopBW(phase_loop_bw);
	setFreqLoopBW(freq_loop_bw);
}

void square_data_tracker_ff_impl::setFreq(double freq){
	d_freq = freq;
	d_phase = 0.0;
}

void square_data_tracker_ff_impl::setPhaseLoopBW(double loop_bw){
	d_phase_loop_bw = loop_bw;
	d_phase = 0.0;
}

void square_data_tracker_ff_impl::setFreqLoopBW(double loop_bw){
	d_freq_loop_bw = loop_bw;
	d_phase = 0.0;
}

int square_data_tracker_ff_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const float *in = (float *) input_items[0];
	float *out = (float *) output_items[0];
	int count=0;

	while(count < noutput_items){
		//TODO: Logic goes here!

		count++;
	}
	return noutput_items;
}

}
}
