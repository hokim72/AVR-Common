// Type-defines required and used by AVRlib. Most types are also generally
// useful.

#ifndef AVRLIBTYPES_H
#define AVRLIBTYPES_H

#ifndef WIN32
	// true/false defines
	#define FALSE	0
	#define	TRUE	-1
#endif

// maximum value that can be held
// by unsigned data types (8,16,32bits)
#define	MAX_U08	255
#define	MAX_U16	65535
#define MAX_U32 4294967295

// maximum values that can be held
// by signed data types (8, 16, 32bits)
#define MIN_S08 -128
#define MAX_S08 127
#define MIN_S16 -32768
#define MAX_S16 32767
#define MIN_S32 -2147483648
#define MAX_S32 2147483647

#if 0
#ifndef WIN32
    // more type redefinitions
    typedef unsigned char   BOOL;
    typedef unsigned char   BYTE;
    typedef unsigned int    WORD;
    typedef unsigned long   DWORD;

    typedef unsigned char   UCHAR;
    typedef unsigned int    UINT;
    typedef unsigned short  USHORT;
    typedef unsigned long   ULONG;

    typedef char            CHAR;
    typedef int             INT;
    typedef long            LONG;
#endif
#endif

#endif

