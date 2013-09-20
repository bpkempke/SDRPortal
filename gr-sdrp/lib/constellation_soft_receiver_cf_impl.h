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

#ifndef INCLUDED_SDRP_CONSTELLATION_SOFT_RECEIVER_CF_IMPL_H
#define	INCLUDED_SDRP_CONSTELLATION_SOFT_RECEIVER_CF_IMPL_H

#include <sdrp/constellation_soft_receiver_cf.h>
#include <gnuradio/attributes.h>
#include <gnuradio/gr_complex.h>
#include <gnuradio/blocks/control_loop.h>

namespace gr {
  namespace sdrp {

    class constellation_soft_receiver_cf_impl
      : public constellation_soft_receiver_cf, blocks::control_loop
    {
    public:
      constellation_soft_receiver_cf_impl(gr::digital::constellation_sptr constell, 
				     float loop_bw, float fmin, float fmax);

      ~constellation_soft_receiver_cf_impl();

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);

    protected:
      void phase_error_tracking(float phase_error);

    private:
      unsigned int d_M;

      gr::digital::constellation_sptr d_constellation;
      unsigned int d_current_const_point;

      //! delay line length.
      static const unsigned int DLLEN = 8;
  
      //! delay line plus some length for overflow protection
      __GR_ATTR_ALIGNED(8) gr_complex d_dl[2*DLLEN];
  
      //! index to delay line
      unsigned int d_dl_idx;
    };

  } /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_CONSTELLATION_SOFT_RECEIVER_CF_IMPL_H */
