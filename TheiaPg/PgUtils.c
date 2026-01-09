#include "LinkHeader.h"

/*++
* Routine: SearchPgCtxInCtx 
*
* MaxIRQL: Any level (If IRQL > DISPATCH_LEVEL then the input address must be NonPaged)
*
* Public/Private: Public
*
* @param Ctx: Pointer to _CONTEXT structure
*
* Description: Routine to check the _CONTEXT structure for PgCtx.
--*/
PVOID SearchPgCtxInCtx(IN PCONTEXT pCtx)
{
	CheckStatusTheiaCtx();

	if (!((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(pCtx) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(pCtx)))
	{
		DbgLog("[TheiaPg <->] SearchPgCtxInCtx: Invalid Ctx\n\n");

		return NULL;
	}

	PULONG64 pPgCtx = (PULONG64)&pCtx->Rax;

	for (UCHAR i = 0UI8; i < 16UI8; ++i, ++pPgCtx)
	{
		if (!((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(*pPgCtx) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(*pPgCtx))) { continue; }

		if (!(memcmp(*pPgCtx, &g_pTheiaCtx->PgXorRoutineSig, sizeof(g_pTheiaCtx->PgXorRoutineSig)))) { return *(PVOID*)pPgCtx; }
	}

	return NULL;
}
