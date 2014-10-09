#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "Common.h"
#include "Uart.h"
#include "UartLine.h"
#include "Portable.h"
#include "BthMod.h"

#define DEBUG_MODULE
#define DEBUG_LEVEL DBG_TRACE|DBG_WARN|DBG_ERROR 
#include "Debug.h"

#define MAX_LINE        256
#define MAX_RESPONSE    256

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
    {UART_RATE_115200},
    {UART_RATE_256000},
    {UART_RATE_128000},
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

int
BthModSendCommand(
    IN BthMod_T     *pMod,
    IN const char   *pCmd,
    OUT int         *pCmdResult,
    OUT char        *pResponse,
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

static
int
BthModSync(
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

    /* Ensure we are in command mode sync, echo is off */
    ret = BthModSync(pMod);
    CHECK_RETVAL(ret, ExitOnFailure);
    
    if (ret == S_FALSE)
    {
        ret = BthModFactoryReset(pMod);
	    CHECK_RETVAL(ret, ExitOnFailure);

        ret = BthModScan(pMod);
	    CHECK_RETVAL(ret, ExitOnFailure);
    
        if (ret == S_FALSE)
        {
            DBG_MSG(DBG_TRACE, "Still not at standard baud rate\n");
            ret = E_FAIL;
    	    CHECK_RETVAL(ret, ExitOnFailure);
        }
    }

    /* Configure the module the way we want it */
    ret = BthModConfig(pMod);
    CHECK_RETVAL(ret, ExitOnFailure);

    *phMod = (uhandle_t)pMod;
    pMod = NULL;

ExitOnFailure:

    free(pMod);

    return ret;
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
    int ret;
    char Buf[256];
    BthMod_T *pMod = (BthMod_T *)hMod;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), uWait);
    CHECK_RETVAL(ret, ExitOnFailure);

    DBG_MSG(DBG_TRACE, "Message: %s\n", Buf);

ExitOnFailure:

    return ret;
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

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    for (i = 0; i < COUNTOF(gBthModBaudTable); i = i + 1)
    {
        DBG_MSG(DBG_TRACE, "Scan: port %s, rate %d\n", pMod->Name, gBthModBaudTable[i].uRate);

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

        /* Attempt to exit command mode, we need to be in a known state */
        ret = UartLineWrite2(pMod->hUart, "\n---\n", 100, 50);
	    CHECK_RETVAL(ret, ExitOnFailure);

        /* Dump the read data, we do not chare what was returned */
        ret = UartPurge(pMod->hUart);
	    CHECK_RETVAL(ret, ExitOnFailure);

        /* Attempt to enter command mode */
        ret = UartLineWrite2(pMod->hUart, "$$$", 100, 50);
	    CHECK_RETVAL(ret, ExitOnFailure);

        /* Read the response, we should see the cmd prompt */
        ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), 100);
        if (FAILED(ret) && (ret != E_TIMEOUT))
        {
            CHECK_RETVAL(ret, ExitOnFailure);
        }

        /* Have we entered command mode */
        if (SUCCEEDED(ret) && !strncmp(Buf, "CMD", 3))
        {
            ret = (i == 0) ? S_OK : S_FALSE;
            break;
        }            

        UartClose(pMod->hUart);
    }

ExitOnFailure:

    return ret;
}

int
BthModSync(
    IN BthMod_T *pMod
    )
{
    int ret;
    char Buf[256];

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    ret = UartLineWrite(pMod->hUart, "\n", 100);
    CHECK_RETVAL(ret, ExitOnFailure);

    PortableSleep(100);

    ret = UartLineWrite(pMod->hUart, "\n", 100);
    CHECK_RETVAL(ret, ExitOnFailure);

    PortableSleep(100);

    ret = UartPurge(pMod->hUart);
    CHECK_RETVAL(ret, ExitOnFailure);

    ret = UartLineWrite(pMod->hUart, "\n", 100);
    CHECK_RETVAL(ret, ExitOnFailure);

    ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), 100);
    CHECK_RETVAL(ret, ExitOnFailure);

    ret = (Buf[0] == '?') ? S_OK : E_FAIL;
    CHECK_RETVAL(ret, ExitOnFailure);

    /* Check the echo status */    
    ret = UartLineWrite(pMod->hUart, "+\n", 100);
    CHECK_RETVAL(ret, ExitOnFailure);

    ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), 100);
    CHECK_RETVAL(ret, ExitOnFailure);

    /* Check the echo status */    
    ret = (strncmp(Buf, "ECHO OFF", 8) ||
           strncmp(Buf, "ECHO ON", 7)) ? S_OK : E_FAIL;
    CHECK_RETVAL(ret, ExitOnFailure);

    /* If echo is on we must turn it off */    
    if (!strncmp(Buf, "ECHO ON", 7))
    {
        ret = UartLineWrite(pMod->hUart, "+\n", 100);
        CHECK_RETVAL(ret, ExitOnFailure);

        ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), 100);
        CHECK_RETVAL(ret, ExitOnFailure);

        ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), 100);
        CHECK_RETVAL(ret, ExitOnFailure);

        ret = !strncmp(Buf, "ECHO OFF", 8) ? S_OK : E_FAIL;
        CHECK_RETVAL(ret, ExitOnFailure);
    }
    
ExitOnFailure:
    
    return ret;
}

int
BthModSendCommand(
    IN BthMod_T     *pMod,
    IN const char   *pCmd,
    OUT int         *pCmdResult,
    OUT char        *pResponse,
    IN uint16_t     Len
    )
{
    int ret;
    char Buf[256];

    if (pResponse)
    {
        memset(pResponse, 0, Len);
    }
    
    if (pCmdResult)
    {
        *pCmdResult = E_FAIL;
    }

    ret = UartLineWrite(pMod->hUart, pCmd, 100);
    CHECK_RETVAL(ret, ExitOnFailure);

    PortableSleep(250);

    ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), 100);
    CHECK_RETVAL(ret, ExitOnFailure);

    if (!strncmp(Buf, "AOK", 3))
    {
        if (pResponse)
        {
            strncpy(pResponse, Buf, Len);
        }

        if (pCmdResult)
        {
            *pCmdResult = S_OK;
        }
    }

    ret = UartPurge(pMod->hUart);
    CHECK_RETVAL(ret, ExitOnFailure);

ExitOnFailure:

    return ret;
}

int 
BthModFactoryReset(
    IN BthMod_T *pMod
    )
{
    int ret;
    int CmdRet;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    ret = BthModSendCommand(pMod, "SF,1\n", &CmdRet, NULL, 0);
    CHECK_RETVAL(ret, ExitOnFailure);

    ret = BthModSendCommand(pMod, "R,1\n", &CmdRet, NULL, 0);
    CHECK_RETVAL(ret, ExitOnFailure);

ExitOnFailure:

    return ret;
}    

int 
BthModCommandMode(
    IN BthMod_T *pMod,
    IN uint16_t  bEnter
    )
{
    int ret;
    char Buf[256];

    /* Attempt to enter command mode */
    ret = UartLineWrite2(pMod->hUart, "$$$", 100, 50);
    CHECK_RETVAL(ret, ExitOnFailure);

    /* Read the response, we should see the cmd prompt */
    ret = UartLineRead(pMod->hUart, Buf, sizeof(Buf), 200);
    if (FAILED(ret) && (ret != E_TIMEOUT))
    {
        CHECK_RETVAL(ret, ExitOnFailure);
    }

    /* Have we entered command mode */
    ret = (SUCCEEDED(ret) && !strncmp(Buf, "CMD", 3)) ? S_OK : E_FAIL;
    CHECK_RETVAL(ret, ExitOnFailure);

ExitOnFailure:

    return ret;
}

int 
BthModConfig(
    IN BthMod_T     *pMod
    )
{
    int ret;
    int CmdRet;
    char Response[256];

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);
    
    /* Set profile to SPP */
    ret = BthModSendCommand(pMod, "S~,0\n", &CmdRet, Response, sizeof(Response));
    CHECK_RETVAL(ret, ExitOnFailure);
    CHECK_RETVAL(CmdRet, ExitOnFailure);

    /* Set authentication mode to SSP */
    ret = BthModSendCommand(pMod, "SA,2\n", &CmdRet, Response, sizeof(Response));
    CHECK_RETVAL(ret, ExitOnFailure);
    CHECK_RETVAL(CmdRet, ExitOnFailure);

    /* Set pin code to 0000 */
    // ret = BthModSendCommand(pMod, "SP,0000\n", &CmdRet, Response, sizeof(Response));
    // CHECK_RETVAL(ret, ExitOnFailure);
    // CHECK_RETVAL(CmdRet, ExitOnFailure);

    /* Disable command mode timer */
    ret = BthModSendCommand(pMod, "ST,200\n", &CmdRet, Response, sizeof(Response));
    CHECK_RETVAL(ret, ExitOnFailure);
    CHECK_RETVAL(CmdRet, ExitOnFailure);

    ret = BthModSendCommand(pMod, "SO,->\n", &CmdRet, Response, sizeof(Response));
    CHECK_RETVAL(ret, ExitOnFailure);
    CHECK_RETVAL(CmdRet, ExitOnFailure);

    ret = BthModSendCommand(pMod, "R,1\n", &CmdRet, Response, sizeof(Response));
    CHECK_RETVAL(ret, ExitOnFailure);

    PortableSleep(1000);

    ret = UartPurge(pMod->hUart);
    CHECK_RETVAL(ret, ExitOnFailure);

    ret = BthModCommandMode(pMod, TRUE);
    CHECK_RETVAL(ret, ExitOnFailure);

    return S_OK;

ExitOnFailure:

    return E_FAIL;
}
