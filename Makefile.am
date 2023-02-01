bin_PROGRAMS = client client_cpp

noinst_LTLIBRARIES = liblumberjack.la
liblumberjack_la_LDFLAGS   = -all-static
liblumberjack_la_SOURCES   = lumberjack.c utils.c

client_SOURCES    = examples/client.c examples/metric.c
client_LDADD      = $(top_builddir)/liblumberjack.la
client_LDFLAGS    = -all-static

client_cpp_SOURCES    = examples/client_thread.cpp examples/metric.c
client_cpp_LDADD      = $(top_builddir)/liblumberjack.la
client_cpp_LDFLAGS    = -all-static -std=c++11