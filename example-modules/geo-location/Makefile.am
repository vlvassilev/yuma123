netconfmodule_LTLIBRARIES = libgeo-location.la

libgeo_location_la_SOURCES = \
geo-location.c

# Standalone mode. Depends on installed libyuma-dev
libgeo_location_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)
libgeo_location_la_LDFLAGS = -module -lyumaagt -lyumancx

yang_DATA = geo-location.yang

bin_SCRIPTS = get-geo-location
