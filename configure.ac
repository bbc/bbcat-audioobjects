AC_PREREQ([2.68])
AC_INIT([APLIBS_AUDIOOBJECTS],[0.1.0.0])

AS_NANO(APLIBS_AUDIOOBJECTS_CVS=no,APLIBS_AUDIOOBJECTS_CVS=yes)

AM_INIT_AUTOMAKE([1.11 silent-rules])
AM_SILENT_RULES([yes])

# Version number that will be appended to the library filename
APLIBS_AUDIOOBJECTS_MAJORMINOR=0.1
AC_SUBST(APLIBS_AUDIOOBJECTS_MAJORMINOR)

AC_CONFIG_HEADERS([config.h])

# CURRENT, REVISION, AGE
# - library source changed but interface the same -> increment REVISION
# - interfaces added/removed/changed -> increment CURRENT, set REVISION = 0
# - interfaces added -> increment AGE also
# - interfaces removed -> set AGE = 0
#    (AGE is the number of previous values of CURRENT that are compatible)
APLIBS_AUDIOOBJECTS_LIBVERSION="1:0:0"
AC_SUBST(APLIBS_AUDIOOBJECTS_LIBVERSION)

LT_PREREQ([2.2.6])
LT_INIT(disable-static win32-dll)

AC_PROG_CXX
AM_PROG_CC_C_O

AC_CONFIG_SRCDIR([aplibs-audioobjects])

AC_CONFIG_MACRO_DIR([m4])
ACLOCAL_AMFLAGS="-I m4 $ACLOCAL_AMFLAGS"
AC_SUBST(ACLOCAL_AMFLAGS)

DX_DOXYGEN_FEATURE(ON)
DX_INIT_DOXYGEN(aplibs-audioobjects, doxygen.cfg)

# Check if we should enable debug support  (./configure --enable-debug)
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging
					   [default=$debug_default]],, enable_debug=$debug_default)
if test "x$enable_debug" = "xyes"; then
		 APLIBS_AUDIOOBJECTS_CFLAGS="$APLIBS_AUDIOOBJECTS_CFLAGS -g -O0"
		 CXXFLAGS="$CXXFLAGS -O0"
AC_MSG_RESULT(yes)
else
AC_MSG_RESULT(no)
fi

dnl aplibs-dsp is required
APLIBS_DSP_VER="0.1.1.0"
PKG_CHECK_MODULES(APLIBSDSP, aplibs-dsp-0.1 >= $APLIBS_DSP_VER, HAVE_APLIBS_DSP=yes, HAVE_APLIBS_DSP=no)
if test "x${HAVE_APLIBS_DSP}" != xyes ; then
  AC_MSG_ERROR([aplibs-dsp-1.2 >= $APLIBS_DSP_VER is required])
fi

dnl tinyxml is required
TINYXML_VER="2.6.2"
PKG_CHECK_MODULES(TINYXML, tinyxml >= $TINYXML_VER, HAVE_TINYXML=yes, HAVE_TINYXML=no)
if test "x${HAVE_TINYXML}" != xyes ; then
  AC_MSG_ERROR([tinyxml >= $APLIBS_DSP_VER is required])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AX_CREATE_STDINT_H([aplibs-audioobjects/aplibs-audioobjects-stdint.h])

AC_APPEND_SUPPORTED_CFLAGS(APLIBS_AUDIOOBJECTS_CFLAGS, [-Wall])
AC_APPEND_SUPPORTED_CFLAGS(APLIBS_AUDIOOBJECTS_CFLAGS, [-Werror])
AC_APPEND_SUPPORTED_CFLAGS(APLIBS_AUDIOOBJECTS_CFLAGS, [-Wextra])

APLIBS_AUDIOOBJECTS_CFLAGS="$APLIBS_AUDIOOBJECTS_CFLAGS -I\$(top_srcdir)"
AC_SUBST(APLIBS_AUDIOOBJECTS_CFLAGS)

APLIBS_AUDIOOBJECTS_LDFLAGS=""
AC_APPEND_SUPPORTED_LDFLAGS(APLIBS_AUDIOOBJECTS_LDFLAGS, [-Wl,--no-undefined])
AC_SUBST(APLIBS_AUDIOOBJECTS_LDFLAGS)

APLIBS_AUDIOOBJECTS_LIBS="\$(top_builddir)/aplibs-audioobjects/libap-audioobjects-$APLIBS_AUDIOOBJECTS_MAJORMINOR.la"
AC_SUBST(APLIBS_AUDIOOBJECTS_LIBS)

AC_SUBST(APLIBS_AUDIOOBJECTS_PKG_DEPS)

pkgconfigdir="\$(libdir)/pkgconfig"
AC_SUBST(pkgconfigdir)

aplibsaudioobjectsbindir="\$(libdir)/aplibs-audioobjects/"
AC_SUBST(aplibsaudioobjectsbindir)

AC_CONFIG_FILES([
Makefile
aplibs-audioobjects-uninstalled.pc
aplibs-audioobjects.pc
aplibs-audioobjects/Makefile
])
AC_OUTPUT
