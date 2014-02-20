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

#include "square_sub_tracker_ff_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <math.h>
#include <gnuradio/math.h>

#define M_TWOPI 2.0f*M_PI

namespace gr {
namespace sdrp {

square_sub_tracker_ff::sptr square_sub_tracker_ff::make(float loop_bw, float max_freq, float min_freq){
	return gnuradio::get_initial_sptr
		(new square_sub_tracker_ff_impl(loop_bw, max_freq, min_freq));
}

square_sub_tracker_ff_impl::square_sub_tracker_ff_impl(float loop_bw, float max_freq, float min_freq)
	: sync_block("square_sub_tracker_ff",
			io_signature::make(1, 1, sizeof(float)),
			io_signature::make(1, 1, sizeof(float))),
	blocks::control_loop(loop_bw, max_freq*M_TWOPI, min_freq*M_TWOPI)
{	
	d_cur_bit = 1.0;
	d_phase_last = 0;
	id_filter_idx = 0;
	for(int ii=0; ii < 4; ii++)
		id_filter[ii] = 0.0;
}

int square_sub_tracker_ff_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const float *in = (float *) input_items[0];
	float *out = (float *) output_items[0];
	int count=0;

	while(count < noutput_items){
		float in_bit = (in[count] > 0.0) ? 1.0 : -1.0;
		d_error = 0.0;

		//for(int ii=0; ii < 4; ii++){
		//	if(id_filter[ii] > 20)
		//		std::cout << "id_filter[" << ii << "] = " << id_filter[ii] << std::endl;
		//}

		if((d_phase <= M_PI/2 && d_phase_last > 3*M_PI/2) || (d_phase >= M_PI && d_phase_last < M_PI)){

			//Check if we should update the filter (once every bit period)
			if(!(id_filter_idx & 1)){
				int id_idx_first = (id_filter_idx == 0) ? 2 : 0;
				int id_idx_second = (id_filter_idx == 0) ? 3 : 1;

				d_error = id_filter[id_idx_second]*d_freq/100;
				if(id_filter[id_idx_first] > 0)
					d_error = -d_error;

				//printf debugging
				d_sample_count++;
				if((d_sample_count % 10001) == 0){
					std::cout << "d_phase = " << d_phase << " d_phase_last = " << d_phase_last << " d_freq = " << d_freq << " d_error = " << d_error << " id_filter_idx = " << id_filter_idx << " id_idx_second = " << id_idx_second << " id_filter[id_idx_second] = " << id_filter[id_idx_second] << std::endl;
					//for(int ii=0; ii < 40; ii++){
					//	std::cout << in[ii] << std::endl;
					//}
				}

				id_filter[id_idx_first] = 0.0;
				id_filter[id_idx_second] = 0.0;
			}

			id_filter_idx = (id_filter_idx == 3) ? 0 : id_filter_idx+1;
		}

		//Increment integrate and dump filters depending on current position
		if(id_filter_idx == 0){
			id_filter[0] += in_bit;
			id_filter[3] += in_bit;
		} else if(id_filter_idx == 1){
			id_filter[0] += in_bit;
			id_filter[1] += in_bit;
		} else if(id_filter_idx == 2){
			id_filter[2] += in_bit;
			id_filter[1] += in_bit;
		} else {
			id_filter[2] += in_bit;
			id_filter[3] += in_bit;
		}

		//The current id filter has our bit, push it out
		out[count] = in[count] * ((id_filter_idx & 2) ? 1.0 : -1.0);

		//Save phase for next loop's comparison
		d_phase_last = d_phase;
		advance_loop(d_error);
		phase_wrap();
		frequency_limit();

		count++;
	}
	return noutput_items;
}

void square_sub_tracker_ff_impl::set_loop_bandwidth(float bw) {
	blocks::control_loop::set_loop_bandwidth(bw);
}

void square_sub_tracker_ff_impl::set_damping_factor(float df) {
	blocks::control_loop::set_damping_factor(df);
}

void square_sub_tracker_ff_impl::set_alpha(float alpha) {
	blocks::control_loop::set_alpha(alpha);
}

void square_sub_tracker_ff_impl::set_beta(float beta) {
	blocks::control_loop::set_beta(beta);
}

void square_sub_tracker_ff_impl::set_frequency(float freq) {
	blocks::control_loop::set_frequency(freq*M_TWOPI);
}

void square_sub_tracker_ff_impl::set_phase(float phase) {
	blocks::control_loop::set_phase(phase);
}

void square_sub_tracker_ff_impl::set_min_freq(float freq) {
	blocks::control_loop::set_min_freq(freq);
}

void square_sub_tracker_ff_impl::set_max_freq(float freq) {
	blocks::control_loop::set_max_freq(freq);
}


float square_sub_tracker_ff_impl::get_loop_bandwidth() const {
	return blocks::control_loop::get_loop_bandwidth();
}

float square_sub_tracker_ff_impl::get_damping_factor() const {
	return blocks::control_loop::get_damping_factor();
}

float square_sub_tracker_ff_impl::get_alpha() const {
	return blocks::control_loop::get_alpha();
}

float square_sub_tracker_ff_impl::get_beta() const {
	return blocks::control_loop::get_beta();
}

float square_sub_tracker_ff_impl::get_frequency() const {
	return blocks::control_loop::get_frequency();
}

float square_sub_tracker_ff_impl::get_phase() const {
	return blocks::control_loop::get_phase();
}

float square_sub_tracker_ff_impl::get_min_freq() const {
	return blocks::control_loop::get_min_freq();
}

float square_sub_tracker_ff_impl::get_max_freq() const {
	return blocks::control_loop::get_max_freq();
}


}
}
