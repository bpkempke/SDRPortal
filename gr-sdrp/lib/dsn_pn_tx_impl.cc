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

#include "dsn_pn_tx_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <math.h>
#include <gnuradio/math.h>
#include <locale>

#define M_TWOPI 2.0f*M_PI

namespace gr {
namespace sdrp {

dsn_pn_tx::sptr dsn_pn_tx::make(double samples_per_second){
	return gnuradio::get_initial_sptr
		(new dsn_pn_tx_impl(samples_per_second));
}

dsn_pn_tx_impl::dsn_pn_tx_impl(double samples_per_second)
	: sync_block("dsn_pn_tx",
			io_signature::make(1, 1, sizeof(gr_complex)),
			io_signature::make(1, 1, sizeof(gr_complex))),
	d_cal_time_count(0),
	d_cal_time_seconds(0),
	d_cal_time_frac(0.0),
	d_samples_per_second(samples_per_second)
{
	d_composite_queue.clear();
	d_cur_composite.done = true;
}

void dsn_pn_tx_impl::queueRanging(std::string combination_method, uint64_t xmit_time, double T, std::vector<std::vector<bool> > components, double range_freq, bool range_is_square){
	//First switch combination_method to lowercase for later ease
	std::locale loc;
	for(int ii=0; ii < combination_method.size(); ii++)
		combination_method[ii] = std::tolower(combination_method[ii], loc);

	//NOTE: combination_method is expected to be 'and', 'or', 'xor', or 'vote'
	PNComposite new_composite;
	new_composite.cm = (combination_method == "and") ? CM_AND :
	                  (combination_method == "or") ? CM_OR :
	                  (combination_method == "xor") ? CM_XOR : CM_VOTE;
	new_composite.xmit_time = xmit_time;
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

int dsn_pn_tx_impl::work(int noutput_items,
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

	//TODO: make square more perfect in this band-limited case

	while(count < noutput_items){
		if(d_cur_composite.done){
			//Nothing to do if there's no valid sequence, just pass samples through instead
			if(d_composite_queue.size() > 0){
				d_cur_composite = d_composite_queue.front();
				d_composite_queue.pop_front();
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
				if(cur_time_sec >= d_cur_composite.xmit_time){
					//Time to get this sequence running!
					d_cur_composite.running = true;
				}
			} else {
				out_phase = fmod(range_clk_phase,1.0)*M_TWOPI;
				out_square = d_cur_composite.range_is_square;
				int64_t pn_composite_idx = (int64_t)(range_clk_phase*2);
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
					out_phase = fmod(out_phase,M_TWOPI);
				}

				//Check to make sure we are still running
				if(cur_seq_sec > d_cur_composite.T){
					d_cur_composite.running = false;
					d_cur_composite.done = true;
				}
			}

			float real_part = (out_square) ? 
				((out_phase > M_PI) ? -1.0 : 1.0) : sin(out_phase);
			gr_complex out_sample(real_part, 0.0);
			out[count] = in[count] + out_sample;
		}

		count++;
	}
	return noutput_items;
}

}
}
