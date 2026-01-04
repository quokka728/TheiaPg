#include "LinkHeader.h"

PTHEIA_CONTEXT g_pTheiaCtx        = NULL;

BOOLEAN g_CompleteInitGlobalData  = FALSE;

volatile UCHAR g_VolatileNullByte = 0UI8;

PVOID g_pStackAddrRetAddrImgEntry = NULL;
