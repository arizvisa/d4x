%define version	2.03
%define name	d4x
%define prefix	/usr

Name:		%{name}
Version:	%{version}
Release:	1
Serial:		1
Copyright:	free source but restricted to change
URL:		http://www.krasu.ru/soft/chuchelo
Group:		Applications/Internet
Summary:	FTP/HTTP download manager for X window system
Source:		%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
Packager:	Anton Voloshin (vav@isv.ru)
Requires:	gtk+ >= 1.2.10

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
    * proxy support (FTP,HTTP and SOCKS5)
    * supports for traffic limitation
    * mass downloading function
    * FTP search
    * build-in scheduler
    * and many many other ...

%prep
%setup

%build
./configure --enable-release --prefix=%{prefix}
make

%install
mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/X11/applnk/Internet
mkdir -p $RPM_BUILD_ROOT%{_datadir}/pixmaps
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man1
mkdir -p $RPM_BUILD_ROOT%{_datadir}/d4x
make prefix=${RPM_BUILD_ROOT}%{prefix} install
cp -f main/nt $RPM_BUILD_ROOT%{_bindir}/
cp -f nt.desktop $RPM_BUILD_ROOT%{_sysconfdir}/X11/applnk/Internet/nt.desktop
cp -f nt.xpm $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt.xpm
cp -f nt-mini.xpm $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt-mini.xpm
cp -f nt-gray.png $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt-gray.png
cp -f nt.png $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt.png
cp -f nt-wm.png $RPM_BUILD_ROOT%{_datadir}/pixmaps/nt-wm.png
cp -f DOC/nt.1 $RPM_BUILD_ROOT%{_mandir}/man1/nt.1

%find_lang %{name}

%clean
[ "${RPM_BUILD_ROOT}" != "/" ] && [ -d ${RPM_BUILD_ROOT} ] && rm -rf ${RPM_BUILD_ROOT};

%files -f %{name}.lang
%defattr(-, root, root)
%doc DOC/*
%{_bindir}/nt
%{_mandir}/man1/*
%{_sysconfdir}/X11/applnk/Internet/nt.desktop
%{_datadir}/pixmaps/nt.xpm
%{_datadir}/pixmaps/nt-mini.xpm
%{_datadir}/pixmaps/nt-gray.png
%{_datadir}/pixmaps/nt.png
%{_datadir}/pixmaps/nt-wm.png
%{_datadir}/d4x/sounds/*
%{_datadir}/d4x/themes/*
%{_datadir}/d4x/ftpsearch.xml

%changelog

* Sat Mar 9 2002 max@krascoal.ru
- rewrite to spec.in for autoconf/automake

* Sat Oct 6 2001 leon@asplinux.ru
- rewrite spec using new macros system, langify

* Sat Apr 28 2001 Maxim Koshelev <mdem@chat.ru>
- added sounds instalation

* Mon Oct 30 2000 Maxim Koshelev <mdem@chat.ru>
- fixed building under RH-70 or Mandrake-7x