#
# $Header: /var/cvs/d4x/main/Makefile.am,v 1.3 2002/03/10 08:13:15 max Exp $
#

SUBDIRS = face

# FIXME: We override 'default' value used by automake to avoid headers conflict
DEFAULT_INCLUDES = -I$(top_srcdir)

bin_PROGRAMS = nt

nt_SOURCES = base64.cc addr.cc queue.cc liststr.cc dlist.cc locstr.cc \
log.cc  sort.cc meter.cc socket.cc history.cc  speed.cc srvclt.cc html.cc \
client.cc ftp.cc http.cc download.cc ftpd.cc httpd.cc hproxy.cc signal.cc \
main.cc config.cc savelog.cc mainlog.cc var.cc db.cc cookie.cc nt.cc \
eff.cc segments.cc memwl.cc fsearch.cc msgqueue.cc ping.cc savedvar.cc \
schedule.cc socks.cc filter.cc sndserv.cc sm.cc autoadd.cc dqueue.cc \
mutex.cc alt.cc xml.cc

noinst_HEADERS = addr.h alt.h autoadd.h base64.h client.h cond.h config.h \
cookie.h dbc.h db.h dlist.h download.h dqueue.h eff.h filter.h fsearch.h \
ftpd.h ftp.h history.h hproxy.h html.h httpd.h http.h liststr.h locstr.h \
log.h main.h mainlog.h memwl.h meter.h msgqueue.h mutex.h ntlocale.h \
ping.h queue.h savedvar.h savelog.h schedule.h segments.h signal.h sm.h \
sndserv.h socket.h socks.h sort.h speed.h srvclt.h var.h xml.h

nt_LDADD = $(srcdir)/face/libface.a
