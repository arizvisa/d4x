%define ver      1.10
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
ftp or http protocol.
Main features:
    * multithreaded design
    * convient user-friendly interface
    * automatic resuming after connection break
    * multiple simultaneous downloads
    * recursive ftp and http downloading
    * mask support for ftp recursing
    * proxy support
    * and other ...

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
for i in es de it ru bg pt_BR fr cs nl ja id zh_CN.EUC; do
    mkdir -p "${RPM_BUILD_ROOT}"%prefix/share/locale/$i/LC_MESSAGES
    cp -f main/po/$i.gmo "${RPM_BUILD_ROOT}"%prefix/share/locale/$i/LC_MESSAGES/nt.mo
done
cp -f main/nt "${RPM_BUILD_ROOT}"%prefix/bin/
cp -f nt.wmconfig "${RPM_BUILD_ROOT}"/etc/X11/wmconfig/nt
cp -f nt.desktop "${RPM_BUILD_ROOT}"%prefix/share/gnome/apps/Internet/nt.desktop
cp -f nt.xpm "${RPM_BUILD_ROOT}"%prefix/share/pixmaps/nt.xpm

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-, root, root)
%doc ChangeLog FAQ FAQ.es FAQ.fr INSTALL LICENSE NAMES PLANS README THANKS TODO TROUBLES
%prefix/bin/nt
/etc/X11/wmconfig/nt
%prefix/share/gnome/apps/Internet/nt.desktop
%prefix/share/pixmaps/nt.xpm
%{prefix}/share/locale/*/*/*
