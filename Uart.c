#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Portable.h"
#include "Common.h"
#include "Uart.h"
#include "WinUart.h"
#include "OsxUart.h"

#define DEBUG_MODULE 
#define DEBUG_LEVEL DBG_ERROR|DBG_WARN
#include "Debug.h"

#ifdef WINDOWS
#pragma warning(disable: 4127)
#endif

int
UartCtor(
    OUT uhandle_t	*phUart
    )
{
    int Retval;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    Retval = phUart ? S_OK : E_INVALIDARG;
    CHECK_RETVAL(Retval, ExitOnFailure);

#ifdef WINDOWS
    Retval = WinUartCtor(phUart);
#elif OSX
    Retval = OsxUartCtor(phUart);
#else
    DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif

ExitOnFailure:

    return Retval;
}

void
UartDtor(
    IN uhandle_t 	hUart
    )
{
    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    if (hUart)
    {
        UartClose(hUart);

#ifdef WINDOWS
        WinUartDtor(hUart);
#elif OSX
        OsxUartDtor(hUart);
#else
        DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif

    }
}

int
UartOpen(
    IN uhandle_t 	hUart,
    IN const char   *pName,
    IN unsigned int uRate,
    IN unsigned int uDataBits,
    IN unsigned int uParity,
    IN unsigned int uStopBits
    )
{
    int Retval;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    Retval = hUart && pName ? S_OK : E_INVALIDARG;
    CHECK_RETVAL(Retval, ExitOnFailure);

#ifdef WINDOWS
    Retval = WinUartOpen(hUart, pName, uRate, uDataBits, uParity, uStopBits);
#elif OSX
    Retval = OsxUartOpen(hUart, pName, uRate, uDataBits, uParity, uStopBits);
#else
    DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif 

ExitOnFailure:

    return Retval;
}

void
UartClose(
    IN uhandle_t	hUart
    )
{
    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

#ifdef WINDOWS
    WinUartClose(hUart);
#elif OSX
    OsxUartClose(hUart);
#else
    DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif
}

int
UartRead(
    IN uhandle_t	hUart,
    IO void         *pBuff, 
    IN unsigned int uLength, 
    OUT unsigned int *puRead,
    IN unsigned int uWaitTime
    )
{
    int Retval;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    Retval = hUart && pBuff ? S_OK : E_INVALIDARG;
    CHECK_RETVAL(Retval, ExitOnFailure);

#ifdef WINDOWS
    Retval = WinUartRead(hUart, pBuff, uLength, puRead, uWaitTime);
#elif OSX
    Retval = OsxUartRead(hUart, pBuff, uLength, puRead, uWaitTime);
#else
    DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif

ExitOnFailure:

    return Retval;
}

int
UartWrite(
    IN uhandle_t	hUart,
    OUT const void  *pBuff,
    IN unsigned int uLength,
    OUT unsigned int *puWritten,
    IN unsigned int uWaitTime
    )
{
    int Retval;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    Retval = hUart && pBuff ? S_OK : E_INVALIDARG;
    CHECK_RETVAL(Retval, ExitOnFailure);

#ifdef WINDOWS
    Retval = WinUartWrite(hUart, pBuff, uLength, puWritten, uWaitTime);
#elif OSX
    Retval = OsxUartWrite(hUart, pBuff, uLength, puWritten, uWaitTime);
#else
    DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif

ExitOnFailure:

    return Retval;
}

int
UartSetStatus(
    IN uhandle_t	hUart,
    IN unsigned int uState
    )
{
    int Retval;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    Retval = hUart ? S_OK : E_INVALIDARG;
    CHECK_RETVAL(Retval, ExitOnFailure);

#ifdef WINDOWS
    Retval = WinUartSetStatus(hUart, uState);
#elif OSX
    Retval = OsxUartSetStatus(hUart, uState);
#else
    DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif

ExitOnFailure:

    return Retval;
}

int
UartGetStatus(
    IN uhandle_t	hUart,
    OUT unsigned int *puState
    )
{
    int Retval;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    Retval = hUart && puState ? S_OK : E_INVALIDARG;
    CHECK_RETVAL(Retval, ExitOnFailure);

#ifdef WINDOWS
    Retval = WinUartGetStatus(hUart, puState);
#elif OSX
    Retval = OsxUartGetStatus(hUart, puState);
#else
    DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif

ExitOnFailure:

    return Retval;
}

int
UartPurge(
    IN uhandle_t 	hUart
    )
{
    int Retval;
#ifdef WINDOWS
    Retval = WinUartPurge(hUart);
#elif OSX
    Retval = OsxUartPurge(hUart);
#else
    DBG_MSG(DBG_TRACE, "Unknown OS_TYPE %d\n", OS_TYPE);
#endif
    return Retval;
}


/*** Uart Test *******************************************************************/
#if defined (UART_TESTS)

void
UartTestShowModemStatus(
    IN unsigned int uModemStatus
    )
{
    printf("ModemStatus %04x - ", uModemStatus);

    if (uModemStatus & UART_STATUS_CTS)
        printf(" cts ");
    if (uModemStatus & UART_STATUS_DSR)
        printf(" dsr ");
    if (uModemStatus & UART_STATUS_RI)
        printf(" ri ");
    if (uModemStatus & UART_STATUS_DCD)
        printf(" dcd ");
    printf("\n");
}

void
UartTestToUpper(
    IN char 		*p,
    IN unsigned int uLen
    )
{
    for ( ; uLen; uLen = uLen - 1)
    {
        *p = (char)toupper(*p);
        p = p + 1;
    }
}

/* Show state of modem status signals */ 
int 
UartTest(
    IN const char *pszPort
    )
{
    int Retval;
    uhandle_t hUart;
    unsigned int uStatus;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    Retval = UartCtor(&hUart);
    CHECK_RETVAL(Retval, ExitOnFailure);

    Retval = UartOpen(hUart, 
                      pszPort,
                      UART_RATE_57600, 
                      UART_DATA_BITS_8, 
                      UART_PARITY_NONE, 
                      UART_STOP_1);
    CHECK_RETVAL(Retval, ExitOnFailure);

    for ( ; ; )
    {
        Retval = UartGetStatus(hUart, &uStatus);
        CHECK_RETVAL(Retval, ExitOnFailure);

        UartTestShowModemStatus(uStatus);

        PortableSleep(1000);
    }
  
ExitOnFailure:

    UartDtor(hUart);

    return Retval;
}

/* Echo back recevied data in upper case */
int 
UartTest1(
    IN const char *pszPort
    )
{
    int Retval;
    uhandle_t hUart;
    char Buff[256];
    unsigned int BytesRead;
    unsigned int BytesWritten;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    Retval = UartCtor(&hUart);
    CHECK_RETVAL(Retval, ExitOnFailure);

    Retval = UartOpen(hUart, 
                      pszPort,
                      UART_RATE_57600, 
                      UART_DATA_BITS_8, 
                      UART_PARITY_NONE, 
                      UART_STOP_1);
    CHECK_RETVAL(Retval, ExitOnFailure);

    for ( ; ; )
    {
        Retval = UartRead(hUart, Buff, sizeof(Buff), &BytesRead, 1000);
    
        if (BytesRead)
        {	
            UartTestToUpper(Buff, BytesRead);

            Retval = UartWrite(hUart, Buff, BytesRead, &BytesWritten, 1000);
            CHECK_RETVAL(Retval, ExitOnFailure);
        }
    }
  
ExitOnFailure:

    UartDtor(hUart);

    return Retval;
}

#endif


