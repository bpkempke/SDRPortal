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

square_data_tracker_ff::sptr square_data_tracker_ff::make(float loop_bw, float max_freq, float min_freq){
	return gnuradio::get_initial_sptr
		(new square_data_tracker_ff_impl(loop_bw, max_freq, min_freq));
}

square_data_tracker_ff_impl::square_data_tracker_ff_impl(float loop_bw, float max_freq, float min_freq)
	: sync_block("square_data_tracker_ff",
			io_signature::make(1, 1, sizeof(float)),
			io_signature::make(1, 1, sizeof(float))),
	blocks::control_loop(loop_bw, max_freq, min_freq)
{	
}

int square_data_tracker_ff_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const float *in = (float *) input_items[0];
	float *out = (float *) output_items[0];
	int count=0;
	float error;

	while(count < noutput_items){
		//TODO: Logic goes here!
		//error = ???;

		advance_loop(error);
		phase_wrap();
		frequency_limit();

		count++;
	}
	return noutput_items;
}

void square_data_tracker_ff_impl::set_loop_bandwidth(float bw) {
	blocks::control_loop::set_loop_bandwidth(bw);
}

void square_data_tracker_ff_impl::set_damping_factor(float df) {
	blocks::control_loop::set_damping_factor(df);
}

void square_data_tracker_ff_impl::set_alpha(float alpha) {
	blocks::control_loop::set_alpha(alpha);
}

void square_data_tracker_ff_impl::set_beta(float beta) {
	blocks::control_loop::set_beta(beta);
}

void square_data_tracker_ff_impl::set_frequency(float freq) {
	blocks::control_loop::set_frequency(freq);
}

void square_data_tracker_ff_impl::set_phase(float phase) {
	blocks::control_loop::set_phase(phase);
}

void square_data_tracker_ff_impl::set_min_freq(float freq) {
	blocks::control_loop::set_min_freq(freq);
}

void square_data_tracker_ff_impl::set_max_freq(float freq) {
	blocks::control_loop::set_max_freq(freq);
}


float square_data_tracker_ff_impl::get_loop_bandwidth() const {
	return blocks::control_loop::get_loop_bandwidth();
}

float square_data_tracker_ff_impl::get_damping_factor() const {
	return blocks::control_loop::get_damping_factor();
}

float square_data_tracker_ff_impl::get_alpha() const {
	return blocks::control_loop::get_alpha();
}

float square_data_tracker_ff_impl::get_beta() const {
	return blocks::control_loop::get_beta();
}

float square_data_tracker_ff_impl::get_frequency() const {
	return blocks::control_loop::get_frequency();
}

float square_data_tracker_ff_impl::get_phase() const {
	return blocks::control_loop::get_phase();
}

float square_data_tracker_ff_impl::get_min_freq() const {
	return blocks::control_loop::get_min_freq();
}

float square_data_tracker_ff_impl::get_max_freq() const {
	return blocks::control_loop::get_max_freq();
}


}
}
