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

#define M_TWOPI (2.0f*M_PI)

namespace gr {
namespace sdrp {

square_data_tracker_ff::sptr square_data_tracker_ff::make(float loop_bw, float max_freq, float min_freq){
	return gnuradio::get_initial_sptr
		(new square_data_tracker_ff_impl(loop_bw, max_freq, min_freq));
}

square_data_tracker_ff_impl::square_data_tracker_ff_impl(float loop_bw, float max_freq, float min_freq)
	: block("square_data_tracker_ff",
			io_signature::make(1, 1, sizeof(float)),
			io_signature::make(1, 1, sizeof(float))),
	blocks::control_loop(loop_bw, max_freq*M_TWOPI, min_freq*M_TWOPI)
{	
	std::cout << "max_freq = " << max_freq*M_TWOPI << " min_freq = " << min_freq*M_TWOPI << std::endl;
	d_phase_last = 0;
	id_filter_idx = 0;
	filter_updated = false;
	d_manchester_en = false;
	bit_integrator = 0.0;
	for(int ii=0; ii < 4; ii++)
		id_filter[ii] = 0.0;
}

void square_data_tracker_ff_impl::setManchesterEn(bool manchester_en){
	d_manchester_en = manchester_en;
}

void square_data_tracker_ff_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required){
        ninput_items_required[0] = (int)((float)(noutput_items)/d_freq*M_TWOPI); 
}

int square_data_tracker_ff_impl::general_work(int noutput_items,
		gr_vector_int &ninput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const float *in = (float *) input_items[0];
	float *out = (float *) output_items[0];
	int count=0;
	int out_count = 0;
	float error;//Error (in radians) corresponds to how far off the current estimate of phase is observed to be

	while(count < ninput_items[0]){
		float in_bit = (in[count] > 0.0) ? 1.0 : -1.0;
		error = 0.0;

		//Check if we should update the filter (once every bit period)
		id_filter_idx = (int)(d_phase/M_TWOPI*4.0);
		if(id_filter_idx == 1 && filter_updated == false){

			if(d_manchester_en)
				error = (id_filter[2]+id_filter[0])*id_filter[3]*d_freq*d_freq;
			else
				error = -(id_filter[2]-id_filter[0])*id_filter[3]*d_freq*d_freq;

			////The current id filter has our bit, push it out
			//if(d_manchester_en)
			//	out[out_count++] = (id_filter[2]-id_filter[0])*d_freq/M_TWOPI;
			//else
			//	out[out_count++] = (id_filter[3]+id_filter[1])*d_freq/M_TWOPI;

			//printf debugging
			//d_sample_count++;
			//if((d_sample_count % 1001) == 0)
			//	std::cout << "d_freq = " << d_freq << " error = " << error << " id_filter_idx = " << id_filter_idx << " id_filter[0] = " << id_filter[0] << " id_filter[1] = " << id_filter[1] << " id_filter[2] = " << id_filter[2] << " id_filter[3] = " << id_filter[3] << std::endl;

			//Restore temporary zero ID
			id_filter[0] = 0.0;
			id_filter[1] = 0.0;
			id_filter[2] = 0.0;
			id_filter[3] = 0.0;

			filter_updated = true;

		} else if(id_filter_idx != 1){
			filter_updated = false;
		}

		//Bit integrator
		if(id_filter_idx == 0 && integrator_updated == false){
			out[out_count++] = bit_integrator*d_freq/M_TWOPI;
			bit_integrator = 0.0;
			integrator_updated = true;
		} else if(id_filter_idx != 0){
			integrator_updated = false;
		}
		if(d_manchester_en){
			if(id_filter_idx < 2)
				bit_integrator += in_bit;
			else
				bit_integrator -= in_bit;
		} else {
			bit_integrator += in_bit;
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

		//Save phase for next loop's comparison
		d_phase_last = d_phase;
		advance_loop(error);
		//std::cout << "count = " << count << " error = " << error << " d_phase = " << d_phase << " d_freq = " << d_freq << std::endl;
		phase_wrap();
		frequency_limit();

		count++;

	}

	consume_each(count);
	return out_count;
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
	blocks::control_loop::set_frequency(freq*M_TWOPI);
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
