# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

include(GrMiscUtils)
GR_INCLUDE_SUBDIRECTORY(fec)

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND sdrp_sources
    sweep_generator_cc_impl.cc
    ccsds_aos_framer_impl.cc
    ccsds_tm_framer_impl.cc
    ccsds_tm_tx_impl.cc
    constellation_soft_receiver_cf_impl.cc
    correlate_long_access_code_bb_impl.cc
    correlate_soft_access_tag_ff_impl.cc
    pll_refout_freqout_ccf_impl.cc
    costas_refout_freqout_ccf_impl.cc
    sdrp_packet_interpreter_impl.cc
)

add_library(gnuradio-sdrp SHARED ${sdrp_sources})
target_link_libraries(gnuradio-sdrp gnuradio-digital ${Boost_LIBRARIES} ${GNURADIO_RUNTIME_LIBRARIES})
set_target_properties(gnuradio-sdrp PROPERTIES DEFINE_SYMBOL "gnuradio_sdrp_EXPORTS")

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-sdrp
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_sdrp_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sdrp.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_sdrp.cc
)

add_executable(test-sdrp ${test_sdrp_sources})

target_link_libraries(
  test-sdrp
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-sdrp
)

GR_ADD_TEST(test_sdrp test-sdrp)
