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

#ifndef INCLUDED_SDRP_PLL_REFOUT_FREQOUT_CCF_IMPL_H
#define INCLUDED_SDRP_PLL_REFOUT_FREQDET_CCF_IMPL_H

#include <sdrp/pll_refout_freqout_ccf.h>

namespace gr {
  namespace sdrp {
    
    class pll_refout_freqout_ccf_impl : public pll_refout_freqout_ccf
    {
    private:
      float phase_detector(gr_complex sample,float ref_phase);

    public:
      pll_refout_freqout_ccf_impl(float loop_bw, float max_freq, float min_freq);
      ~pll_refout_freqout_ccf_impl();

      float mod_2pi(float in);

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

#endif /* INCLUDED_SDRP_REFOUT_FREQOUT_CCF_IMPL_H */
