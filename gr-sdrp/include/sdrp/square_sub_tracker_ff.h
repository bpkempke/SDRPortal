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

#ifndef INCLUDED_SDRP_SQUARE_SUB_TRACKER_FF_H
#define INCLUDED_SDRP_SQUARE_SUB_TRACKER_FF_H

#include <sdrp/api.h>
#include <gnuradio/blocks/control_loop.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/msg_queue.h>

namespace gr {
namespace sdrp {

class SDRP_API square_sub_tracker_ff : virtual public gr::sync_block, virtual public blocks::control_loop {
public:
	typedef boost::shared_ptr<square_sub_tracker_ff> sptr;

	static sptr make(float loop_bw, float max_freq, float min_freq);

	virtual void set_loop_bandwidth(float bw) = 0;
	virtual void set_damping_factor(float df) = 0;
	virtual void set_alpha(float alpha) = 0;
	virtual void set_beta(float beta) = 0;
	virtual void set_frequency(float freq) = 0;
	virtual void set_phase(float phase) = 0;
	virtual void set_min_freq(float freq) = 0;
	virtual void set_max_freq(float freq) = 0;
	
	virtual float get_loop_bandwidth() const = 0;
	virtual float get_damping_factor() const = 0;
	virtual float get_alpha() const = 0;
	virtual float get_beta() const = 0;
	virtual float get_frequency() const = 0;
	virtual float get_phase() const = 0;
	virtual float get_min_freq() const = 0;
	virtual float get_max_freq() const = 0;

};

} /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_SQUARE_SUB_TRACKER_FF_H */
