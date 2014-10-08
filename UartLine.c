#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Portable.h"
#include "Common.h"
#include "Uart.h"
#include "UartLine.h"

#define DEBUG_MODULE 
//#define DEBUG_LEVEL DBG_ERROR|DBG_WARN|DBG_TRACE
#define DEBUG_LEVEL DBG_ERROR|DBG_WARN
#include "Debug.h"

static char gLineCache[512];
static int gLineIndex = 0;

int
UartLineCopy(
	IN char 		*pLine,
	IN int 			Size
	)
{
	int i;
	int Retval = E_FAIL;

	DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

	for (i = 0; i < gLineIndex; i = i + 1)
	{
		pLine[i] = gLineCache[i];

		if (gLineCache[i] == '\n')
		{
			i = i + 1;
			pLine[i] = '\0';
			gLineIndex = gLineIndex - i;
			memmove(&gLineCache[0], &gLineCache[i], gLineIndex);
			Retval = S_OK;
			break;
		}
	}

	if (FAILED(Retval))
	{
		pLine[0] = '\0';
	}

	return Retval;
}

int
UartLineRead(
	IN uhandle_t 	hUart,
    IN char 		*pLine,
    IN int 			Size,
	IN uint32_t 	MaxWait
	)
{
	int Retval;
	uint32_t Expire;
	unsigned int BytesRead;
	
	DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

	Expire = PortableGetTick() + MaxWait;

	for ( ; ; )
	{
		Retval = UartRead(hUart, 
				 &gLineCache[gLineIndex], 
				 sizeof(gLineCache) - gLineIndex, 
				 &BytesRead, 
				 100);

		if (SUCCEEDED(Retval) || Retval == E_TIMEOUT)
		{
            Retval = S_OK;

			if (gLineIndex || BytesRead)
			{
				gLineIndex = gLineIndex + BytesRead;

				Retval = UartLineCopy(pLine, Size);
				if (Retval == S_OK)
				{
					break;
				}
			}

			if (TIME_AFTER(PortableGetTick(), Expire))
			{
                if (gLineIndex)
                {
				    Retval = S_FALSE;
				    *pLine = '\0';
                }
                else
                {   
                    Retval = E_TIMEOUT;
                }
				break;
			}
		}

		CHECK_RETVAL(Retval, ExitOnFailure);
	}

ExitOnFailure:

	return Retval;
}

	
int
UartLineWrite(
	IN uhandle_t 	hUart,
    IN const char 	*pLine,
	IN uint32_t 	MaxWait
	)
{
	DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);
	return UartWrite(hUart, pLine, strlen(pLine), NULL, MaxWait);
}

int
UartLineWrite2(
	IN uhandle_t 	hUart,
    IN const char 	*pLine,
    IN uint32_t     InterCharacterDelay,
	IN uint32_t 	MaxWait
	)
{
    int ret = E_FAIL;
    int i;
    int Len;

	DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    if (InterCharacterDelay == 0)
    {
        ret = UartLineWrite(hUart, pLine, MaxWait);
        goto ExitOnFailure;
    }

    Len = strlen(pLine);
    
    for (i = 0; i < Len; i = i + 1 )
    {
	    ret = UartWrite(hUart, &pLine[i], 1, NULL, 0);
		CHECK_RETVAL(ret, ExitOnFailure);
        
        PortableSleep(InterCharacterDelay);
    }

ExitOnFailure:

    return ret;
}
