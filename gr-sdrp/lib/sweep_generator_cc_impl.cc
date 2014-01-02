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

#include "sweep_generator_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <math.h>
#include <gnuradio/math.h>

#define M_TWOPI 2.0f*M_PI

namespace gr {
namespace sdrp {

sweep_generator_cc::sptr sweep_generator_cc::make(double samples_per_second){
	return gnuradio::get_initial_sptr
		(new sweep_generator_cc_impl(samples_per_second));
}

sweep_generator_cc_impl::sweep_generator_cc_impl(double samples_per_second)
	: sync_block("sweep_generator_cc",
			io_signature::make(1, 1, sizeof(gr_complex)),
			io_signature::make(1, 1, sizeof(gr_complex))),
	d_cur_profile_idx(0),
	d_freq(0.0),
	d_phase(0.0),
	d_time_step(1.0/samples_per_second)
{
	//TODO: Anything needed here?
	d_profile_times.clear();
	d_profile_freqs.clear();
}

void sweep_generator_cc_impl::setProfile(std::vector<double> profile_times, std::vector<double> profile_freqs){
	//Convert times to integer times to disallow loss-of-precision with doubles
	d_profile_times.clear();
	for(unsigned int ii=0; ii < profile_times.size(); ii++){
		d_profile_times.push_back(profile_times[ii] / d_time_step);
	}

	//Convert frequencies to radians per sample
	d_profile_freqs.clear();
	for(unsigned int ii=0; ii < profile_freqs.size(); ii++){
		d_profile_freqs.push_back(profile_freqs[ii] * d_time_step * M_TWOPI);
	}
}

void sweep_generator_cc_impl::sweep(){
	//Just need to set the sweep profile index to the beginning to restart
	d_cur_profile_idx = 0;
	d_cur_time = 0.0;
}

int sweep_generator_cc_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const gr_complex *in = (gr_complex *) input_items[0];
	gr_complex *out = (gr_complex *) output_items[0];
	int count=0;
	gr_complex nco_out;

	while(count < noutput_items){
		//If we're in the middle of a frequency sweep, calculate current frequency
		if(d_cur_profile_idx < (int)(d_profile_times.size())-1){
			//Check to see if we've transitioned to a new slope
			if(d_cur_time > d_profile_times[d_cur_profile_idx+1]){
				std::cout << "switching legs of sweep..." << std::endl;
				d_cur_profile_idx++;
				d_freq = 0.0;
				continue;
			}

			//Otherwise figure out where we are within the slope
			d_freq = ((d_cur_time-d_profile_times[d_cur_profile_idx])*d_profile_freqs[d_cur_profile_idx+1] + 
			          (d_profile_times[d_cur_profile_idx+1]-d_cur_time)*d_profile_freqs[d_cur_profile_idx]) / 
			         (d_profile_times[d_cur_profile_idx+1]-d_profile_times[d_cur_profile_idx]);
			
			d_cur_time += 1.0;
		}

		//Increment phase based on calculated current frequency
		d_phase += d_freq;
		if(d_phase > M_TWOPI)
			d_phase -= M_TWOPI;
		else if(d_phase < 0)
			d_phase += M_TWOPI;


		nco_out = gr_expj(d_phase);
		out[count] = in[count] * nco_out;
		

		count++;
	}
	return noutput_items;
}

}
}
