Name: xorg-x11-module-xdbg
Summary: Xserver debug module
Version: 0.1.47
Release:    3
VCS:        adaptation/xorg/driver/xserver-xorg-module-xdbg#xorg-x11-module-xdbg-0.1.13-1-79-gfb1d23b62faa7f302ac1b19ff1d3f757c4745b13
Group:      System/Libraries
License:    MIT
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(xorg-server)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(xorg-macros)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(bigreqsproto)
BuildRequires:  pkgconfig(xcomposite)
BuildRequires:  pkgconfig(compositeproto)
BuildRequires:  pkgconfig(damageproto)
BuildRequires:  pkgconfig(libdri2)
BuildRequires:  pkgconfig(dri2proto)
BuildRequires:  pkgconfig(fixesproto)
BuildRequires:  pkgconfig(fontsproto)
BuildRequires:  pkgconfig(xgesture)
BuildRequires:  pkgconfig(gestureproto)
BuildRequires:  pkgconfig(inputproto)
BuildRequires:  pkgconfig(kbproto)
BuildRequires:  pkgconfig(xrandr)
BuildRequires:  pkgconfig(randrproto)
BuildRequires:  pkgconfig(recordproto)
BuildRequires:  pkgconfig(renderproto)
BuildRequires:  pkgconfig(resourceproto)
BuildRequires:  pkgconfig(videoproto)
BuildRequires:  pkgconfig(xcmiscproto)
BuildRequires:  pkgconfig(xextproto)
BuildRequires:  pkgconfig(xf86vidmodeproto)
BuildRequires:  pkgconfig(xproto)
BuildRequires:  pkgconfig(dri3proto)
BuildRequires:  pkgconfig(presentproto)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(xtst)
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xv)

%description
This package provides the runtime debug library and module for debug of inside X server.

%package devel
Summary: X server runtime debug library development package
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: pkgconfig(xorg-server)
Requires: pkgconfig(x11)
Requires: pkgconfig(xproto)
Requires: pkgconfig(dlog)

%description devel
X server runtime debug library development package

%prep
%setup -q

%build
%if 0%{?tizen_build_binary_release_type_eng}
export XDBG_OPTIONS="--enable-secure-log"
%endif
%reconfigure --disable-static --prefix=/usr $XDBG_OPTIONS \
  CFLAGS="$CFLAGS -Wall -Werror" \
  LDFLAGS="$LDFLAGS -Wl,--hash-style=both -Wl,--as-needed"
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp -af COPYING %{buildroot}/usr/share/license/%{name}
%make_install

%remove_docs

%files
%manifest xorg-x11-module-xdbg.manifest
%defattr(-,root,root,-)
/usr/share/license/%{name}
%{_bindir}/xdbg
%{_bindir}/xevlog_analyze
%{_libdir}/libxdbg-lib.so.*
%{_libdir}/xorg/modules/libxdbg.so
%{_sysconfdir}/dbus-1/system.d/xdbg.conf

%files devel
%dir %{_includedir}/xdbg/
%{_includedir}/xdbg/*.h
%{_libdir}/libxdbg-lib.so
%{_libdir}/pkgconfig/xdbg.pc

