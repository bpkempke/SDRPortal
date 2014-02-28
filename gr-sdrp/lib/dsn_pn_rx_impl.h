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

#ifndef INCLUDED_SDRP_DSN_PN_RX_IMPL_H
#define INCLUDED_SDRP_PLL_REFOUT_FREQDET_CCF_IMPL_H

#include <sdrp/dsn_pn_rx.h>
#include <dsn_pn_common.h>

namespace gr {
namespace sdrp {
    
class dsn_pn_rx_impl : public dsn_pn_rx
{
private:
	float phase_detector(gr_complex sample,float ref_phase);
	std::list<PNComposite> d_composite_queue;
	PNComposite d_cur_composite;
	uint64_t d_cal_time_count;
	uint64_t d_cal_time_seconds;
	uint64_t d_samples_per_second;
	std::vector<float> d_history_circ_queue;
	unsigned int d_history_circ_queue_idx;
	std::vector<float> d_matched_filter_coeffs;
	double d_cal_time_frac;

	std::vector<std::vector<double> > d_correlator_matrix;
	std::vector<std::vector<double> > d_range_clk_corr;
	int d_range_clk_corr_idx;
	double d_range_clk_corr_max;
	int d_range_clk_corr_max_idx;
	int d_last_max_trigger_idx;

public:
	dsn_pn_rx_impl(double samples_per_second, float loop_bw, float max_freq, float min_freq);
	~dsn_pn_rx_impl();
	
	float mod_2pi(float in);
	virtual void queueRanging(std::string combination_method, uint64_t rx_time, double T, std::vector<std::vector<bool> > components, double range_freq, bool range_square);
	
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

#endif /* INCLUDED_SDRP_DSN_PN_RX_IMPL_H */
