#
# $Header: /var/cvs/d4x/main/Makefile.am,v 1.7 2002/05/21 11:19:30 zaufi Exp $
#

SUBDIRS = face

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~

# FIXME: We override 'default' value used by automake to avoid headers conflict
DEFAULT_INCLUDES = -I$(top_srcdir)

bin_PROGRAMS = nt

#
# HINT: c note from face/Makefile.am
#       I suppose it can be applied here too... :)
#       (with little modification of couse :)

# List of C++ modules (got from `ls -w50 -q *.cc
nt_SOURCES = $(strip                                \
  addr.cc      eff.cc      log.cc       schedule.cc \
  alt.cc       filter.cc   main.cc      segments.cc \
  autoadd.cc   fsearch.cc  mainlog.cc   signal.cc   \
  base64.cc    ftp.cc      memwl.cc     sm.cc       \
  client.cc    ftpd.cc     meter.cc     sndserv.cc  \
  xml.cc       history.cc  msgqueue.cc  socket.cc   \
  config.cc    hproxy.cc   mutex.cc     socks.cc    \
  cookie.cc    html.cc     nt.cc        sort.cc     \
  db.cc        http.cc     ping.cc      speed.cc    \
  dlist.cc     httpd.cc    queue.cc     srvclt.cc   \
  download.cc  liststr.cc  savedvar.cc  var.cc      \
  dqueue.cc    locstr.cc   savelog.cc               \
)

# List of headers (got from `ls -w50 -q *.h`)
noinst_HEADERS = $(strip                            \
  addr.h      eff.h      main.h      signal.h       \
  alt.h       filter.h   mainlog.h   sm.h           \
  autoadd.h   fsearch.h  memwl.h     sndserv.h      \
  base64.h    ftpd.h     meter.h     socket.h       \
  client.h    ftp.h      msgqueue.h  socks.h        \
  cond.h      history.h  mutex.h     sort.h         \
  config.h    hproxy.h   ntlocale.h  speed.h        \
  cookie.h    html.h     ping.h      srvclt.h       \
  dbc.h       httpd.h    queue.h     var.h          \
  db.h        http.h     savedvar.h  xml.h          \
  dlist.h     liststr.h  savelog.h                  \
  download.h  locstr.h   schedule.h                 \
  dqueue.h    log.h      segments.h                 \
)

nt_LDADD = $(srcdir)/face/libface.a
