%define ver      1.26
%define prefix   /usr

Name: nt
Version: %ver
Release: 1
Copyright: free source but restricted to change
URL: http://www.krasu.ru/soft/chuchelo
Group: Applications/Internet
Summary: ftp/http download manager for X window system
Source: http://www.krasu.ru/soft/chuchelo/files/nt-%{ver}.tar.gz
BuildRoot: /tmp/nt-root
Packager: Anton Voloshin (vav@isv.ru)

%description
This program lets you download files from internet/intranet using
FTP or HTTP protocol.
Main features:
    * multithreaded design
    * convient user-friendly interface
    * automatic resuming after connection breaks
    * multiple simultaneous downloads
    * recursive FTP and HTTP downloading
    * ability to change links in HTML file for offline browsing
    * wildcards support for FTP recursing
    * filters support for HTTP recursing
    * proxy support (FTP and HTTP)
    * supports for traffic limitation
    * mass downloading function
    * FTP search
    * build-in scheduler
    * and many many other ...

%prep
%setup

%build
make -C main CCFLAGS="${RPM_OPT_FLAGS}" LDFLAGS="${RPM_OPT_FLAGS} -s" DEST=%prefix

%install
mkdir -p "${RPM_BUILD_ROOT}"%prefix/bin
mkdir -p "${RPM_BUILD_ROOT}"%prefix/doc
mkdir -p "${RPM_BUILD_ROOT}"/etc/X11/wmconfig
mkdir -p "${RPM_BUILD_ROOT}"%prefix/share/gnome/apps/Internet
mkdir -p "${RPM_BUILD_ROOT}"%prefix/share/pixmaps
mkdir -p "${RPM_BUILD_ROOT}"%prefix/man/man1
for i in main/po/*.gmo; do
    j=`basename $i .gmo`
    mkdir -p "${RPM_BUILD_ROOT}"%prefix/share/locale/$j/LC_MESSAGES
    cp -f $i "${RPM_BUILD_ROOT}"%prefix/share/locale/$j/LC_MESSAGES/nt.mo
done
cp -f main/nt "${RPM_BUILD_ROOT}"%prefix/bin/
cp -f nt.wmconfig "${RPM_BUILD_ROOT}"/etc/X11/wmconfig/nt
cp -f nt.desktop "${RPM_BUILD_ROOT}"%prefix/share/gnome/apps/Internet/nt.desktop
cp -f nt.xpm "${RPM_BUILD_ROOT}"%prefix/share/pixmaps/nt.xpm
cp -f nt-mini.xpm "${RPM_BUILD_ROOT}"%prefix/share/pixmaps/nt-mini.xpm
cp -f nt-gray.png "${RPM_BUILD_ROOT}"%prefix/share/pixmaps/nt-gray.png
cp -f nt.png "${RPM_BUILD_ROOT}"%prefix/share/pixmaps/nt.png
cp -f nt-wm.png "${RPM_BUILD_ROOT}"%prefix/share/pixmaps/nt-wm.png
cp -f nt.1 "${RPM_BUILD_ROOT}"%prefix/man/man1/nt.1

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-, root, root)
%doc ChangeLog FAQ FAQ.* INSTALL LICENSE NAMES PLANS README THANKS TODO TROUBLES README.* INSTALL.*
%prefix/bin/nt
%prefix/man/man1/*
/etc/X11/wmconfig/nt
%prefix/share/gnome/apps/Internet/nt.desktop
%prefix/share/pixmaps/nt.xpm
%prefix/share/pixmaps/nt-mini.xpm
%prefix/share/pixmaps/nt-gray.png
%prefix/share/pixmaps/nt.png
%prefix/share/pixmaps/nt-wm.png
%{prefix}/share/locale/*/*/*

%changelog
* Mon Oct 30 2000 Maxim Koshelev <mdem@chat.ru>
- fixed building under RH-70 or Mandrake-7x
