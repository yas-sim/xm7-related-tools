#ifndef _SYSDEF_H_
#define _SYSDEF_H_

#define VERBOSE	(1)

#if VERBOSE==0
#define SHOW_ERROR
#else
#define SHOW_ERROR		{ \
							char pErrString[256]; \
							FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, pErrString, 255, 0); \
							puts(pErrString); \
						}
#endif

typedef unsigned short FDC_STATUS;

#define FDC_WRITE_PROTECTED		(1)

#endif // _SYSDEF_H_
