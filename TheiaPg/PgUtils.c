#include "LinkHeader.h"

/*++
* Routine: SearchPgCtx 
*
* MaxIRQL: Any level (If IRQL > DISPATCH_LEVEL then the input address must be NonPaged)
*
* Public/Private: Public
*
* @param Ctx: Pointer to _CONTEXT structure
*
* Description: Routine to check the _CONTEXT structure for PgCtx.
--*/
PVOID SearchPgCtx(IN PCONTEXT pCtx)
{
	CheckStatusTheiaCtx();

	if (!((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(pCtx) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(pCtx)))
	{
		DbgLog("[TheiaPg <->] SearchPgCtx: Invalid Ctx\n\n");

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

/*++
* Routine: SearchPgKdpc
*
* MaxIRQL: Any level (If IRQL > DISPATCH_LEVEL then the input address must be NonPaged)
*
* Public/Private: Public
*
* @param Ctx: Pointer to _CONTEXT structure
*
* Description: Routine to check the _CONTEXT structure for PgKdpc.
--*/
PVOID SearchPgKdpc(IN PCONTEXT pCtx)
{
	CheckStatusTheiaCtx();

	if (!((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(pCtx) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(pCtx)))
	{
		DbgLog("[TheiaPg <->] SearchPgKdpc: Invalid Ctx\n\n");

		return NULL;
	}

	PULONG64 pPgKdpc = (PULONG64)&pCtx->Rax;

	for (UCHAR i = 0UI8; i < 16UI8; ++i, ++pPgKdpc)
	{
		if (!((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid((PVOID)(*pPgKdpc)) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid((PVOID)(*pPgKdpc)))) { continue; }

		if (((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(((PKDPC)(*pPgKdpc))->DeferredRoutine) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(((PKDPC)(*pPgKdpc))->DeferredRoutine)))
		{
			if (!((HrdGetPteVa(((PKDPC)(*pPgKdpc))->DeferredRoutine))->NoExecute)) { return *pPgKdpc; }
		}
	}

	return NULL;
}
