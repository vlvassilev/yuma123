netconfmodule_LTLIBRARIES = libietf-traffic-generator.la

libietf_traffic_generator_la_SOURCES = \
ietf-traffic-generator.c

if STANDALONE
  # Standalone mode. Depends on installed libyuma-dev
  libietf_traffic_generator_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
  libietf_traffic_generator_la_LDFLAGS = -module -lyumaagt -lyumancx
else
  # Integrated yuma123 build
  libietf_traffic_generator_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform
  libietf_traffic_generator_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la
endif

bin_PROGRAMS = traffic-generator

traffic_generator_SOURCES = \
traffic-generator.c \
libtraffic-generator.c \
raw-socket.c \
timespec-math.c \
yang-date-and-time.c

traffic_generator_CPPFLAGS =
traffic_generator_LDFLAGS = -lz

bin_PROGRAMS += traffic-generator-make-testframe

traffic_generator_make_testframe_SOURCES = \
traffic-generator-make-testframe.c \
libtraffic-generator.c \
timespec-math.c \
yang-date-and-time.c

traffic_generator_make_testframe_CPPFLAGS =
traffic_generator_make_testframe_LDFLAGS = -lz

#bin_PROGRAMS += traffic-generator-dpdk
#
#traffic_generator_dpdk_SOURCES = \
#traffic-generator-dpdk.c

#traffic_generator_dpdk_CPPFLAGS = -I${includedir}/dpdk -I${includedir}/x86_64-linux-gnu/dpdk
#traffic_generator_dpdk_LDFLAGS =

netconfmodule_LTLIBRARIES += libietf-traffic-analyzer.la

libietf_traffic_analyzer_la_SOURCES = \
ietf-traffic-analyzer.c \
dict.c


if STANDALONE
  # Standalone mode. Depends on installed libyuma-dev
  libietf_traffic_analyzer_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
  libietf_traffic_analyzer_la_LDFLAGS = -module -lyumaagt -lyumancx
else
  # Integrated yuma123 build
  libietf_traffic_analyzer_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform
  libietf_traffic_analyzer_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la
endif

bin_PROGRAMS += traffic-analyzer

traffic_analyzer_SOURCES = \
traffic-analyzer.c \
libtraffic-analyzer.c \
raw-socket.c \
timespec-math.c \
yang-date-and-time.c

traffic_analyzer_LDFLAGS = -pthread

#dist_examples_yang_DATA = \
#traffic-generator-testframe.yang

