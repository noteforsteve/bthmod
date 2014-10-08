#ifndef UARTLINE_HXX
#define UARTLINE_HXX

int
UartLineRead(
	IN uhandle_t 	hUart,
    IN char 		*pLine,
    IN int 			Line,
	IN uint32_t 	MaxWait
	);
	
int
UartLineWrite(
	IN uhandle_t 	hUart,
    IN const char   *pLine,
	IN uint32_t 	MaxWait
	);

int
UartLineWrite2(
	IN uhandle_t 	hUart,
    IN const char 	*pLine,
    IN uint32_t     InterCharacterDelay,
	IN uint32_t 	MaxWait
	);

#endif

