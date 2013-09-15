/* -*- c++ -*- */

#define SDRP_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "sdrp_swig_doc.i"

%{
#include "sdrp/ws_sink_c.h"
#include "sdrp/sdrp_ccsds_aos_framer.h"
#include "sdrp/sdrp_correlate_long_access_code_bb.h"
%}

%include "sdrp/ws_sink_c.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, ws_sink_c);

%include "sdrp/sdrp_ccsds_aos_framer.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, ccsds_aos_framer);

%include "sdrp/sdrp_correlate_long_access_code_bb.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, correlate_long_access_code_bb);
