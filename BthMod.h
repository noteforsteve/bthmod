#ifndef BTHMOD_H
#define BTHMOD_H
 
typedef struct
{
    uint8_t raw[6];
} BthModBda_T;
 
int
BthModOpen(
    IN const char   *port,
    OUT uhandle_t   *phMod
    );
 
void
BthModClose(
    IN uhandle_t    hMod
    );
 
int
BthModConnect(
    IN uhandle_t    hMod,
    IN BthModBda_T *pBda
    );
 
int
BthModListen(
    IN uhandle_t    hMod,
    IN uint32_t     uWait,
    OUT BthModBda_T *pBda
    );
 
int
BthModDisconnect(
    IN uhandle_t    hMod
    );
 
int
BthModSend(
    IN uhandle_t    hMod,
    IN uint16_t     uLen,
    IN uint8_t      *pData
    );
 
int
BthModRecv(
    IN uhandle_t    hMod,
    IN uint16_t     uLen,
    IN uint8_t      *pData,
    OUT uint16_t    *puRecv
    );
 
int
BthModBdaToStr(
    IN BthModBda_T  *pBda,
    IN char         *pStr,
    IN uint16_t     uLen
    );
 
int
BthModStrToBda(
    IN const char   *pStr,
    IN BthModBda_T  *pBda
    );
 
#endif
