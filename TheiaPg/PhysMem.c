#include "LinkHeader.h"

/*++
* Routine: HrdIndpnRWVMemory
* 
* MaxIRQL: IPI_LEVEL
* 
* Public/Private: Public
* 
* @param FlagsExecute [MEM_INDPN_RW_READ_OP_BIT]: Read operation flag
* 
* @param FlagsExecute [MEM_INDPN_RW_WRITE_OP_BIT]: Write operation flag
* 
* @param Va: The target virtual address of physical memory
* 
* @param IoBufferMirror: I/O buffer for Read/Write
* 
* @param LengthRW: Length Read/Write
* 
* Description: Independent RW virtual memory.
--*/
VOID HrdIndpnRWVMemory(IN ULONG64 FlagsExecute, IN OUT PVOID pVa, IN OUT PVOID pIoBuffer, IN ULONG32 LengthRW)
{	
	//
	// InternalCodeError HrdIndpnRWVMemory.
	//
    #define ERROR_READ_V_MEMORY  0x848d7e65UI32
    #define ERROR_WRITE_V_MEMORY 0x0fa7049fUI32

	//
	// Execution barriers are required to implement recursive IPI (All including self) initiation.
	//
	volatile static LONG32 SynchBarrier0 = 0I32;
	volatile static LONG32 SynchBarrier1 = 0I32;


	//
	// Are present to retrieve passed arguments from an IPI-CTX passed as a 64-bit moment non-ipi execution array.
	//
	ULONG64 FlagsExecuteMirror = 0UI64;	 
	PVOID pVaMirror = NULL;
	PVOID pIoBufferMirror = NULL;
	ULONG32 LengthRWMirror = 0UI32;
	 

	ULONG64 RelatedDataIpiCtx[4] = { 0 }; 
	 
	LONG32 SaveRel32Offset = 0I32;
	
	PMMPTE_HARDWARE pPteInputVa = NULL;
	
	PVOID pMetaVPage = NULL;
	
	PMMPTE_HARDWARE pMetaVPagePte = NULL;
	
	ULONG64 SizeMetaVPage = 0UI64;
	
	ULONG64 FilteredConstsAfterCompute[6] = { 0 }; ///< [0/1/2]: MmBase/Offset/Mask | [3]: Alligment | [4/5]: Mask/Mask
	
	ULONG64 SaveGlobalVarsInMiShwBadMap[2] = { 0 };
	
	PBOOLEAN pKdPitchDebugger = NULL;
	
	PULONG64 pVfRuleClasses = NULL;
	
	PVOID pResultVa = NULL;

	CheckStatusTheiaCtx();

	if (!SynchBarrier0)
	{
		if (__readcr8() > IPI_LEVEL)
		{
			DbgLog("[TheiaPg <->] HrdIndpnRWVMemory: Inadmissible IRQL | IRQL: 0x%02X\n", __readcr8());

			if (FlagsExecute & MEM_INDPN_RW_READ_OP_BIT) { goto DIE_CALL_ERROR_READ_V_MEMORY; }
			else { goto DIE_CALL_ERROR_WRITE_V_MEMORY; }
		}

		if (!(FlagsExecute & 0x3UI64))
		{
			DbgLog("[TheiaPg <->] HrdIndpnRWVMemory: Invalid FlagsExecute | FlagsExecute: 0x%I32X\n", (ULONG32)FlagsExecute);

			if (FlagsExecute & MEM_INDPN_RW_READ_OP_BIT) { goto DIE_CALL_ERROR_READ_V_MEMORY; }
			else { goto DIE_CALL_ERROR_WRITE_V_MEMORY; }
		}
		else if (!pVa || !((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(pVa) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(pVa)))
		{
			DbgLog("[TheiaPg <->] HrdIndpnRWVMemory: Invalid Va\n");

			if (FlagsExecute & MEM_INDPN_RW_READ_OP_BIT) { goto DIE_CALL_ERROR_READ_V_MEMORY; }
			else { goto DIE_CALL_ERROR_WRITE_V_MEMORY; }
		}
		else if (!pVa || !((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(pIoBuffer) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(pIoBuffer)))
		{
			DbgLog("[TheiaPg <->] HrdIndpnRWVMemory: Invalid InputBuffer\n");

			if (FlagsExecute & MEM_INDPN_RW_READ_OP_BIT) { goto DIE_CALL_ERROR_READ_V_MEMORY; }
			else { goto DIE_CALL_ERROR_WRITE_V_MEMORY; }
		}
		else if (!LengthRW)
		{
			DbgLog("[TheiaPg <->] HrdIndpnRWVMemory: Invalid LengthRW\n");

			if (FlagsExecute & MEM_INDPN_RW_READ_OP_BIT) { goto DIE_CALL_ERROR_READ_V_MEMORY; }
			else { goto DIE_CALL_ERROR_WRITE_V_MEMORY; }
		}
		else { VOID; } ///< For clarity.

		RelatedDataIpiCtx[0] = FlagsExecute;

		RelatedDataIpiCtx[1] = pVa;

		RelatedDataIpiCtx[2] = pIoBuffer;

		RelatedDataIpiCtx[3] = LengthRW;

		SynchBarrier0 = 1I32;

		g_pTheiaCtx->pKeIpiGenericCall(&HrdIndpnRWVMemory, &RelatedDataIpiCtx);

		goto ExitForNoMainExecuter;
	}
	else
	{
		_disable();

		if (!(_interlockedbittestandset(&SynchBarrier1,0I32)))
		{		
			FlagsExecuteMirror = *(PULONG64)FlagsExecute; ///< When executed as IPI, FlagsExecute will be used as ptr on IpiContext.

			pVaMirror = ((PULONG64)FlagsExecute)[1];

			pIoBufferMirror = ((PULONG64)FlagsExecute)[2];

			LengthRWMirror = ((PULONG64)FlagsExecute)[3];
		}
		else
		{
			while (SynchBarrier1) { _mm_pause(); }

			goto ExitForNoMainExecuter;
		}
	}

	do
	{
		//
		// PXE is not included in the VA analysis because the probability of a 512-GB LargePage is extremely close to 0.
		//

		pPteInputVa = ((PMMPTE_HARDWARE)(g_pTheiaCtx->pMmPpeBase + (((ULONG64)pVaMirror >> 27UI64) & 0x1FFFF8UI64)));

		if (pPteInputVa->LargePage)
		{
			SizeMetaVPage = 0x40000000UI64;

			FilteredConstsAfterCompute[0] = g_pTheiaCtx->pMmPpeBase;

			FilteredConstsAfterCompute[1] = 27UI64;

			FilteredConstsAfterCompute[2] = 0x1FFFF8UI64;

			FilteredConstsAfterCompute[4] = ~0x3FFFFFFFUI64;

			FilteredConstsAfterCompute[5] = 0x3FFFFFFFUI64;

			break;
		}

		pPteInputVa = ((PMMPTE_HARDWARE)(g_pTheiaCtx->pMmPdeBase + (((ULONG64)pVaMirror >> 18) & 0x3FFFFFF8I64)));

		if (pPteInputVa->LargePage)
		{
			SizeMetaVPage = 0x200000UI64;

			FilteredConstsAfterCompute[0] = g_pTheiaCtx->pMmPdeBase;

			FilteredConstsAfterCompute[1] = 18;

			FilteredConstsAfterCompute[2] = 0x3FFFFFF8UI64;

			FilteredConstsAfterCompute[4] = ~0x1FFFFFUI64;

			FilteredConstsAfterCompute[5] = 0x1FFFFFUI64;

			break;
		}

		pPteInputVa = ((PMMPTE_HARDWARE)(g_pTheiaCtx->pMmPteBase + (((ULONG64)pVaMirror >> 9UI64) & 0x7FFFFFFFF8UI64)));

		SizeMetaVPage = 0x1000UI64;

		FilteredConstsAfterCompute[0] = g_pTheiaCtx->pMmPteBase;

		FilteredConstsAfterCompute[1] = 9;

		FilteredConstsAfterCompute[2] = 0x7FFFFFFFF8UI64;

		FilteredConstsAfterCompute[4] = ~0x0FFFUI64;

		FilteredConstsAfterCompute[5] = 0x0FFFUI64;

		break;

	} while (FALSE);

	SaveGlobalVarsInMiShwBadMap[0] = *KdDebuggerNotPresent;

	*KdDebuggerNotPresent = 1I8;

	SaveRel32Offset = *(PLONG32)((PUCHAR)g_pTheiaCtx->pIoCancelIrp + 0x12UI64);

	pVfRuleClasses = (PULONG64)(((PUCHAR)g_pTheiaCtx->pIoCancelIrp + 0x16UI64) + ((SaveRel32Offset < 0I32) ? ((LONG64)SaveRel32Offset | 0xffffffff00000000UI64) : (LONG64)SaveRel32Offset));

	SaveGlobalVarsInMiShwBadMap[1UI8] = *pVfRuleClasses;

	*pVfRuleClasses |= 0x400000UI64;

	                                                                          /* PaBaseSystemVaRange is used as a stub for mapping */
	                                                                                                  /* | */
	                                                                                                  /* # */
	pMetaVPage = (PVOID)g_pTheiaCtx->pMmMapIoSpaceEx(g_pTheiaCtx->pMmGetPhysicalAddress(g_pTheiaCtx->pKernelBase), SizeMetaVPage, PAGE_READWRITE | PAGE_NOCACHE);
	                                    /* ^ */
	                                    /* | */ 
	                    /* Important: the call takes place on IPI_LEVEL (With repeated debugging, this did not cause problems) */

	*KdDebuggerNotPresent = (BOOLEAN)(SaveGlobalVarsInMiShwBadMap[0]);

	*pVfRuleClasses = SaveGlobalVarsInMiShwBadMap[1];

	if (!pMetaVPage)
	{
		if (FlagsExecute & MEM_INDPN_RW_READ_OP_BIT) { goto DIE_CALL_ERROR_READ_V_MEMORY; }
		else { goto DIE_CALL_ERROR_WRITE_V_MEMORY; }
	}

	pMetaVPagePte = (PMMPTE_HARDWARE)(FilteredConstsAfterCompute[0] + (((ULONG64)pMetaVPage >> FilteredConstsAfterCompute[1]) & FilteredConstsAfterCompute[2]));

	if (SizeMetaVPage != 0x1000UI64) { *(PULONG64)pMetaVPagePte |= 0x80UI64; } ///< LargePageBitFix.

	pMetaVPagePte->PageFrameNumber = pPteInputVa->PageFrameNumber;

	pResultVa = (((ULONG64)pMetaVPage & FilteredConstsAfterCompute[4]) | ((ULONG64)pVaMirror & FilteredConstsAfterCompute[5]));

	__writecr3(__readcr3()); ///< Flush TLB.

	if (FlagsExecuteMirror & MEM_INDPN_RW_READ_OP_BIT) { memcpy(pIoBufferMirror, pResultVa, LengthRWMirror); }
	else { memcpy(pResultVa, pIoBufferMirror, LengthRWMirror); }

	g_pTheiaCtx->pMmUnmapIoSpace(pMetaVPage, SizeMetaVPage);

	SynchBarrier0 = 0I32;

	SynchBarrier1 = 0I32;

	if (g_VolatileNullByte) { DIE_CALL_ERROR_READ_V_MEMORY: DieDispatchIntrnlError(ERROR_READ_V_MEMORY); }

	if (g_VolatileNullByte) { DIE_CALL_ERROR_WRITE_V_MEMORY: DieDispatchIntrnlError(ERROR_WRITE_V_MEMORY); }

ExitForNoMainExecuter:

	if (!(HrdGetIF())) { _enable(); }

	return;
}

/*++
* Routine: HrdPatchAttributesInputPte
*
* MaxIRQL: DISPATCH_LEVEL
* 
* Public/Private: Public
*
* @param AndMask: AndMask for PTE attributes
*
* @param OrMask: OrMask for PTE attributes
*
* @param Va: Target virtual address
* 
* Description: Allows you to change the attributes of the end PTE VA/GVA.
--*/
VOID HrdPatchAttributesInputPte(IN ULONG64 AndMask, IN ULONG64 OrMask, IN OUT PVOID pVa)
{
#define ERROR_PATCH_PTE_ATTRIBUTES 0xdec74dfaUI32

	PMMPTE_HARDWARE pPteInputVa = NULL;

	CheckStatusTheiaCtx();

	if ((!AndMask && !OrMask) || (AndMask && OrMask))
	{
		DbgLog("[TheiaPg <->] HrdPatchAttributesInputPte: Invalid ???Mask.\n");

		goto DIE_CALL_ERROR_PATCH_PTE_ATTRIBUTES;
	}
	else if (!pVa || !((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(pVa) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(pVa)))
	{
		DbgLog("[TheiaPg <->] HrdPatchAttributesInputPte: Invalid Va\n");

		goto DIE_CALL_ERROR_PATCH_PTE_ATTRIBUTES;
	}
	else { VOID; } ///< For clarity.

	pPteInputVa = HrdGetPteVa(pVa);

	if (AndMask) { _InterlockedAnd64(pPteInputVa, AndMask); }
	else { _InterlockedOr64(pPteInputVa, OrMask); }

	__writecr3(__readcr3());


	if (g_VolatileNullByte) { DIE_CALL_ERROR_PATCH_PTE_ATTRIBUTES: DieDispatchIntrnlError(ERROR_PATCH_PTE_ATTRIBUTES); }

	return;
}

/*++
* Routine: HrdGetPteVa
*
* MaxIRQL: Any level
*
* Public/Private: Public
*
* @param Va: Target virtual address
*
* Description: Getting Self-Mapp-PTE VA
--*/
PMMPTE_HARDWARE HrdGetPteVa(IN PVOID pVa)
{
    #define ERROR_GET_PTE_VA 0x11ecdf34UI32

	PMMPTE_HARDWARE pPteInputVa = NULL;

	CheckStatusTheiaCtx();
	
	if (!pVa || !((__readcr8() <= DISPATCH_LEVEL) ? g_pTheiaCtx->pMmIsAddressValid(pVa) : g_pTheiaCtx->pMmIsNonPagedSystemAddressValid(pVa)))
	{
		DbgLog("[TheiaPg <->] HrdPatchAttributesInputPte: Invalid Va\n");

		goto DIE_CALL_ERROR_GET_PTE_VA;
	}
	else { VOID; } ///< For clarity.

	do
	{
		//
		// PXE is not included in the VA analysis because the probability of a 512-GB LargePage is extremely close to 0.
		//

		pPteInputVa = ((PMMPTE_HARDWARE)(g_pTheiaCtx->pMmPpeBase + (((ULONG64)pVa >> 27UI64) & 0x1FFFF8UI64)));

		if (pPteInputVa->LargePage) { goto IsLargePte; }

		pPteInputVa = ((PMMPTE_HARDWARE)(g_pTheiaCtx->pMmPdeBase + (((ULONG64)pVa >> 18UI64) & 0x3FFFFFF8UI64)));

		if (pPteInputVa->LargePage) { goto IsLargePte; }

		pPteInputVa = ((PMMPTE_HARDWARE)(g_pTheiaCtx->pMmPteBase + (((ULONG64)pVa >> 9UI64) & 0x7FFFFFFFF8UI64)));

	IsLargePte:

		break;

	} while (FALSE);

	if (g_VolatileNullByte) { DIE_CALL_ERROR_GET_PTE_VA: DieDispatchIntrnlError(ERROR_GET_PTE_VA); }

	return pPteInputVa;
}
