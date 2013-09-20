/* -*- c++ -*- */
/*
 * Copyright 2011,2012 Free Software Foundation, Inc.
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

#include "constellation_soft_receiver_cf_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/math.h>
#include <gnuradio/expj.h>
#include <stdexcept>

namespace gr {
  namespace sdrp {

#define M_TWOPI (2*M_PI)
#define VERBOSE_MM     0     // Used for debugging symbol timing loop
#define VERBOSE_COSTAS 0     // Used for debugging phase and frequency tracking

    constellation_soft_receiver_cf::sptr 
    constellation_soft_receiver_cf::make(gr::digital::constellation_sptr constell,
				    float loop_bw, float fmin, float fmax)
    {
      return gnuradio::get_initial_sptr
	(new constellation_soft_receiver_cf_impl(constell, loop_bw,
					    fmin, fmax));
    }
 
    constellation_soft_receiver_cf_impl::constellation_soft_receiver_cf_impl(gr::digital::constellation_sptr constellation, 
								   float loop_bw, float fmin, float fmax)
      : block("constellation_soft_receiver_cf",
		 io_signature::make(1, 1, sizeof(gr_complex)),
		 io_signature::make(1, 1, sizeof(float))),
	blocks::control_loop(loop_bw, fmax, fmin),
	d_constellation(constellation),
	d_current_const_point(0)
    {
      if(d_constellation->dimensionality() != 1)
	throw std::runtime_error("This receiver only works with constellations of dimension 1.");
    }

    constellation_soft_receiver_cf_impl::~constellation_soft_receiver_cf_impl()
    {
    }

    void
    constellation_soft_receiver_cf_impl::phase_error_tracking(float phase_error)
    {
      advance_loop(phase_error);
      phase_wrap();
      frequency_limit();
  
#if VERBOSE_COSTAS
      printf("cl: phase_error: %f  phase: %f  freq: %f  sample: %f+j%f  constellation: %f+j%f\n",
	     phase_error, d_phase, d_freq, sample.real(), sample.imag(), 
	     d_constellation->points()[d_current_const_point].real(),
	     d_constellation->points()[d_current_const_point].imag());
#endif
    }

    int
    constellation_soft_receiver_cf_impl::general_work(int noutput_items,
						 gr_vector_int &ninput_items,
						 gr_vector_const_void_star &input_items,
						 gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *)input_items[0];
      float *out = (float *)output_items[0];

      int i=0;

      float phase_error;
      gr_complex sample, nco;

      std::vector<float> soft_bits;
      while((i < noutput_items) && (i < ninput_items[0])) {
	sample = in[i];
	nco = gr_expj(d_phase);   // get the NCO value for derotating the current sample
	sample = nco*sample;      // get the downconverted symbol

	soft_bits = d_constellation->soft_decision_maker(sample);
	d_constellation->decision_maker_pe(&sample, &phase_error);
	phase_error_tracking(phase_error);  // corrects phase and frequency offsets

	out[i] = soft_bits[0];

	i++;
      }

      consume_each(i);
      return i;
    }

  } /* namespace sdrp */
} /* namespace gr */

