/* -*- c++ -*- */
/*
 * Copyright 2004,2010,2011 Free Software Foundation, Inc.
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

#include "dsn_pn_rx_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <math.h>
#include <gnuradio/math.h>

namespace gr {
namespace sdrp {

#ifndef M_TWOPI
#define M_TWOPI (2.0f*M_PI)
#endif

dsn_pn_rx::sptr dsn_pn_rx::make(double samples_per_second, float loop_bw, float max_freq, float min_freq) {
	return gnuradio::get_initial_sptr
		(new dsn_pn_rx_impl(samples_per_second, loop_bw, max_freq, min_freq));
}

static int ios[] = {sizeof(gr_complex), sizeof(float)};
static std::vector<int> iosig(ios, ios+sizeof(ios)/sizeof(int));
dsn_pn_rx_impl::dsn_pn_rx_impl(double samples_per_second, float loop_bw, float max_freq, float min_freq)
	: sync_block("dsn_pn_rx",
		io_signature::make(1, 1, sizeof(gr_complex)),
		io_signature::makev(1, 2, iosig)),
	blocks::control_loop(loop_bw, max_freq, min_freq),
	d_cal_time_count(0),
	d_cal_time_seconds(0),
	d_cal_time_frac(0.0),
	d_samples_per_second(samples_per_second)
{
	d_composite_queue.clear();
	d_cur_composite.done = true;
}

dsn_pn_rx_impl::~dsn_pn_rx_impl() {

}

void dsn_pn_tx_impl::queueRanging(std::string combination_method, uint64_t rx_time, double T, std::vector<std::vector<bool> > components, double range_freq, bool range_is_square){
	//First switch combination_method to lowercase for later ease
	std::locale loc;
	for(int ii=0; ii < combination_method.size(); ii++)
		combination_method[ii] = std::tolower(combination_method[ii], loc);

	//NOTE: combination_method is expected to be 'and', 'or', 'xor', or 'vote'
	PNComposite new_composite;
	new_composite.cm = (combination_method == "and") ? CM_AND :
	                  (combination_method == "or") ? CM_OR :
	                  (combination_method == "xor") ? CM_XOR : CM_VOTE;
	new_composite.rx_time = rx_time;
	new_composite.T = T;
	new_composite.components = components;
	new_composite.range_freq = range_freq;
	new_composite.range_is_square = range_is_square;
	new_composite.done = false;
	new_composite.running = false;

	//Push new composite onto queue and sort
	d_composite_queue.push_back(new_composite);
	d_composite_queue.sort(compare_composite_start);
}

float dsn_pn_rx_impl::mod_2pi(float in) {
	if(in > M_PI)
		return in - M_TWOPI;
	else if(in < -M_PI)
		return in + M_TWOPI;
	else
		return in;
}

float dsn_pn_rx_impl::phase_detector(gr_complex sample, float ref_phase) {
	float sample_phase;
	sample_phase = gr::fast_atan2f(sample.imag(), sample.real());
	return mod_2pi(sample_phase - ref_phase);
}

int dsn_pn_rx_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items) {
	const gr_complex *iptr = (gr_complex*)input_items[0];
	gr_complex *o_ref = (gr_complex*)output_items[0];

	float error;
	float t_imag, t_real;
	int size = noutput_items;
	gr_complex nco_out;

	//Synchronize with a timed RX if there are any in the stream
	//TODO: Figure out how timed RX works with USRP...

	while(size-- > 0) {
		nco_out = gr_expj(-d_phase);
		*o_ref++ = nco_out;
		error = phase_detector(*iptr++, d_phase);

		//TODO: Make this work as RX not TX
		if(d_cur_composite.done){
			//Nothing to do if there's no valid sequence, just pass samples through instead
			if(d_composite_queue.size() > 0){
				d_cur_composite = d_composite_queue.pop_front();
			}
			out[count] = in[count];
		} else {
			float out_phase = 0.0;
			bool out_square = false;

			//Figure out where we are in time
			uint64_t cur_time_sec = (nread+count-d_cal_time_count)/d_samples_per_second + d_cal_time_seconds;
			uint64_t cur_time_frac = (nread+count-d_cal_time_count)%d_samples_per_second;

			//Figure out where we are in the current sequence
			int64_t cur_seq_sec = cur_time_sec - d_cur_composite.xmit_time;
			uint64_t cur_seq_frac = cur_time_frac;

			//Convert to number of range clock cycles
			double range_clk_phase = (double)cur_seq_sec*d_cur_composite.range_freq + cur_seq_frac*d_cur_composite.range_freq/d_samples_per_second;
			int64_t range_clk_cycles = (int64_t)(range_clk_phase);

			//Check if the current sequence is running or not
			if(d_cur_composite.running == false){
				if(cur_time_sec >= d_cur_composite.xmit_time-1){
					//Time to get this sequence running!
					d_cur_composite.running = true;
				}
			} else {
				out_phase = (range_clk_phase/pow%1.0)*M_TWOPI;
				out_square = d_cur_composite.range_is_square;
				int64_t pn_composite_idx = range_clk_cycles*2;
				if(out_phase > M_PI)
					pn_composite_idx++;

				bool cur_bit = (d_cur_composite.cm == CM_AND) ? true : false;
				int vote_count = 0;
				for(unsigned int ii=0; ii < d_cur_composite.components.size(); ii++){
					int cur_component_idx = pn_composite_idx % d_cur_composite.components[ii].size();
					switch(d_cur_composite.cm){
						case CM_AND:
							cur_bit &= d_cur_composite.components[ii][cur_component_idx];
							break;
						case CM_OR:
							cur_bit |= d_cur_composite.components[ii][cur_component_idx];
							break;
						case CM_XOR:
							cur_bit ^= d_cur_composite.components[ii][cur_component_idx];
							break;
						case CM_VOTE:
							if(d_cur_composite.components[ii][cur_component_idx]) vote_count++;
							break;
					}
				}

				//set the current bit according to vote if needed
				if(d_cur_composite.cm == CM_VOTE)
					cur_bit = (vote_count > d_cur_composite.components.size()/2);

				//1 corresponds to positive half-cycle, 0 corresponds to negative half-cycle
				if(cur_bit == false){
					out_phase += M_PI;
					out_phase %= M_TWOPI;
				}
			}

			gr_complex out_sample;
			out_sample.real = (out_square) ? 
				((out_phase > M_PI) ? -1.0 : 1.0) : sin(out1_phase);
			out_sample.imag = 0.0;
			out[count] = in[count] + out_sample;
		}


		advance_loop(error);
		phase_wrap();
		frequency_limit();
	}
	return noutput_items;
}

void dsn_pn_rx_impl::set_loop_bandwidth(float bw) {
	blocks::control_loop::set_loop_bandwidth(bw);
}

void dsn_pn_rx_impl::set_damping_factor(float df) {
	blocks::control_loop::set_damping_factor(df);
}

void dsn_pn_rx_impl::set_alpha(float alpha) {
	blocks::control_loop::set_alpha(alpha);
}

void dsn_pn_rx_impl::set_beta(float beta) {
	blocks::control_loop::set_beta(beta);
}

void dsn_pn_rx_impl::set_frequency(float freq) {
	blocks::control_loop::set_frequency(freq);
}

void dsn_pn_rx_impl::set_phase(float phase) {
	blocks::control_loop::set_phase(phase);
}

void dsn_pn_rx_impl::set_min_freq(float freq) {
	blocks::control_loop::set_min_freq(freq);
}

void dsn_pn_rx_impl::set_max_freq(float freq) {
	blocks::control_loop::set_max_freq(freq);
}


float dsn_pn_rx_impl::get_loop_bandwidth() const {
	return blocks::control_loop::get_loop_bandwidth();
}

float dsn_pn_rx_impl::get_damping_factor() const {
	return blocks::control_loop::get_damping_factor();
}

float dsn_pn_rx_impl::get_alpha() const {
	return blocks::control_loop::get_alpha();
}

float dsn_pn_rx_impl::get_beta() const {
	return blocks::control_loop::get_beta();
}

float dsn_pn_rx_impl::get_frequency() const {
	return blocks::control_loop::get_frequency();
}

float dsn_pn_rx_impl::get_phase() const {
	return blocks::control_loop::get_phase();
}

float dsn_pn_rx_impl::get_min_freq() const {
	return blocks::control_loop::get_min_freq();
}

float dsn_pn_rx_impl::get_max_freq() const {
	return blocks::control_loop::get_max_freq();
}

} /* namespace analog */
} /* namespace gr */
