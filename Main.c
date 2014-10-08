#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Common.h"
#include "Uart.h"
#include "Portable.h"
#include "BthMod.h"
#include "Main.h"

#define DEBUG_MODULE
#define DEBUG_LEVEL DBG_TRACE|DBG_WARN|DBG_ERROR 
#include "Debug.h"

int
BthModTest(
    IN const char *pszPort
    )
{
    int ret;
    uhandle_t hModule = 0;
    BthModBda_T Bda;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    ret = BthModOpen(pszPort, &hModule);
	CHECK_RETVAL(ret, ExitOnFailure);

    for ( ; ; )
    {    
        ret = BthModListen(hModule, 1000, &Bda);
        if (ret == E_TIMEOUT)
        {
            printf(".");
            continue;
        }
    	CHECK_RETVAL(ret, ExitOnFailure);

        if (ret == S_OK)
        {
            break;
        }
    }

    printf("\n");
    printf("Connection formed\n");

    PortableSleep(5000);

ExitOnFailure:

    BthModClose(hModule);

    return ret;
}

int 
main(
    int     ac, 
    char    **av
    )
{
    int ret = 0;

    DBG_MSG(DBG_TRACE, "%s\n", __FUNCTION__);

    printf("usage: bthmod <port>\n");

	ret = ac == 2 ? S_OK : E_FAIL;
	CHECK_RETVAL(ret, ExitOnFailure);

	ret = BthModTest(av[1]);
	CHECK_RETVAL(ret, ExitOnFailure);

ExitOnFailure:

    return ret;
}

    
