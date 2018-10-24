/* @(#) $Revision: ../hdr/strings.h@@/main/r11ros/0 $ */
#ifndef _STRINGS_INCLUDED
#define _STRINGS_INCLUDED

/*
 * This header file is for BSD applications importability which expects
 * a header file with this name to include all the string functions and
 * types.
 */

#include <sys/stdsyms.h>
#include <stdlib.h>
#if defined(_INCLUDE_HPUX_SOURCE) || !defined(_XPG4_EXTENDED)
   /* string.h is not included for CASPEC */
/*#  include <string.h>*/
#endif /* _INCLUDE_HPUX_SOURCE || !_XPG4_EXTENDED  */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _INCLUDE_XOPEN_SOURCE_EXTENDED

#ifndef _SYS_TYPES_INCLUDED
#       include <sys/types.h>
#endif /* _SYS_TYPES_INCLUDED */

#  if defined(__STDC__) || defined(__cplusplus)
     extern char *index(const char *, int);
     extern char *rindex(const char *, int);
/*     extern void bcopy(const void *, void *, size_t);
     extern int bcmp(const void *, const void *, size_t);
     extern void bzero(void *, size_t);  */

	/* For !_XPG4_EXTENDED the declarations for strcasecmp
           and strncasecmp are in <string.h> */
#     ifdef _XPG4_EXTENDED
			extern int strcasecmp(const char *, const char *);
			extern int strncasecmp(const char *, const char *, size_t);
#     endif /* _XPG4_EXTENDED */
     extern int ffs(int);
#  else /* __STDC__ || __cplusplus */
     extern char *index();
     extern char *rindex();
     extern void bcopy();
     extern int bcmp();
     extern void bzero();
     extern int ffs();
#  endif /* __STDC__ || __cplusplus */
#endif /* _INCLUDE_XOPEN_SOURCE_EXTENDED */

#ifdef __cplusplus
}
#endif

#endif /* _STRINGS_INCLUDED */
