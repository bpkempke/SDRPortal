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

#ifndef INCLUDED_SDRP_DSN_SEQUENTIAL_TX_IMPL_H
#define INCLUDED_SDRP_DSN_SEQUENTIAL_TX_IMPL_H

#include <sdrp/dsn_sequential_tx.h>

namespace gr {
namespace sdrp {

//Refer to DSN document for terminology used in this block:
// deepspace.jpl.nasa.gov/dsndocs/810-005/203/203C.pdf
enum sequenceState {
	SEQ_T1, SEQ_T1_POST, SEQ_T2_PRE, SEQ_T2, SEQ_T2_POST
};

struct sequenceType {
	double f0;
	uint64_t XMIT;
	uint64_t T1;
	uint64_t T2;
	int range_clk_component;
	int chop_component;
	int end_component;
	bool range_is_square;
	bool done;
	bool running;
	sequenceState state;
};

bool compare_sequence_start(const sequenceType &first, const sequenceType &second){
	if(first.XMIT < second.XMIT)
		return true;
	else
		return false;
}

class dsn_sequential_tx_impl : public dsn_sequential_tx
{
private:
	double d_cur_time;
	double d_freq;
	double d_phase;
	int d_cur_component;
	std::list<sequenceType> sequence_queue;
	sequenceType cur_sequence;
	float d_out1_phase_last;

	uint64_t d_cal_time_count;
	uint64_t d_cal_time_seconds;
	uint64_t d_samples_per_second;
	double d_cal_time_frac;

protected:

public:
	dsn_sequential_tx_impl(double samples_per_second);
	~dsn_sequential_tx_impl(){};

	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);

	virtual void queueSequence(double f0, uint64_t XMIT, uint64_t T1, uint64_t T2, int range_clk_component, int chop_component, int end_component, bool range_is_square);
};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_DSN_SEQUENTIAL_TX_IMPL_H */
