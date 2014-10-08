#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Common.h"
#include "Uart.h"
#include "UartLine.h"
#include "Portable.h"
#include "BthMod.h"

#define DEBUG_MODULE
#define DEBUG_LEVEL DBG_TRACE|DBG_WARN|DBG_ERROR 
#include "Debug.h"

#define MAX_LINE 256

typedef struct
{
    unsigned int uRate;
} BthModBaudEntry_T;

typedef struct 
{
    uhandle_t hUart;
    char Name[MAX_PATH];    
} BthMod_T;

const BthModBaudEntry_T gBthModBaudTable[] = 
{
    {UART_RATE_256000},
    {UART_RATE_128000},
    {UART_RATE_115200},
    {UART_RATE_57600},
    {UART_RATE_56000},
    {UART_RATE_38400},
    {UART_RATE_19200}, 
    {UART_RATE_14400},
    {UART_RATE_9600}, 
    {UART_RATE_1200}, 
};

/*****************************************************************************/

static
int 
BthModScan(
    IN BthMod_T *pMod
    );

static
int
BthModSendCommand(
    IN BthMod_T     *pMod,
    IN const char   *pCmd,
    OUT uint8_t     *pResponse,
    IN uint16_t     Len
    );

static
int 
BthModConfig(
    IN BthMod_T     *pMod
    );

static
int 
BthModFactoryReset(
    IN BthMod_T *pMod
    );

/*****************************************************************************/

int
BthModOpen(
    IN const char   *pPort,
    OUT uhandle_t   *phMod
    )
{
    int ret;
    BthMod_T *pMod = NULL;
    
    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    pMod = malloc(sizeof(BthMod_T));
    ret = pMod ? S_OK : E_NOMEMORY;
	CHECK_RETVAL(ret, ExitOnFailure);

    strncpy(pMod->Name, pPort, sizeof(pMod->Name));

    ret = UartCtor(&pMod->hUart);
	CHECK_RETVAL(ret, ExitOnFailure);

    ret = BthModScan(pMod);
	CHECK_RETVAL(ret, ExitOnFailure);
    
    if (ret == S_FALSE)
    {
        ret = BthModFactoryReset(pMod);
	    CHECK_RETVAL(ret, ExitOnFailure);

        ret = BthModScan(pMod);
	    CHECK_RETVAL(ret, ExitOnFailure);
    }

    ret = BthModConfig(pMod);
    CHECK_RETVAL(ret, ExitOnFailure);

    *phMod = (uhandle_t)pMod;
    pMod = NULL;

ExitOnFailure:

    free(pMod);

    return E_NOTIMPL;
}
 
void
BthModClose(
    IN uhandle_t    hMod
    )
{
    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

}
 
int
BthModConnect(
    IN uhandle_t    hMod,
    IN BthModBda_T *pBda
    )
{
    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    return E_NOTIMPL;
}
 
int
BthModListen(
    IN uhandle_t    hMod,
    IN uint32_t     uWait,
    OUT BthModBda_T *pBda
    )
{
    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    return E_NOTIMPL;
}
 
int
BthModDisconnect(
    IN uhandle_t    hMod
    )
{
    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    return E_NOTIMPL;
}
 
int
BthModSend(
    IN uhandle_t    hMod,
    IN uint16_t     uLen,
    IN uint8_t      *pData
    )
{
    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    return E_NOTIMPL;
}
 
int
BthModRecv(
    IN uhandle_t    hMod,
    IN uint16_t     uLen,
    IN uint8_t      *pData,
    OUT uint16_t    *puRecv
    )
{
    return E_NOTIMPL;
}
 
int
BthModBdaToStr(
    IN BthModBda_T  *pBda,
    IN char         *pStr,
    IN uint16_t     uLen
    )
{
    return E_NOTIMPL;
}
 
int
BthModStrToBda(
    IN const char   *pStr,
    IN BthModBda_T  *pBda
    )
{
    return E_NOTIMPL;
}

/*** Private methods *********************************************************/

int 
BthModScan(
    IN BthMod_T *pMod
    )
{
    int ret = E_FAIL;
    int i;
    char Buf[MAX_LINE];
    unsigned int BytesRead;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    for (i = 0; i < COUNTOF(gBthModBaudTable); i = i + 1)
    {
        DBG_MSG(DBG_TRACE, "Opening port %s, rate %d\n", pMod->Name, gBthModBaudTable[i].uRate);

        ret = UartOpen(pMod->hUart, 
                       pMod->Name, 
                       gBthModBaudTable[i].uRate,
                       UART_DATA_BITS_8,
                       UART_PARITY_NONE,
                       UART_STOP_1);
	    CHECK_RETVAL(ret, ExitOnFailure);

        /* Ensure we are flowed on */
        ret = UartSetStatus(pMod->hUart, UART_STATUS_SETRTS);
	    CHECK_RETVAL(ret, ExitOnFailure);

        ret = UartLineWrite2(pMod->hUart, "\n---\n", 100, 50);
	    CHECK_RETVAL(ret, ExitOnFailure);

        ret = UartPurge(pMod->hUart);
	    CHECK_RETVAL(ret, ExitOnFailure);

        ret = UartLineWrite(pMod->hUart, "$$$", 0);
	    CHECK_RETVAL(ret, ExitOnFailure);

        ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), 100);

        if (SUCCEEDED(ret))
        {
            ret = UartPurge(pMod->hUart);
	        CHECK_RETVAL(ret, ExitOnFailure);

            if (!strncmp(Buf, "CMD", 3))
            {
                if (i == 0)
                {
                    DBG_MSG(DBG_TRACE, "Module found standard rate\n");
                    ret = S_OK;
                }
                else
                {
                    DBG_MSG(DBG_TRACE, "Module found in odd rate\n");
                    ret = S_FALSE;
                }
                break;
            }
        }

        UartClose(pMod->hUart);
    }

ExitOnFailure:

    return ret;
}

int 
BthModFactoryReset(
    IN BthMod_T *pMod
    )
{
    int ret;
    uint8_t Response[256];

    ret = BthModSendCommand(pMod, "SF,1\n", Response, sizeof(Response));
    DBG_DUMP(DBG_TRACE, 0, Response, sizeof(Response));

    ret = BthModSendCommand(pMod, "R,1\n", Response, sizeof(Response));
    DBG_DUMP(DBG_TRACE, 0, Response, sizeof(Response));

    return ret;
}    

int
BthModSendCommand(
    IN BthMod_T     *pMod,
    IN const char   *pCmd,
    OUT uint8_t     *pResponse,
    IN uint16_t     Len
    )
{
    int ret;

    ret = UartLineWrite(pMod->hUart, pCmd, 0);
    CHECK_RETVAL(ret, ExitOnFailure);

    PortableSleep(250);
    memset(pResponse, 0, Len);

    ret = UartLineRead(pMod->hUart, (char *)pResponse, Len, 100);
    CHECK_RETVAL(ret, ExitOnFailure);

ExitOnFailure:

    return ret;
}

int 
BthModConfig(
    IN BthMod_T     *pMod
    )
{
    return E_NOTIMPL;
}
