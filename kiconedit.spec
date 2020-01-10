
Name:    kiconedit
Version: 4.4.0
Release: 10%{?dist}
Summary: An icon editor

License: GPLv2+
URL:     http://www.kde.org/
Source0: ftp://ftp.kde.org/pub/kde/stable/%{version}/src/extragear/%{name}-%{version}.tar.bz2
Source1: hi96-app-kiconedit.png

BuildRequires: kdelibs4-devel
BuildRequires: desktop-file-utils
BuildRequires: gettext

%{?_kde4_version:Requires: kdelibs4%{?_isa} >= %{_kde4_version}}

%description
KIconEdit is designed to help create icons for
KDE using the standard icon palette.


%prep
%setup -q

# update docbook version to make doc-translations build with kdelibs >= 4.5
sed -i -e 's#<!DOCTYPE book PUBLIC "-//KDE//DTD DocBook XML V4\.1\.2-Based Variant V1\.1//EN" "dtd/kdex\.dtd" \[#<!DOCTYPE book PUBLIC "-//KDE//DTD DocBook XML V4.2-Based Variant V1.1//EN" "dtd/kdex.dtd" [#g' doc-translations/*_kiconedit/*/index.docbook


%build

mkdir -p %{_target_platform}
pushd %{_target_platform}
%{cmake_kde4} ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

install -p -m644 -D %{SOURCE1} %{buildroot}%{_datadir}/icons/hicolor/96x96/apps/kiconedit.png


%find_lang %{name} --with-kde


%check
desktop-file-validate %{buildroot}%{_datadir}/applications/kde4/%{name}.desktop


%post
touch --no-create %{_datadir}/icons/hicolor &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
touch --no-create %{_datadir}/icons/hicolor &> /dev/null
gtk-update-icon-cache %{_datadir}/icons/hicolor &> /dev/null
update-desktop-database -q &> /dev/null ||:
fi

%posttrans
gtk-update-icon-cache %{_datadir}/icons/hicolor &> /dev/null
update-desktop-database -q &> /dev/null ||:


%files -f %{name}.lang
%doc AUTHORS COPYING COPYING.DOC NEWS
%{_kde4_bindir}/kiconedit
%{_kde4_appsdir}/kiconedit/
%{_kde4_datadir}/applications/kde4/kiconedit.desktop
%{_kde4_iconsdir}/hicolor/*/*/*


%changelog
* Mon Mar 06 2017 Jan Grulich <jgrulich@redhat.com> - 4.4.0-10
- Add icon in higher resolution
  Resolves: bz#823002

* Fri Jan 24 2014 Daniel Mach <dmach@redhat.com> - 4.4.0-9
- Mass rebuild 2014-01-24

* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 4.4.0-8
- Mass rebuild 2013-12-27

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.4.0-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.4.0-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Tue Jul 10 2012 Rex Dieter <rdieter@fedoraproject.org> 4.4.0-5
- cleanup .spec, fix scriptlets

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.4.0-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.4.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Sat Dec 11 2010 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.4.0-2
- update docbook version to make doc-translations build with kdelibs >= 4.5,
  fixes FTBFS (#599843)

* Fri Feb 12 2010 Sebastian Vahl <svahl@fedoraproject.org> - 4.4.0-1
- 4.4.0

* Thu Jan 28 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.3-3
- better Requires: kdelibs4 construct (using %%{_kde4_version})

* Wed Nov 25 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.3.3-2
- rebuild for Qt 4.6.0 RC1 in F13 (was built against Beta 1 with unstable ABI)

* Thu Nov 05 2009 Sebastian Vahl <svahl@fedoraproject.org> - 4.3.3-1
- 4.3.3

* Tue Sep 01 2009 Sebastian Vahl <svahl@fedoraproject.org> - 4.3.1-1
- 4.3.1

* Tue Aug 04 2009 Than Ngo <than@redhat.com> - 4.3.0-1
- 4.3.0

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Jun 10 2009 Sebastian Vahl <fedora@deadbabylon.de> 4.2.4-1
- 4.2.4

* Mon May 11 2009 Rex Dieter <rdieter@fedoraproject.org> 4.2.3-2
- fix %%_docdir/HTML/<lang> ownership

* Fri May 08 2009 Rex Dieter <rdieter@fedoraproject.org> 4.2.3-1
- 4.2.3
- optimize scriptlets

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Fri Jan 23 2009 Than Ngo <than@redhat.com> - 4.2.0-1
- 4.2.0

* Mon Nov 17 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.3-2
- scriptlet, dependency fixes

* Sun Nov 09 2008 Sebastian Vahl <fedora@deadbabylon.de> 4.1.3-1
- 4.1.3

* Fri Oct 03 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-1
- 4.1.2

* Fri Aug 29 2008 Than Ngo <than@redhat.com> 4.1.1-1
- 4.1.1

* Thu Aug 28 2008 Sebastian Vahl <fedora@deadbabylon.de> 4.1.0-1
- 4.1 (final)

* Mon May 26 2008 Than Ngo <than@redhat.com> 4.0.80-1
- 4.1 beta 1

* Thu Apr 03 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-2
- rebuild (again) for the fixed %%{_kde4_buildtype}

* Mon Mar 31 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-1
- update to 4.0.3
- rebuild for NDEBUG and _kde4_libexecdir

* Tue Mar 04 2008 Sebastian Vahl <fedora@deadbabylon.de> 4.0.2-1
- new upstream version: 4.0.2

* Thu Feb 14 2008 Sebastian Vahl <fedora@deadbabylon.de> 4.0.1-2
- remove reference to KDE 4 in summary

* Fri Feb 08 2008 Sebastian Vahl <fedora@deadbabylon.de> 4.0.1-1
- new upstream version: 4.0.1

* Fri Jan 25 2008 Sebastian Vahl <fedora@deadbabylon.de> 4.0.0-1
- Initial version of kde-4.0.0 version
