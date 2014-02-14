/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
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

#include "ccsds_tm_conv_decoder_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
  namespace sdrp {

    ccsds_tm_conv_decoder::sptr ccsds_tm_conv_decoder::make()
    {
      return gnuradio::get_initial_sptr(new ccsds_tm_conv_decoder_impl());
    }

    ccsds_tm_conv_decoder_impl::ccsds_tm_conv_decoder_impl()
      : block("ccsds_tm_conv_decoder",
			  io_signature::make (1, 1, sizeof(float)),
			  io_signature::make (1, 1, sizeof(char))), d_count(0)  // Rate 1/2 code
    {
      float RATE = 0.5;
      float ebn0 = 12.0;
      float esn0 = RATE*pow(10.0, ebn0/10.0);
      
      //Start off with convolutional decoding disabled
      d_conv_en = false;
      
      gen_met(d_mettab, 100, esn0, 0.0, 256);
      viterbi_chunks_init(d_state0);
      viterbi_chunks_init(d_state1);
    }

    //Enable/disable Convolutional decoding functionality
    void ccsds_tm_conv_decoder_impl::setConvEn(bool conv_en){
      d_conv_en = conv_en;
    }

    void ccsds_tm_conv_decoder_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required){
      if(d_conv_en)
        ninput_items_required[0] = noutput_items*2; //Rate 1/2 code
      else
        ninput_items_required[0] = noutput_items; //Either uncoded or coded after binary slicing
    }

    int
    ccsds_tm_conv_decoder_impl::general_work(int noutput_items,
				  gr_vector_int &ninput_items,
				  gr_vector_const_void_star &input_items,
				  gr_vector_void_star &output_items)
    {
      const float *in = (const float *)input_items[0];
      unsigned char *out = (unsigned char *)output_items[0];

      int noutput_ret = 0;
      int consume_count = ninput_items[0];

      //Handle corner case of inconvenient mode switch timing
      if(d_conv_en && (consume_count > (noutput_items-8)*2))
        consume_count = (noutput_items-8)*2;
      else if(!d_conv_en && (consume_count > noutput_items))
        consume_count = noutput_items;
      
      for (int i = 0; i < consume_count; i++) {
        if(d_conv_en){
          //Negate every other bit (???)
          // Translate and clip [-1.0..1.0] to [28..228]
          float sample = in[i]*100.0+128.0;
          if (sample > 255.0)
            sample = 255.0;
          else if (sample < 0.0)
            sample = 0.0;
          unsigned char sym = (unsigned char)(floor(sample));

	  //sym == 128 means erased, so let's avoid that
	  sym = (sym == 128) ? 129 : sym;
          
          d_viterbi_in[d_count % 4] = sym;
          if ((d_count % 4) == 3) {
            // Every fourth symbol, perform butterfly operation
            viterbi_butterfly2(d_viterbi_in, d_mettab, d_state0, d_state1);
            
            // Every sixteenth symbol, read out a byte
            if (d_count % 16 == 11) {
              unsigned char cur_byte;
              viterbi_get_output(d_state0, &cur_byte);
              for(int ii=0; ii<8; ii++)
                out[noutput_ret++] = (cur_byte & (0x80 >> ii)) ? 1 : 0;
            }
          }
          
          d_count++;
        } else {
          //Otherwise just do a straight binary slice
          out[noutput_ret++] = (in[i] > 0.0);
        }
      }

      //Always consume all items from input no matter what.
      consume(0,consume_count);

      //Variable number of output symbols in the case of convolutional encoding
      return noutput_ret;
    }

  } /* namespace sdrp */
}/* namespace gr */
