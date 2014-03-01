/* -*- c++ -*- */
/*
 * Copyright 2004,2011,2012 Free Software Foundation, Inc.
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

#ifndef INCLUDED_SDRP_DSN_SEQUENTIAL_RX_IMPL_H
#define INCLUDED_SDRP_DSN_SEQUENTIAL_RX_IMPL_H

#include <sdrp/dsn_sequential_rx.h>
#include <dsn_sequential_common.h>

namespace gr {
namespace sdrp {
    
class dsn_sequential_rx_impl : public dsn_sequential_rx
{
private:
	float phase_detector(gr_complex sample,float ref_phase);
	uint64_t d_cal_time_count;
	uint64_t d_cal_time_seconds;
	uint64_t d_samples_per_second;
	double d_cal_time_frac;
	float d_phase_last;
	double d_combined_carrier_phase_error;
	int d_cur_component;

	std::list<sequenceType> sequence_queue;
	sequenceType cur_sequence;

	std::vector<double> d_correlator;
	unsigned int d_correlator_idx;
	std::vector<unsigned int> d_corr_results;

	void recordPhase(int component1, int component2, bool c1_is_square, bool c2_is_square);
	void resetCorrelator();
	void updateCorrelator(float in, float phase);

public:
	dsn_sequential_rx_impl(double samples_per_second, float loop_bw, float max_freq, float min_freq);
	~dsn_sequential_rx_impl();

	float mod_2pi(float in);
	virtual void queueSequence(double f0, unsigned int interp_factor, uint64_t RXTIME, uint64_t T1, uint64_t T2, int range_clk_component, int chop_component, int end_component, bool range_is_square);

	void set_loop_bandwidth(float bw);
	void set_damping_factor(float df);
	void set_alpha(float alpha);
	void set_beta(float beta);
	void set_frequency(float freq);
	void set_phase(float phase);
	void set_min_freq(float freq);
	void set_max_freq(float freq);
	
	float get_loop_bandwidth() const;
	float get_damping_factor() const;
	float get_alpha() const;
	float get_beta() const;
	float get_frequency() const;
	float get_phase() const;
	float get_min_freq() const;
	float get_max_freq() const;
	
	int work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items);
};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_DSN_SEQUENTIAL_RX_IMPL_H */
