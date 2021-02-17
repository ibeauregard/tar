#if defined(custom_makedev) && defined(custom_major) && defined(custom_minor)
/* Already defined */
#else

#undef custom_makedev
#undef custom_major
#undef custom_minor

#if defined(__linux__) || defined(__GLIBC__)
/* Linux, Android, and other systems using GNU C library */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif
#include <sys/types.h>
#define custom_makedev(dmajor, dminor) makedev(dmajor, dminor)
#define custom_major(devnum)           major(devnum)
#define custom_minor(devnum)           minor(devnum)

#elif defined(_WIN32)
/* 32- and 64-bit Windows. VERIFY: These are just a guess! */
#define custom_makedev(dmajor, dminor) ((((unsigned int)dmajor << 8) & 0xFF00U) | ((unsigned int)dminor & 0xFFFF00FFU))
#define custom_major(devnum)           (((unsigned int)devnum & 0xFF00U) >> 8)
#define custom_minor(devnum)           ((unsigned int)devnum & 0xFFFF00FFU)

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
/* FreeBSD, OpenBSD, NetBSD, and DragonFlyBSD */
#include <sys/types.h>
#define custom_makedev(dmajor, dminor) makedev(dmajor, dminor)
#define custom_major(devnum)           major(devnum)
#define custom_minor(devnum)           minor(devnum)

#elif defined(__APPLE__) && defined(__MACH__)
/* Mac OS X */
#include <sys/types.h>
#define custom_makedev(dmajor, dminor) makedev(dmajor, dminor)
#define custom_major(devnum)           major(devnum)
#define custom_minor(devnum)           minor(devnum)

#elif defined(_AIX) || defined (__osf__)
/* AIX, OSF/1, Tru64 Unix */
#include <sys/types.h>
#define custom_makedev(dmajor, dminor) makedev(dmajor, dminor)
#define custom_major(devnum)           major(devnum)
#define custom_minor(devnum)           minor(devnum)

#elif defined(hpux)
/* HP-UX */
#include <sys/sysmacros.h>
#define custom_makedev(dmajor, dminor) makedev(dmajor, dminor)
#define custom_major(devnum)           major(devnum)
#define custom_minor(devnum)           minor(devnum)

#elif defined(sun)
/* Solaris */
#include <sys/types.h>
#include <sys/mkdev.h>
#define custom_makedev(dmajor, dminor) makedev(dmajor, dminor)
#define custom_major(devnum)           major(devnum)
#define custom_minor(devnum)           minor(devnum)

#else
/* Unknown OS. Try a the BSD approach. */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif
#include <sys/types.h>
#if defined(makedev) && defined(major) && defined(minor)
#define custom_makedev(dmajor, dminor) makedev(dmajor, dminor)
#define custom_major(devnum)           major(devnum)
#define custom_minor(devnum)           minor(devnum)
#endif
#endif

#if !defined(custom_makedev) || !defined(custom_major) || !defined(custom_minor)
#error Unknown OS: please add definitions for custom_makedev(), custom_major(), and custom_minor(), for device number major/minor handling.
#endif

#endif
