#define _CRT_SECURE_NO_DEPRECATE 
#include <stdio.h>
#include <string.h>
#include "uInterp.h"

#ifdef WINDOWS
#pragma warning(disable: 4127)
#endif

/******************************************************************************/

#define UINTERP_ESCAPE      '\\'
#define UINTERP_MAX_DIGIT   16
#define UINTERP_MAX_INT8    0xFF    
#define UINTERP_MAX_INT16   0xFFFF 
#define UINTERP_MAX_INT32   0xFFFFFFFF 

#define SUCCEEDED(hr)       ((hr) >= 0)
#define FAILED(hr)          ((hr) < 0)

#define CHECK_RETVAL(hr, label)\
        do {\
            if (FAILED(hr))\
            {\
                goto label;\
            }\
        } while(0)

/******************************************************************************/

typedef struct Interp_T
{
    const uInterpCmd_T		*pExtCmds;              /* Enternal command table                   */
    unsigned int            uCmdCount;              /* Number of commands in the table          */
    char                    *pStart;                /* Start of command less white spaces       */
} uInterp_T;

typedef enum 
{
    kSkipWhite,    
    kInQuote,      
    kInWord,       
    kInSlash,      
    kInSlashQuote 
} EParseStates_T;

/******************************************************************************/

static
void
uInterpTrimTrailingChars(
    char            *pString,
    const char      *pChars
    );

static
void
uInterpTrimLeadingChars(
    char            *pString,
    const char      *pChars,
    char            **ppPtr
    );

static
int
uInterpFindCommand(
    const uInterpCmd_T  *pExtCmds,
    unsigned int        uCmdCount,
    const char          *pStart,
    const uInterpCmd_T  **ppCmds
    );

static
unsigned int 
uInterpGetArgs(
    unsigned int        uIndex,     
    char                *pBuff,     
    unsigned int        uLen        
    );

static
void
uInterpAddSet(
    char                *pBuff, 
    unsigned int        uLen, 
    char                c, 
    unsigned int        uCount,
    unsigned int        *puCount,
    unsigned int        *puIndex
    );

static
unsigned long 
uInterpStringToLong(
    const char      *pString,
    char  const     **pNext,
    int             Radix
    );

static
int
uInterpCharToDecimal(
    char    c,
    char    *pDec
    );

static
int
uInterpCharToHex(
    char    c,
    char   *pHex
    );

/******************************************************************************/

static uInterp_T guInterp;

/******************************************************************************/

/* Initialize the interpreter */
int
uInterpCtor(
    const uInterpCmd_T	    *pExtCmds,
    unsigned int            uCmdCount
    )
{
    guInterp.pExtCmds = pExtCmds;
    guInterp.uCmdCount = uCmdCount;

    return UINTERP_OK;
}

/* Release any resources used by interpreter */
void 
uInterpDtor(
    void
    )
{
    memset(&guInterp, 0, sizeof(uInterp_T));
}

/* 
 * Execute the command.  the line passe in is saved to allow for 
 * additional calls to parse the line into various arguments. 
 */
int
uInterpExecute(
    char 	                *pLine
    )
{
    int Retval = UINTERP_EFAIL;
    const uInterpCmd_T *pCmd;

    /* Remove tailing white space */
    uInterpTrimTrailingChars(pLine, " \b\t\r\n");
        
    /* Skip leading white space */
    uInterpTrimLeadingChars(pLine, " \b\t\r\n", &guInterp.pStart);
        
    /* Only process non empty lines */
    if (strlen(guInterp.pStart))
    {
        /* Locate a matching command */
        Retval = uInterpFindCommand(guInterp.pExtCmds, 
                                    guInterp.uCmdCount, 
                                    guInterp.pStart, 
                                    &pCmd);
        CHECK_RETVAL(Retval, ExitOnFailure);

        /* Call the command handler */
        Retval = pCmd->pfCmd();
    }

ExitOnFailure:

    return Retval;	
}

/* Return the number of arguments */
int 
uInterpNumArgs(
    void
    )
{
    return uInterpGetArgs(0, NULL, 0) + 1;
}

/* Return the specified command argument, and coerce into the specified type */
int
uInterpArgs(
    unsigned int            Number,
    unsigned int            eArgType,
    void                    *pBuff,
    unsigned int            Length
    )
{
    int Retval = UINTERP_EFAIL;
    char nBuffer[UINTERP_MAX_DIGIT];
    unsigned long Value = 0;

    switch(eArgType)
    {
    /* Return byte argument */    
    case UINTERP_UINT8:
        Retval = pBuff && Length == sizeof(char) ? UINTERP_OK : UINTERP_EARG;
        CHECK_RETVAL(Retval, ExitOnFailure);
        
        Retval = Number == uInterpGetArgs(Number, nBuffer, sizeof(nBuffer)) ? UINTERP_OK : UINTERP_EFAIL;
        CHECK_RETVAL(Retval, ExitOnFailure);
                    
        Value = uInterpStringToLong(nBuffer, NULL, 0);
        
        Retval = (Value <= UINTERP_MAX_INT8) ? UINTERP_OK : UINTERP_ERANGE;
        CHECK_RETVAL(Retval, ExitOnFailure);
                    
        *(char *)pBuff = (char)Value;
        break;

    /* Return word argument */    
    case UINTERP_UINT16:
        Retval = pBuff && Length == sizeof(unsigned short) ? UINTERP_OK : UINTERP_EARG;
        CHECK_RETVAL(Retval, ExitOnFailure);

        Retval = Number == uInterpGetArgs(Number, nBuffer, sizeof(nBuffer)) ? UINTERP_OK : UINTERP_EFAIL;
        CHECK_RETVAL(Retval, ExitOnFailure);
                    
        Value = uInterpStringToLong(nBuffer, NULL, 0);
        
        Retval = (Value <= UINTERP_MAX_INT16) ? UINTERP_OK : UINTERP_ERANGE;
        CHECK_RETVAL(Retval, ExitOnFailure);
                    
        *(unsigned short *)pBuff = (unsigned short)Value;
        break;
        
    /* Return long argument */    
    case UINTERP_UINT32:
        Retval = pBuff && Length == sizeof(unsigned long) ? UINTERP_OK : UINTERP_EARG;
        CHECK_RETVAL(Retval, ExitOnFailure);
        
        Retval = Number == uInterpGetArgs(Number, nBuffer, sizeof(nBuffer)) ? UINTERP_OK : UINTERP_EFAIL;
        CHECK_RETVAL(Retval, ExitOnFailure);
                    
        Value = uInterpStringToLong(nBuffer, NULL, 0);
        
        Retval = (Value <= UINTERP_MAX_INT32) ? UINTERP_OK : UINTERP_ERANGE;
        CHECK_RETVAL(Retval, ExitOnFailure);
                    
        *(unsigned long *)pBuff = (unsigned long)Value;
        break;     

    /* Return string argument */    
    case UINTERP_STR:
        Retval = pBuff && Length ? UINTERP_OK : UINTERP_EARG;
        CHECK_RETVAL(Retval, ExitOnFailure);

        *(char *)pBuff = '\0';

        Retval = Number == uInterpGetArgs(Number, pBuff, Length) ? UINTERP_OK : UINTERP_EFAIL;
        CHECK_RETVAL(Retval, ExitOnFailure);
        break;
 
    /* Return raw input line to the caller */
    case UINTERP_RAW:
        Retval = pBuff && Length ? UINTERP_OK : UINTERP_EARG;
        CHECK_RETVAL(Retval, ExitOnFailure);
        strncpy(pBuff, guInterp.pStart, Length);
        break;
              
    default:
        Retval = UINTERP_EARG;
        break;
    }

ExitOnFailure:
                    
    return Retval;
}

/******************************************************************************/

/* Trim any trailing characters */
void
uInterpTrimTrailingChars(
    char            *pString,
    const char      *pChars
    )
{
    char *p;

    for (p = pString + strlen(pString) - 1; p >= pString; p = p - 1)
    {
        if (!strchr(pChars, *p))
        {
            break;
        }    
        
        *p = '\0';
    }
}

/* Trims leading characters, returns new string pointer */    
void
uInterpTrimLeadingChars(
    char            *pString,
    const char      *pChars,
    char            **ppPtr
    )
{
    for ( ; *pString; pString = pString + 1)
    {
        if (!strchr(pChars, *pString))
        {
            break;
        }    
    }
    *ppPtr = pString;
}    

/* Attempt to find a command */
int
uInterpFindCommand(
    const uInterpCmd_T  *pExtCmds,
    unsigned int        uCmdCount,
    const char          *pStart,
    const uInterpCmd_T  **ppCmds
    )
{
    int Retval = UINTERP_ECOMMAND;
    int Length = 0;
    int i;
    const char *p;

    /* Look for the command it must end with a white space */
    for (p = pStart; *p; p++)
    {
        if (*p == ' ' || *p == '\t')  
        {
            break;
        }

        Length = Length + 1;
    }

    /* Look for a matching command */
    for (i = 0; i < (int)uCmdCount; i = i + 1)
    {
        if (!strncmp(pExtCmds[i].pName, pStart, Length))
        {        
            *ppCmds = &pExtCmds[i];
            Retval = UINTERP_OK;
            break;
        }
    }

    return Retval;
}

/* 
 * Convert a string of numbers to an unsigned long 32 bit value 
 * 
 * Known issues:
 * 1. Over flow checking / detection is not done
 * 2. Negative values are not supported
 * 3. Only radix 10 and radix 16 are supported 
 *
 */
unsigned long
uInterpStringToLong(
    const char      *pString,
    char  const     **ppNext,
    int             Radix
    )
{
    unsigned long Value = 0;
    char b = 0;
    const char *p;

    /* Auto detect the radix if zero */
    if (!Radix && pString[0] && pString[1])
    {
        if (pString[0] == '0' && (pString[1] == 'x' || pString[1] == 'X'))
        {
            pString = pString + 2;
            Radix = 16;
        }     
        else
        {
            Radix = 10;
        }       
    }
    else
    {
        Radix = 10;
    }

    for (p = pString; *p; )
    {
        if (Radix == 10)
        {                                
            if (uInterpCharToDecimal(*p, &b) != UINTERP_OK)
            {
                break;
            }
        }
        else if (Radix == 16)
        {
            if (uInterpCharToHex(*p, &b) != UINTERP_OK)
            {
                break;
            }        
        }     
                    
        Value = Value * Radix + b;

        p = p + 1;
    }

    if (ppNext)
    {
        *ppNext = p;
    }

    return Value;
}                                    

/* 
 * Add character to the end of buffer ensuring there is space and the 
 * buffer is large enough for the next character.  The buffer is always
 * null terminated, to simplify the callers code path. 
 * 
 * NOTE:  This is an internal routine, minimal error checking is done to 
 * keep it reasonably fast.
 */
void
uInterpAddSet(
    char                *pBuff, 
    unsigned int        uLen, 
    char                c, 
    unsigned int        uCount,
    unsigned int        *puCount,
    unsigned int        *puIndex
    )
{
    /* Have we reached the argument count */
    if (pBuff && uCount == *puCount)
    {
        if (*puIndex < (uLen - 1))
        {
            pBuff[*puIndex] = c;    
            *puIndex = *puIndex + 1;
            pBuff[*puIndex] = '\0';    
        }        
    }

    /* End of argument found, update count */
    else 
    {
        if (c == '\0')
        {
            *puCount = *puCount + 1;
        }
    }
} 
   
/*
 * This routine breaks the line up based on spaces and the uses of 
 * quotes.  A quote can be escaped using a slash.  When pBuff is 
 * null this routine will parse the entire line counting the number 
 * of arguments.
 *
 * NOTE:  This is an internal routine, minimal error checking is done to 
 * keep it reasonably fast.
 */
unsigned int
uInterpGetArgs(
    unsigned int    uIndex,     /* argument number to look for */
    char            *pBuff,     /* buffer where to return found argument */
    unsigned int    uLen        /* length in bytes of the buffer  */
    )
{
    char c = '\0';
    const char *p = guInterp.pStart;
    int state = kSkipWhite;
    unsigned int next = 0;
    unsigned int ret = 0;
    unsigned int last = 0;

    for ( ; *p; p = p + next)
    {
        c = *p;

        switch(state)
        {
        case kSkipWhite:
            if (c == ' ')
            {
                state = kSkipWhite;
                next = 1;
            }
            else 
            {
                state = kInWord;
                next = 0;
            }
            break;

        case kInWord:
            if (c == '"')
            {
                state = kInQuote;
            }
            else if (c == '\\')
            {
                state = kInSlash;
            }
            else if (c == ' ')
            {
                state = kSkipWhite;
                uInterpAddSet(pBuff, uLen, '\0', uIndex, &ret, &last);
            }
            else 
            {
                state = kInWord;
                uInterpAddSet(pBuff, uLen, c, uIndex, &ret, &last);
            }
            next = 1;
            break;

        case kInQuote:
            if (c == '"')
            {
                state = kInWord;
            }
            else if (c == '\\')
            {
                state = kInSlashQuote;
            }
            else 
            {
                state = kInQuote;
                uInterpAddSet(pBuff, uLen, c, uIndex, &ret, &last);
            }
            next = 1;
            break;

        case kInSlash:
            if (c == '"')
            {
                uInterpAddSet(pBuff, uLen, '"', uIndex, &ret, &last);
            }
            else if (c == '\\')
            {
                uInterpAddSet(pBuff, uLen, '\\', uIndex, &ret, &last);
            }                
            else
            { 
                uInterpAddSet(pBuff, uLen, c, uIndex, &ret, &last);
            }
            state = kInWord;
            next = 1;
            break;

        case kInSlashQuote:
            if (c == '"')
            {
                uInterpAddSet(pBuff, uLen, '"', uIndex, &ret, &last);
            }
            else
            {
                uInterpAddSet(pBuff, uLen, '\\', uIndex, &ret, &last);
                uInterpAddSet(pBuff, uLen, c, uIndex, &ret, &last);
            }                
            state = kInQuote;
            next = 1;
            break;

        default:
            break;
        }
    }

    return ret;
}

/* Convert a character to a hex digit */
int
uInterpCharToHex(
    char    c,
    char   *pHex
    )
{
    int Retval = UINTERP_OK;

    if ((c >= '0') && (c <= '9') )
    {
        *pHex = c - '0';
    }
    else if ((c >= 'A') && (c <= 'F'))
    {
        *pHex = c - 'A' + 10;
    }
    else if ((c >= 'a') && (c <= 'f'))
    {
        *pHex = c - 'a' + 10;
    }
    else
    {
        Retval = UINTERP_EFAIL;
    }
            
    return Retval;
}

/* Convert a character to a decimal digit */
int
uInterpCharToDecimal(
    char    c,
    char    *pDec
    )
{
    int Retval = UINTERP_OK;

    if ((c >= '0') && (c <= '9') )
    {
        *pDec = c - '0';
    }
    else
    {
        Retval = UINTERP_EFAIL;
    }
            
    return Retval;
}

int
uInterpStrToBin(
    const char              *pStr,
    unsigned char           *pBuf,
    int                     Len,
    int                     *pLen
    )
{
    int Retval;
    const char *pNext = NULL;
    unsigned long val = 0;
    int i;

    Retval = pStr && pBuf && Len ? UINTERP_OK : UINTERP_EFAIL;
    CHECK_RETVAL(Retval, ExitOnFailure);

    if (pLen)
    {
        *pLen = 0;
    }

    i = 0;

    for ( ; *pStr; )
    {
        /* Skip tokens, treat as white space */
        if (*pStr == '{' || *pStr == '}' || *pStr == ',' || *pStr == ' ' || *pStr == '\t')
        {
            pStr = pStr + 1;
            continue;
        }

        val = uInterpStringToLong(pStr, &pNext, 0);

        /* Did we convert any characters */
        Retval = pStr != pNext ? UINTERP_OK : UINTERP_EFAIL;
        CHECK_RETVAL(Retval, ExitOnFailure);

        /* Are we in byte range */
        Retval = (val <= 255) ? UINTERP_OK : UINTERP_EFAIL;
        CHECK_RETVAL(Retval, ExitOnFailure);

        /* Is there room in the binary buffer */
        Retval = (i < Len) ? UINTERP_OK : UINTERP_EFAIL;
        CHECK_RETVAL(Retval, ExitOnFailure);

        pBuf[i] = (char)val;
        i = i + 1;
        pStr = pNext;
        if (pLen)
        {
            *pLen = *pLen + 1;
        }
    }

ExitOnFailure:

    return Retval;
}

