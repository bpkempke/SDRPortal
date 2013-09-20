/* -*- c++ -*- */

#define DIGITAL_API
#define SDRP_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "sdrp_swig_doc.i"
%include "gnuradio/digital/constellation.h"

%{
#include "sdrp/ccsds_aos_framer.h"
#include "sdrp/ccsds_tm_framer.h"
#include "sdrp/constellation_soft_receiver_cf.h"
#include "sdrp/correlate_long_access_code_bb.h"
#include "sdrp/correlate_soft_access_tag_ff.h"
%}

%include "sdrp/ccsds_aos_framer.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, ccsds_aos_framer);

%include "sdrp/ccsds_tm_framer.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, ccsds_tm_framer);

%include "sdrp/constellation_soft_receiver_cf.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, constellation_soft_receiver_cf);

%include "sdrp/correlate_long_access_code_bb.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, correlate_long_access_code_bb);

%include "sdrp/correlate_soft_access_tag_ff.h"
GR_SWIG_BLOCK_MAGIC2(sdrp, correlate_soft_access_tag_ff);
