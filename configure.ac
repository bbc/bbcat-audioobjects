AC_PREREQ([2.68])
AC_INIT([BBCAT_AUDIOOBJECTS],[0.1.1.0])

AS_NANO(BBCAT_AUDIOOBJECTS_CVS=no,BBCAT_AUDIOOBJECTS_CVS=yes)

AM_INIT_AUTOMAKE([1.11 silent-rules])
AM_SILENT_RULES([yes])

# Version number that will be appended to the library filename
BBCAT_AUDIOOBJECTS_MAJORMINOR=0.1
AC_SUBST(BBCAT_AUDIOOBJECTS_MAJORMINOR)

AC_CONFIG_HEADERS([config.h])

# CURRENT, REVISION, AGE
# - library source changed but interface the same -> increment REVISION
# - interfaces added/removed/changed -> increment CURRENT, set REVISION = 0
# - interfaces added -> increment AGE also
# - interfaces removed -> set AGE = 0
#    (AGE is the number of previous values of CURRENT that are compatible)
BBCAT_AUDIOOBJECTS_LIBVERSION="1:0:0"
AC_SUBST(BBCAT_AUDIOOBJECTS_LIBVERSION)

LT_PREREQ([2.2.6])
LT_INIT(disable-static win32-dll)

AC_PROG_CXX
AM_PROG_CC_C_O

AC_CONFIG_SRCDIR([src])

AC_CONFIG_MACRO_DIR([m4])
ACLOCAL_AMFLAGS="-I m4 $ACLOCAL_AMFLAGS"
AC_SUBST(ACLOCAL_AMFLAGS)

DX_DOXYGEN_FEATURE(ON)
DX_INIT_DOXYGEN(bbcat-audioobjects, doxygen.cfg)

# Check if we should disable optimization  (./configure --disable-opt)
AC_MSG_CHECKING(whether to disable optimization)
AC_ARG_ENABLE(opt, AS_HELP_STRING([--disable-opt], [disable optimzation]), DISABLE_OPTIMIZATION="yes", DISABLE_OPTIMIZATION="no")
if test "x${DISABLE_OPTIMIZATION}" = "xyes"; then
  CFLAGS="$CFLAGS -O0"
  CXXFLAGS="$CXXFLAGS -O0"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

dnl bbcat-base is required
BBCAT_BASE_VER="0.1.1.0"
PKG_CHECK_MODULES(BBCAT_BASE, bbcat-base-0.1 >= $BBCAT_BASE_VER, HAVE_BBCAT_BASE=yes, HAVE_BBCAT_BASE=no)
if test "x${HAVE_BBCAT_BASE}" != xyes ; then
  AC_MSG_ERROR([bbcat-base-0.1 >= $BBCAT_BASE_VER is required])
fi

dnl bbcat-dsp is required
BBCAT_DSP_VER="0.1.1.0"
PKG_CHECK_MODULES(BBCAT_DSP, bbcat-dsp-0.1 >= $BBCAT_DSP_VER, HAVE_BBCAT_DSP=yes, HAVE_BBCAT_DSP=no)
if test "x${HAVE_BBCAT_DSP}" != xyes ; then
  AC_MSG_ERROR([bbcat-dsp-0.1 >= $BBCAT_DSP_VER is required])
fi

dnl bbcat-control is required
BBCAT_CONTROL_VER="0.1.1.0"
PKG_CHECK_MODULES(BBCAT_CONTROL, bbcat-control-0.1 >= $BBCAT_CONTROL_VER, HAVE_BBCAT_CONTROL=yes, HAVE_BBCAT_CONTROL=no)
if test "x${HAVE_BBCAT_CONTROL}" != xyes ; then
  AC_MSG_ERROR([bbcat-control-0.1 >= $BBCAT_CONTROL_VER is required])
fi

TINYXML_VER="2.6.2"
PKG_CHECK_MODULES(TINYXML, tinyxml >= $TINYXML_VER, HAVE_TINYXML=yes, HAVE_TINYXML=no)
if test "x${HAVE_TINYXML}" != xyes ; then
  AC_MSG_ERROR([tinyxml >= $TINYXML_VER is required])
fi

AC_ARG_WITH([thread],
    AS_HELP_STRING([--with-thread=IMPL],[use threading library {auto,pthread,gthread,win32,none} [[default=auto]]]),
    [],
    [with_thread=auto])

AC_MSG_CHECKING([threading implementation to use])
if test "x${with_thread}" = "xauto" ; then
  case $host_os in
    mingw* | pw32*)
      with_thread=win32
      ;;
    *)
      with_thread=pthread
      ;;
  esac
fi
AC_MSG_RESULT([$with_thread])

THREAD_IMPL=${with_thread}
AC_SUBST(THREAD_IMPL)

if test "x${with_thread}" = "xpthread" ; then
  case "$host_os" in
    *)
      PTHREAD_CFLAGS=
      PTHREAD_LIBS=-lpthread
      ;;
  esac
  AC_SUBST(PTHREAD_CFLAGS)
  AC_SUBST(PTHREAD_LIBS)
fi

if test "x${with_thread}" = "xgthread" ; then
  PKG_CHECK_MODULES(GTHREAD, gthread-2.0, HAVE_GTHREAD=yes, HAVE_GTHREAD=no)
  AC_SUBST(GTHREAD_LIBS)
  AC_SUBST(GTHREAD_CFLAGS)
fi

AC_APPEND_SUPPORTED_CFLAGS(BBCAT_AUDIOOBJECTS_CFLAGS, [-Wall])
AC_APPEND_SUPPORTED_CFLAGS(BBCAT_AUDIOOBJECTS_CFLAGS, [-Werror])
AC_APPEND_SUPPORTED_CFLAGS(BBCAT_AUDIOOBJECTS_CFLAGS, [-Wextra])
AC_APPEND_SUPPORTED_CFLAGS(BBCAT_AUDIOOBJECTS_CFLAGS, [-msse2])

BBCAT_AUDIOOBJECTS_CFLAGS="$BBCAT_AUDIOOBJECTS_CFLAGS -I\$(top_srcdir)"
AC_SUBST(BBCAT_AUDIOOBJECTS_CFLAGS)

BBCAT_AUDIOOBJECTS_LDFLAGS=""
AC_APPEND_SUPPORTED_LDFLAGS(BBCAT_AUDIOOBJECTS_LDFLAGS, [-Wl,--no-undefined])
AC_SUBST(BBCAT_AUDIOOBJECTS_LDFLAGS)

BBCAT_AUDIOOBJECTS_LIBS="\$(top_builddir)/src/libbbcat-audioobjects-$BBCAT_AUDIOOBJECTS_MAJORMINOR.la"
AC_SUBST(BBCAT_AUDIOOBJECTS_LIBS)

AC_SUBST(BBCAT_AUDIOOBJECTS_PKG_DEPS)

pkgconfigdir="\$(libdir)/pkgconfig"
AC_SUBST(pkgconfigdir)

bbcataudioobjectsbindir="\$(libdir)/bbcat-audioobjects/"
AC_SUBST(bbcataudioobjectsbindir)

AC_CONFIG_FILES([
Makefile
bbcat-audioobjects-uninstalled.pc
bbcat-audioobjects.pc
src/Makefile
])
AC_OUTPUT
