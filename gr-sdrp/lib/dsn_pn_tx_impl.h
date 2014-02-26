/* -*- c++ -*- */
/*
 * Copyright 2005,2006,2012 Free Software Foundation, Inc.
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

#ifndef INCLUDED_SDRP_DSN_PN_TX_IMPL_H
#define INCLUDED_SDRP_DSN_PN_TX_IMPL_H

#include <sdrp/dsn_pn_tx.h>
#include <dsn_pn_common.h>

namespace gr {
namespace sdrp {

class dsn_pn_tx_impl : public dsn_pn_tx
{
private:
	double d_time_step;
	double d_cur_time;
	int d_cur_profile_idx;
	std::list<PNComposite> d_composite_queue;
	PNComposite d_cur_composite;
	uint64_t d_cal_time_count;
	uint64_t d_cal_time_seconds;
	uint64_t d_samples_per_second;
	double d_cal_time_frac;

protected:

public:
	dsn_pn_tx_impl(double samples_per_second);
	~dsn_pn_tx_impl(){};

	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);

	virtual void queueRanging(std::string combination_method, uint64_t xmit_time, double T, std::vector<std::vector<bool> > components, double range_freq, bool range_square);
	virtual void sweep();
};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_DSN_PN_TX_IMPL_H */
