bin_PROGRAMS = client

noinst_LTLIBRARIES = liblumberjack.la
liblumberjack_la_LDFLAGS   = -all-static
liblumberjack_la_SOURCES   = lumberjack.c utils.c

client_SOURCES    = examples/client.c
client_LDADD      = $(top_builddir)/liblumberjack.la
client_LDFLAGS    = -all-static