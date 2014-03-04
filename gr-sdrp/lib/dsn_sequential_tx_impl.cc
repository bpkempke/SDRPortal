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

#include "dsn_sequential_tx_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <math.h>
#include <gnuradio/math.h>

#define M_TWOPI 2.0f*M_PI

namespace gr {
namespace sdrp {

dsn_sequential_tx::sptr dsn_sequential_tx::make(double samples_per_second){
	return gnuradio::get_initial_sptr
		(new dsn_sequential_tx_impl(samples_per_second));
}

dsn_sequential_tx_impl::dsn_sequential_tx_impl(double samples_per_second)
	: sync_block("dsn_sequential_tx",
			io_signature::make(1, 1, sizeof(gr_complex)),
			io_signature::make(1, 1, sizeof(gr_complex))),
	d_cal_time_count(0),
	d_cal_time_seconds(0),
	d_cal_time_frac(0.0),
	d_samples_per_second(samples_per_second)
{
	sequence_queue.clear();
	cur_sequence.done = true;
}

void dsn_sequential_tx_impl::queueSequence(double f0, uint64_t XMIT, uint64_t T1, uint64_t T2, int range_clk_component, int chop_component, int end_component, bool range_is_square){

	sequenceType new_sequence;
	new_sequence.f0 = f0;
	new_sequence.XMIT = XMIT;
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

int dsn_sequential_tx_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items){

	const gr_complex *in = (gr_complex *) input_items[0];
	gr_complex *out = (gr_complex *) output_items[0];
	int count=0;
	gr_complex nco_out;

	//Synchronize with a timed TX if there are any in the stream
	std::vector<tag_t> tags;
	const uint64_t nread = this->nitems_read(0);
	this->get_tags_in_range(tags, 0, nread, nread+noutput_items, pmt::string_to_symbol("tx_time"));
	if(tags.size() > 0){
		const pmt::pmt_t &value = tags[tags.size()-1].value;
		d_cal_time_count = tags[tags.size()-1].offset-nread;
		d_cal_time_seconds = pmt::to_uint64(pmt::tuple_ref(value, 0));
		d_cal_time_frac = pmt::to_double(pmt::tuple_ref(value, 1));
		//Always make sure it is just synchronizing to a second boundary.  Otherwise this block gets confused
		assert(d_cal_time_frac == 0.0);
	}

	while(count < noutput_items){
		if(cur_sequence.done){
			//Nothing to do if there's no valid sequence, just pass samples through instead
			if(sequence_queue.size() > 0){
				cur_sequence = sequence_queue.front();
				sequence_queue.pop_front();
				d_cur_component = cur_sequence.range_clk_component;
			}
			out[count] = in[count];
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
			int64_t cur_seq_sec = cur_time_sec - cur_sequence.XMIT;
			uint64_t cur_seq_frac = cur_time_frac;

			//Convert to number of range clock cycles
			double range_clk_phase = (double)cur_seq_sec*pow(2.0, -cur_sequence.range_clk_component)*cur_sequence.f0 + cur_seq_frac*pow(2.0, -cur_sequence.range_clk_component)*cur_sequence.f0/d_samples_per_second;
			int64_t range_clk_cycles = (int64_t)(range_clk_phase);

			//Check if the current sequence is running or not
			if(cur_sequence.running == false){
				if(cur_time_sec >= cur_sequence.XMIT-1){
					//Time to get this sequence running!
					cur_sequence.running = true;
					cur_sequence.state = SEQ_T1;
				}
			} else {
				//out1 --> primary component
				//out2 --> chop component (if any)
				out1_phase = fmod((range_clk_phase/pow(2.0, d_cur_component-cur_sequence.range_clk_component)),1.0)*M_TWOPI;
				out2_phase = (d_cur_component >= cur_sequence.chop_component) ? 
					fmod((range_clk_phase/pow(2.0, cur_sequence.chop_component-cur_sequence.range_clk_component)),1.0)*M_TWOPI : 
					0.0;
				out1_square = cur_sequence.range_is_square || (d_cur_component >= cur_sequence.chop_component);
				out2_square = false; //Don't think there's any case where the chop component isn't square...

				//Current sequence is running, figure out where we are in the sequence and react accordingly
				switch(cur_sequence.state){
					case SEQ_T1: //Includes 1-second padding before
						//Don't need to wait for phase continuity here, just go if it's time
						if(cur_seq_sec >= cur_sequence.T1)
							cur_sequence.state = SEQ_T1_POST;
						break;

					case SEQ_T1_POST:
						//Frequency equals range clock until a switch can be made to the next component
						if(out1_phase >= 0.0 && d_out1_phase_last > M_PI){
							d_cur_component++;
							cur_sequence.state = SEQ_T2_PRE;
						}
						break;

					case SEQ_T2_PRE:
						if(cur_seq_sec >= cur_sequence.T1+2+(cur_sequence.T2+1)*(d_cur_component-cur_sequence.range_clk_component-1))
							cur_sequence.state = SEQ_T2;
						break;

					case SEQ_T2:
						//Check to see if the sequence is done yet
						if(cur_seq_sec >= cur_sequence.T1+1+(cur_sequence.T2+1)*(d_cur_component-cur_sequence.range_clk_component)){
							if(out1_phase >= 0.0 && d_out1_phase_last > M_PI){
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

			float real_out = (out1_square) ? 
				((out1_phase > M_PI/2 && out1_phase < 3*M_PI/2) ? -1.0 : 1.0) : cos(out1_phase);
			real_out += (out2_square) ? 
				((out2_phase > M_PI/2 && out2_phase < 3*M_PI/2) ? -1.0 : 1.0) : cos(out2_phase);
			gr_complex out_sample(real_out, 0.0);
			out[count] = in[count] + out_sample;
		}

		count++;
	}
	return noutput_items;
}

}
}
