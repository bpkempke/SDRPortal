/* -*- c++ -*- */
/*
 * Copyright 2006,2011,2012 Free Software Foundation, Inc.
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


#ifndef INCLUDED_SDRP_COSTAS_REFOUT_FREQOUT_CCF_IMPL_H
#define INCLUDED_SDRP_COSTAS_REFOUT_FREQOUT_CCF_IMPL_H

#include <sdrp/costas_refout_freqout_ccf.h>

namespace gr {
  namespace sdrp {

    class costas_refout_freqout_ccf_impl : public costas_refout_freqout_ccf
    {
    private:
      int d_order;

      /*! \brief the phase detector circuit for 8th-order PSK loops
       *  \param sample complex sample
       *  \return the phase error
       */
      float phase_detector_8(gr_complex sample) const;    // for 8PSK

      /*! \brief the phase detector circuit for fourth-order loops
       *  \param sample complex sample
       *  \return the phase error
       */
      float phase_detector_4(gr_complex sample) const;    // for QPSK

      /*! \brief the phase detector circuit for second-order loops
       *  \param sample a complex sample
       *  \return the phase error
       */
      float phase_detector_2(gr_complex sample) const;    // for BPSK

      float (costas_refout_freqout_ccf_impl::*d_phase_detector)(gr_complex sample) const;

    public:
      costas_refout_freqout_ccf_impl(float loop_bw, int order);
      ~costas_refout_freqout_ccf_impl();
  
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } /* namespace sdrp */
} /* namespace gr */

#endif /* INCLUDED_SDRP_COSTAS_REFOUT_FREQOUT_CCF_IMPL_H */
