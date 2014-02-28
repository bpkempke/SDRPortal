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

#include "dsn_sequential_rx_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <math.h>
#include <gnuradio/math.h>

namespace gr {
namespace sdrp {

#ifndef M_TWOPI
#define M_TWOPI (2.0f*M_PI)
#endif

dsn_sequential_rx::sptr dsn_sequential_rx::make(double samples_per_second, float loop_bw, float max_freq, float min_freq) {
	return gnuradio::get_initial_sptr
		(new dsn_sequential_rx_impl(samples_per_second, loop_bw, max_freq, min_freq));
}

static int ios[] = {sizeof(gr_complex), sizeof(float)};
static std::vector<int> iosig(ios, ios+sizeof(ios)/sizeof(int));
dsn_sequential_rx_impl::dsn_sequential_rx_impl(double samples_per_second, float loop_bw, float max_freq, float min_freq)
	: sync_block("dsn_sequential_rx",
		io_signature::make(1, 1, sizeof(gr_complex)),
		io_signature::makev(1, 2, iosig)),
	blocks::control_loop(loop_bw, max_freq, min_freq),
	d_cal_time_count(0),
	d_cal_time_seconds(0),
	d_cal_time_frac(0.0),
	d_samples_per_second(samples_per_second)
{
}

dsn_sequential_rx_impl::~dsn_sequential_rx_impl() {

}

void dsn_sequential_rx_impl::queueSequence(double f0, double RXTIME, double T1, double T2, int range_clk_component, int chop_component, int end_component, bool range_is_square){

	sequenceType new_sequence;
	new_sequence.f0 = f0;
	new_sequence.downlink_freq = f0*128/221*880;
	new_sequence.RXTIME = RXTIME;
	new_sequence.T1 = T1;
	new_sequence.T2 = T2;
	new_sequence.range_clk_component = range_clk_component;
	new_sequence.chop_component = chop_component;
	new_sequence.end_component = end_component;
	new_sequence.range_is_square = range_is_square;
	new_sequence.done = false;
	new_sequence.running = false;

	//Push new sequence onto queue and sort
	sequence_queue.push_back(new_sequence);
	sequence_queue.sort(compare_sequence_start);
}

float dsn_sequential_rx_impl::mod_2pi(float in) {
	if(in > M_PI)
		return in - M_TWOPI;
	else if(in < -M_PI)
		return in + M_TWOPI;
	else
		return in;
}

float dsn_sequential_rx_impl::phase_detector(gr_complex sample, float ref_phase) {
	float sample_phase;
	sample_phase = gr::fast_atan2f(sample.imag(), sample.real());
	return mod_2pi(sample_phase - ref_phase);
}

int dsn_sequential_rx_impl::work(int noutput_items,
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

		//TODO: Convert this code to do RX instead of TX...
		if(cur_sequence.done){
			//Nothing to do if there's no valid sequence, just pass samples through instead
			if(sequence_queue.size() > 0){
				cur_sequence = sequence_queue.pop_front();
				d_cur_component = cur_sequence.range_clk_component;
			}
		} else {
			//TODO: Check amplitude and phase of output modulation
			float out_real = 0.0;
			float out_imag = 0.0;

			float out1_phase = 0.0;
			float out2_phase = 0.0;
			bool out1_square = false;
			bool out2_square = false;

			//Figure out where we are in time
			uint64_t cur_time_sec = (nread+count-d_cal_time_count)/d_samples_per_second + d_cal_time_seconds;
			uint64_t cur_time_frac = (nread+count-d_cal_time_count)%d_samples_per_second;

			//Figure out where we are in the current sequence
			int64_t cur_seq_sec = cur_time_sec - cur_sequence.RXTIME;
			uint64_t cur_seq_frac = cur_time_frac;

			double phase_step = (d_phase < M_PI/2 && d_phase_last > 3*M_PI/2) ? M_TWOPI-d_phase_last+d_phase : 
			                    (d_phase > 3*M_PI/2 && d_phase_last < M_PI/2) ? d_phase-M_TWOPI-d_phase_last :
			                    d_phase-d_phase_last;
			d_combined_carrier_phase_error += phase_step;

			//Convert to number of range clock cycles
			double range_clk_phase = (double)cur_seq_sec*pow(2.0, -range_clk_component)*cur_sequence.f0 + cur_seq_frac*pow(2.0, -range_clk_component)*cur_sequence.f0/d_samples_per_second + d_combined_carrier_phase_error/M_TWOPI/cur_sequence.downlink_freq*pow(2.0, -range_clk_component)*cur_sequence.f0;
			int64_t range_clk_cycles = (int64_t)(range_clk_phase);

			//Check if the current sequence is running or not
			if(cur_sequence.running == false){
				if(cur_time_sec >= cur_sequence.RXTIME){
					//Time to get this sequence running!
					cur_sequence.running = true;
					cur_sequence.state = SEQ_T1;

					d_combined_carrier_phase_error;
				}
			} else {
				//out1 --> primary component
				//out2 --> chop component (if any)
				out1_phase = ((range_clk_phase/pow(2.0, d_cur_component-cur_sequence.range_clk_component))%1.0)*M_TWOPI;
				out2_phase = (d_cur_component >= cur_sequence.chop_component) ? 
					((range_clk_phase/pow(2.0, cur_sequence.chop_component-cur_sequence.range_clk_component))%1.0)*M_TWOPI : 
					0.0;
				out1_square = cur_component.range_is_square | (d_cur_component >= cur_sequence.chop_component);
				out2_square = false; //Don't think there's any case where the chop component isn't square...

				//Current sequence is running, figure out where we are in the sequence and react accordingly
				switch(cur_sequence.state){
					case SEQ_T1: //Does not include 1-second padding before
						//Don't need to wait for phase continuity here, just go if it's time
						if(out1_square)
							correlateSquare(*optr, out1_phase);
						else
							correlateSine(*optr, out1_phase);
						if(cur_seq_sec >= cur_sequence.T1){
							recordPhase(d_cur_component);
							cur_sequence.state = SEQ_T1_POST;
						}
						break;

					case SEQ_T1_POST:
						//Frequency equals range clock until a switch can be made to the next component
						if(out1_phase >= 0.0 && d_out1_phase_last > M_PI){
							d_cur_component++;
							cur_sequence.state = STATE_T2_PRE;
						}
						break;

					case SEQ_T2_PRE:
						if(cur_seq_sec >= cur_sequence.T1+2+(cur_sequence.T2+1)*(d_cur_component-cur_sequence.range_clk_component-1))
							cur_sequence.state = SEQ_T2;
						break;

					case SEQ_T2:

						//Record phase and de-chop if necessary
						if(d_cur_component >= cur_sequence.chop_component){
							//TODO: How do I deal with chop components??
						} else {
							if(out1_square)
								correlateSquare(*optr, out1_phase);
							else
								correlateSine(*optr, out1_phase);
						}
						//Check to see if the sequence is done yet
						if(cur_seq_sec >= cur_sequence.T1+1+(cur_sequence.T2+1)*(d_cur_component-cur_sequence.range_clk_component)){
							if(out1_phase >= 0.0 && d_out1_phase_last > M_PI){
								recordPhase(d_cur_component);
								cur_sequence.state = (d_cur_component == cur_sequence.end_component) ? SEQ_T2_POST : SEQ_T2_PRE;
								d_cur_component++;
							}
						}

						break;

					case SEQ_T2_POST:
						if(cur_seq_sec >= cur_sequence.T1+2+(cur_sequence.T2+1)*(d_cur_component-cur_sequence.range_clk_component-1)){
							cur_sequence.done = true;
							cur_sequence.running = false;
						}

						break;
				}

				d_out1_phase_last = out1_phase;
			}

			gr_complex out_sample;
			out_sample.real = (out1_square) ? 
				((out1_phase > M_PI/2 && out1_phase < 3*M_PI/2) ? -1.0 : 1.0) : cos(out1_phase);
			out_sample.real += (out2_square) ? 
				((out2_phase  M_PI/2 && out2_phase < 3*M_PI/2) ? -1.0 : 1.0) : cos(out2_phase);
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

void dsn_sequential_rx_impl::set_loop_bandwidth(float bw) {
	blocks::control_loop::set_loop_bandwidth(bw);
}

void dsn_sequential_rx_impl::set_damping_factor(float df) {
	blocks::control_loop::set_damping_factor(df);
}

void dsn_sequential_rx_impl::set_alpha(float alpha) {
	blocks::control_loop::set_alpha(alpha);
}

void dsn_sequential_rx_impl::set_beta(float beta) {
	blocks::control_loop::set_beta(beta);
}

void dsn_sequential_rx_impl::set_frequency(float freq) {
	blocks::control_loop::set_frequency(freq);
}

void dsn_sequential_rx_impl::set_phase(float phase) {
	blocks::control_loop::set_phase(phase);
}

void dsn_sequential_rx_impl::set_min_freq(float freq) {
	blocks::control_loop::set_min_freq(freq);
}

void dsn_sequential_rx_impl::set_max_freq(float freq) {
	blocks::control_loop::set_max_freq(freq);
}


float dsn_sequential_rx_impl::get_loop_bandwidth() const {
	return blocks::control_loop::get_loop_bandwidth();
}

float dsn_sequential_rx_impl::get_damping_factor() const {
	return blocks::control_loop::get_damping_factor();
}

float dsn_sequential_rx_impl::get_alpha() const {
	return blocks::control_loop::get_alpha();
}

float dsn_sequential_rx_impl::get_beta() const {
	return blocks::control_loop::get_beta();
}

float dsn_sequential_rx_impl::get_frequency() const {
	return blocks::control_loop::get_frequency();
}

float dsn_sequential_rx_impl::get_phase() const {
	return blocks::control_loop::get_phase();
}

float dsn_sequential_rx_impl::get_min_freq() const {
	return blocks::control_loop::get_min_freq();
}

float dsn_sequential_rx_impl::get_max_freq() const {
	return blocks::control_loop::get_max_freq();
}

} /* namespace analog */
} /* namespace gr */
