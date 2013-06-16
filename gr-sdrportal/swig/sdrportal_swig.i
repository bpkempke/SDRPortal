/* -*- c++ -*- */

#define SDRPORTAL_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "sdrportal_swig_doc.i"

%{
#include "sdrportal/ws_sink_c.h"
#include "sdrportal/ws_source_c.h"
%}


%include "sdrportal/ws_sink_c.h"
GR_SWIG_BLOCK_MAGIC2(sdrportal, ws_sink_c);
%include "sdrportal/ws_source_c.h"
GR_SWIG_BLOCK_MAGIC2(sdrportal, ws_source_c);
