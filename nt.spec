%define ver      1.30

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
make -C main CCFLAGS="${RPM_OPT_FLAGS}" LDFLAGS="${RPM_OPT_FLAGS} -s" DEST=%{_prefix}

%install
mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/X11/applnk/Internet
mkdir -p $RPM_BUILD_ROOT%{_datadir}/pixmaps
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man1
for i in main/po/*.gmo; do
    j=`basename $i .gmo`
    mkdir -p $RPM_BUILD_ROOT%{_datadir}/locale/$j/LC_MESSAGES
    cp -f $i $RPM_BUILD_ROOT%{_datadir}/locale/$j/LC_MESSAGES/nt.mo
done
for i in main/sounds/*.wav; do
    mkdir -p $RPM_BUILD_ROOT%{_datadir}/d4x/sounds
    cp -f $i $RPM_BUILD_ROOT%{_datadir}/d4x/sounds/
done
cp -f main/nt $RPM_BUILD_ROOT%{_bindir}/
cp -f nt.desktop $RPM_BUILD_ROOT%{_sysconfdir}/X11/applnk/Internet/nt.desktop
cp -f nt.xpm $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt.xpm
cp -f nt-mini.xpm $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt-mini.xpm
cp -f nt-gray.png $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt-gray.png
cp -f nt.png $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt.png
cp -f nt-wm.png $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt-wm.png
cp -f nt.1 $RPM_BUILD_ROOT%{_mandir}/man1/nt.1

%find_lang %{name}

%clean
rm -rf ${RPM_BUILD_ROOT}
rm -rf nt-%{ver}

%files -f %{name}.lang
%defattr(-, root, root)
%doc ChangeLog FAQ FAQ.* INSTALL LICENSE NAMES PLANS README THANKS TODO TROUBLES README.* INSTALL.*
%{_bindir}/nt
%{_mandir}/man1/*1.gz
%{_sysconfdir}/X11/applnk/Internet/nt.desktop
%{_datadir}/pixmaps/nt.xpm
%{_datadir}/pixmaps/nt-mini.xpm
%{_datadir}/pixmaps/nt-gray.png
%{_datadir}/pixmaps/nt.png
%{_datadir}/pixmaps/nt-wm.png
%{_datadir}/d4x/sounds/*

%changelog

* Sat Oct 6 2001 leon@asplinux.ru
- rewrite spec using new macros system, langify

* Sat Apr 28 2001 Maxim Koshelev <mdem@chat.ru>
- added sounds instalation

* Mon Oct 30 2000 Maxim Koshelev <mdem@chat.ru>
- fixed building under RH-70 or Mandrake-7x