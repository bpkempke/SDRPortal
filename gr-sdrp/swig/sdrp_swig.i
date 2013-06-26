/* -*- c++ -*- */

#define SDRP_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "sdrp_swig_doc.i"

%{
#include "sdrp/ws_sink_c.h"
%}


%include "sdrp/ws_sink_c.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, ws_sink_c);
