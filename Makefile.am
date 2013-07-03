AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = uhdd shelld
uhdd_SOURCES = src/base64.cc src/genericSDRInterface.cc src/hierarchicalDataflowBlock.cc src/portalDataSocket.cc src/sha1.cc src/uhdInterface.cc src/generic.cc src/genericSocketInterface.cc src/portalCommandSocket.cc src/rtlInterface.cc src/uhd_daemon.cc
shelld_SOURCES = src/base64.cc src/genericSDRInterface.cc src/hierarchicalDataflowBlock.cc src/portalDataSocket.cc src/sha1.cc src/shellPortal.cc src/generic.cc src/genericSocketInterface.cc src/rtlInterface.cc src/shell_daemon.cc
noinst_HEADERS = lib/base64.h lib/genericSDRInterface.h lib/hierarchicalDataflowBlock.h lib/portalDataSocket.h lib/sha1.h lib/streamConverter.h lib/generic.h lib/genericSocketInterface.h lib/portalCommandSocket.h lib/rtlInterface.h lib/shellPortal.h lib/uhdInterface.h

uhdd_CPPFLAGS = -std=c++0x -Wall -g -Ilib
uhdd_LDFLAGS = -pthread
uhdd_LDADD = -luhd -lrtlsdr -lhackrf

shelld_CPPFLAGS = -std=c++0x -Wall -g -Ilib
shelld_LDFLAGS = -pthread
shelld_LDADD = -luhd -lrtlsdr -lhackrf
