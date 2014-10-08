#ifndef COMMON_H
#define COMMON_H

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

/* Returnes the countof items in an array NOT size in bytes */
#if !defined(COUNTOF)
#define COUNTOF(a) (sizeof(a)/sizeof(*a))
#endif

#define IN          /* Parameter is an input                */
#define IO          /* Parameter is both input and output   */
#define OUT         /* Parameter is strictly an output      */
#define ORPHAN      /* Parameter is orphaned to the caller  */
#define ADOPT       /* Callee adopts the parameter          */
#define OPTIONAL    /* The out parameter is optional        */

/* Useful 16 bit value macros */
#define WORD_LOW_HALF(w)        (((uint16_t)(w) >> 0)& 0x00FF)
#define WORD_HIGH_HALF(w)       (((uint16_t)(w) >> 8) & 0x00FF)
#define WORD_MAKE(msb,lsb)      (((uint16_t)(msb) << 8) |\
                                 ((uint16_t)(lsb) << 0))

/* Useful 32 bit value macros */
#define DWORD_LOW_HALF(l)       (((uint32_t)(l) >>  0) & 0x0000FFFF) 
#define DWORD_HIGH_HALF(l)      (((uint32_t)(l) >> 16) & 0x0000FFFF)
#define DWORD_MAKE(b1, b2, b3, b4) (((uint32_t)(b1) << 24) |\
                                    ((uint32_t)(b2) << 16) |\
                                    ((uint32_t)(b3) <<  8) |\
                                    ((uint32_t)(b4) << 0))

/* 16 bit byte swap */
#if !defined (HTONS)

#define HTONS(v)\
         ((((v) >> 8) & 0x00FF) |\
          (((v) <<  8) & 0xFF00))

#endif

/* 32 bit byte swap */
#if !defined (HTONL)

#define HTONL(v)\
         ((((v) >> 24) & 0x000000FF) |\
          (((v) >>  8) & 0x0000FF00) |\
          (((v) <<  8) & 0x00FF0000) |\
          (((v) << 24) & 0xFF000000))

#endif

#if !defined(_STDINT_H_)

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef signed char     int8_t;
typedef short           int16_t;
typedef int             int32_t;

#endif

typedef unsigned long   uhandle_t;

/* 
 * Time releated macros, the tick parameter is the system tick and the
 * timeout parameter is the expiration timeout i.e. the tick plus some
 * time to wait.
 *
 * TIME_AFTER(a,b) returns true if tick is after timeout.
 */
#define TIME_AFTER(tick, timeout)     ((int32_t)(timeout) - (int32_t)(tick) < 0)
#define TIME_BEFORE(tick, timeout)    TIME_AFTER(timeout, tick)
#define TIME_AFTER_EQ(tick, timeout)  ((int32_t)(tick) - (int32_t)(timeout) >= 0)
#define TIME_BEFORE_EQ(tick, timeout) TIME_AFTER_EQ(timeout, tick);

/******************************************************************************/

#if defined (OSX)

#define S_OK            0
#define S_FALSE         1
#define S_SECTIONEND    2
#define S_COMMENT       3
#define S_BLANK         4

#define E_FAIL          -1
#define E_NOTIMPL       -2
#define E_NOMEMORY      -3
#define E_ENDOFFILE     -4
#define E_RANGE         -5
#define E_INVALIDARG    -6
#define E_FILENOTFOUND  -7
#define E_MATCH         -8
#define E_BADCMDARG     -9
#define E_NOBUFFSPACE   -10
#define E_DUPLICATE_SEC -11
#define E_DUPLICATE_KEY -12
#define E_NOTFOUND      -13
#define E_TIMEOUT       -14
#define E_PROTOCOL      -15
#define E_DISCONNECT    -16
#define E_POINTER       -17

#define SUCCEEDED(ret)  ((ret) >= 0)
#define FAILED(ret)     ((ret) < 0)

#endif

#if defined(WINDOWS)
#include <Windows.h>
#define E_NOMEMORY	    -3
#define E_TIMEOUT 	    -14
#endif

#define CHECK_RETVAL(ret, label)\
    do {\
        if (FAILED(ret))\
        {\
            DBG_MSG(DBG_WARN, "Retval: 0x%08X or %d\n", ret, ret);\
            goto label;\
        }\
    } while (0)

#define CHECK_RETVAL_SAFE(ret, safe, label)\
    do {\
        if (FAILED(ret))\
        {\
            if ((ret) != (safe))\
            {\
                DBG_MSG(DBG_WARN, "Retval: 0x%08X or %d\n", ret, ret);\
            }\
            goto label;\
        }\
    } while (0)

#endif

