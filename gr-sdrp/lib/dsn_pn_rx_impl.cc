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
	std::vector<tag_t> tags;
	const uint64_t nread = this->nitems_read(0);
	this->get_tags_in_range(tags, 0, nread, nread+noutput_items, pmt::string_to_symbol("rx_time"));
	if(tags.size() > 0){
		const pmt::pmt_t &value = tags[tags.size()-1].value;
		d_cal_time_count = tags[tags.size()-1].offset-nread;
		d_cal_time_seconds = pmt::to_uint64(pmt::tuple_ref(value, 0));
		d_cal_time_frac = pmt::to_double(pmt::tuple_ref(value, 1));
		//Always make sure it is just synchronizing to a second boundary.  Otherwise this block gets confused
		assert(d_cal_time_frac == 0.0);
	}


	while(size-- > 0) {
		gr::sincosf(d_phase, &t_imag, &t_real);
		*optr = *iptr * gr_complex(t_real, -t_imag);
		
		error = phase_detector(*iptr, d_phase);

		//TODO: Make this work as RX not TX
		if(d_cur_composite.done){
			//Nothing to do if there's no valid sequence, just pass samples through instead
			if(d_composite_queue.size() > 0){
				d_cur_composite = d_composite_queue.pop_front();
			}
		} else {
			float out_phase = 0.0;
			bool out_square = false;

			//Figure out where we are in time
			uint64_t cur_time_sec = (nread+count-d_cal_time_count)/d_samples_per_second + d_cal_time_seconds;
			uint64_t cur_time_frac = (nread+count-d_cal_time_count)%d_samples_per_second;

			//Figure out where we are in the current sequence
			int64_t cur_seq_sec = cur_time_sec - d_cur_composite.xmit_time;
			uint64_t cur_seq_frac = cur_time_frac;

			double phase_step = (d_phase < M_PI/2 && d_phase_last > 3*M_PI/2) ? M_TWOPI-d_phase_last+d_phase : 
			                    (d_phase > 3*M_PI/2 && d_phase_last < M_PI/2) ? d_phase-M_TWOPI-d_phase_last :
			                    d_phase-d_phase_last;
			d_combined_carrier_phase_error += phase_step;

			//Convert to number of range clock cycles
			double range_clk_phase = (double)cur_seq_sec*d_cur_composite.range_freq + cur_seq_frac*d_cur_composite.range_freq/d_samples_per_second + d_combined_carrier_phase_error/M_TWOPI/cur_sequence.downlink_freq*d_cur_composite.range_freq;
			int64_t range_clk_cycles = (int64_t)(range_clk_phase);

			//Check if the current sequence is running or not
			if(d_cur_composite.running == false){
				if(cur_time_sec >= d_cur_composite.xmit_time-1){
					//Time to get this sequence running!
					d_cur_composite.running = true;

					//Resize and initialize correlator matrix vectors
					d_range_clk_corr_max = 0.0;
					d_range_clk_corr_max_idx = 0;
					d_last_max_trigger_idx = -1;
					d_range_clk_corr_idx = 0;
					d_range_clk_corr.resize(d_samples_per_second/d_cur_composite.range_freq/2);
					for(unsigned int ii=0; ii < d_range_clk_corr.size(); ii++)
						d_range_clk_corr[ii] = 0.0;
					d_correlator_matrix.resize(d_cur_composite.components.size());
					for(unsigned int ii=0; ii < d_cur_composite.components.size(); ii++){
						d_correlator_matrix[ii].resize(d_cur_composite.components[ii].size());
						for(unsigned int jj=0; jj < d_cur_composite.components[ii].size(); jj++){
							d_correlator_matrix[ii][jj] = 0.0;
						}
					}

					//Change the amount of history for matched filtering
					d_history_circ_queue_idx = 0;
					d_history_circ_queue.resize(d_samples_per_second/d_cur_composite.range_freq/2);
					d_matched_filter_coeffs.resize(d_samples_per_second/d_cur_composite.range_freq/2);

					//Initialize the matched filter coefficients
					for(unsigned int ii=0; ii < d_matched_filter_coeffs.size(); ii++){
						d_matched_filter_coeffs[ii] = sin((float)ii/d_cur_composite.range_freq);
						d_history_circ_queue[ii] = 0.0;
					}

				}
			} else {
				out_phase = (range_clk_phase/pow%1.0)*M_TWOPI;
				out_square = d_cur_composite.range_is_square;
				int64_t pn_composite_idx = (int64_t)(range_clk_phase*2);
				if(out_phase > M_PI)
					pn_composite_idx++;

				//Do matched filtering
				float matched_filter_result = 0.0;
				d_history_circ_queue[d_history_circ_queue_idx] = *optr.imag;
				int history_idx = (int)d_history_circ_queue_idx;
				for(unsigned int ii=0; ii < d_history_circ_queue.size(); ii++){
					matched_filter_result += d_history_circ_queue[history_idx]*d_matched_filter_coeffs[ii];
					history_idx--;
					if(history_idx < 0)
						history_idx = d_history_circ_queue.size()-1;
				}
				d_history_circ_queue_idx++;
				if(d_history_circ_queue_idx >= d_history_circ_queue.size())
					d_history_circ_queue_idx = 0;

				//Update range clock correlator
				//This is so that we known when to update the correlator matrix
				bool max_trigger = false;
				int new_range_clk_corr_idx = ((range_clk_phase*2) % 1) * .9999 * d_range_
				while(d_range_clk_corr_idx != new_range_clk_corr_idx){
					d_range_clk_corr[d_range_clk_corr_idx] += abs(matched_filter_result);
					if(d_range_clk_corr[d_range_clk_corr_idx] > d_range_clk_corr_max){
						d_range_clk_corr_max = d_range_clk_corr[d_range_clk_corr_idx];
						d_range_clk_corr_max_idx = d_range_clk_corr_idx;
						max_trigger = true;
					} else if(d_range_clk_corr_idx == d_range_clk_corr_max_idx){
						max_trigger = true;
					}
					d_range_clk_corr_idx = (d_range_clk_corr_idx + 1) % d_range_clk_corr.size();
				}

				//If we hit a max in the clock correlation and we haven't trigger for this bit before, go ahead and update correlator matrices
				if(max_trigger && pn_composite_idx != d_last_max_trigger_idx){
					d_last_max_trigger_idx = pn_composite_idx;
					for(unsigned int ii=0; ii < d_cur_composite.components.size(); ii++){
						//Update d_correlator_matrix with new matched filter measurements
						d_correlator_matrix[ii][pn_composite_idx % d_cur_composite.components[ii].size()] += matched_filter_result;
					}
				}

				//Check to make sure we are still running
				if(cur_seq_sec > d_cur_composite.T){
					d_cur_composite.running = false;
					d_cur_composite.done = true;

					//Figure out what the range was by first determining each component's delay, then applying the chinese remainder theorem
					std::vector<int> best_component_delay(d_cur_composite.components.size());
					for(unsigned int ii=0; ii < d_cur_composite.components.size(); ii++){
						double best_corr = 0.0;
						int best_corr_shift = 0;
						//Try every possible shift and find the best fit
						for(unsigned int shift=0; shift < d_cur_composite.components[ii].size(); shift++){
							double total_corr = 0.0;
							for(unsigned int kk=0; kk < d_cur_composite.components[ii].size(); kk++){
								double cur_corr = d_correlator_matrix[ii][(shift+kk) % d_cur_composite.components[ii].size()];
								//Flip correlation if it's supposed to be a negative cycle
								if(!d_cur_composite.components[ii][kk]) cur_corr = -cur_corr;
								total_corr += cur_corr;
							}
							if(total_corr > best_corr){
								best_corr = total_corr;
								best_corr_shift = shift;
								best_component_delay[ii] = shift;
							}
						}
					}

					//Calculate overall PN pattern length
					unsigned int pn_length = 1;
					for(unsigned int ii=0; ii < d_cur_composite.components.size(); ii++)
						pn_length *= d_cur_composite.components[ii].size();
					//Figure out range using brute-force (would be better with generalized chinese remainder theorem...)
					for(unsigned int ii=0; ii < pn_length; ii++){
						unsigned int num_matching = 0;
						for(unsigned int jj=0; jj < d_cur_composite.components.size(); jj++){
							if(ii % jj == best_component_delay[jj])
								num_matching++;
						}
						if(num_matching == d_cur_composite.components.size()){
							//Found delay!
							//TODO: Figure out what to do with result....
							//TODO: Figure out what the inherent delay is within this gnuradio implementation
						}
					}

					//TODO: Could get better accuracy if we included range clock phase measurements in as well...
				}
			}

			gr_complex out_sample;
			out_sample.real = (out_square) ? 
				((out_phase > M_PI) ? -1.0 : 1.0) : sin(out1_phase);
			out_sample.imag = 0.0;
			out[count] = in[count] + out_sample;
		}


		d_phase_last = d_phase;

		iptr++;
		optr++;
		
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
