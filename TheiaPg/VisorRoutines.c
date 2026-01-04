#include "LinkHeader.h"

/*++
* notes DPCS
* 
* PG artifacts in the fields of the KDPC DeferredContext structure ceased to be a static artifact in 25h2,
* PG began to use a different method of obfuscating the transfer of the local context address by obfuscating the lower 4 bytes of the context address transferred in DeferredContext
* 
* 0: kd> dt nt!_KDPC FFFF9E83407FD750
*    +0x000 TargetInfoAsUlong : 0x113
*    +0x000 Type             : 0x13 ''
*    +0x001 Importance       : 0x1 ''
*    +0x002 Number           : 0
*    +0x008 DpcListEntry     : _SINGLE_LIST_ENTRY
*    +0x010 ProcessorHistory : 1
*    +0x018 DeferredRoutine  : 0xfffff801`8409ba30 Void  nt!ExpTimerDpcRoutine+0
*    +0x020 DeferredContext  : 0xffff9e83`407fd6b0 Void !!! <- The actual address of the local PG context at the time of analysis was 0xFFFF9E833E1301BC, differences in the 4 lower bytes.
*    +0x028 SystemArgument1  : (null)
*    +0x030 SystemArgument2  : (null)
*    +0x038 DpcData          : (null)
--*/

/*++
* Routine: VsrKiExecuteAllDpcs
*
* MaxIRQL: DISPATCH_LEVEL
*
* Public/Private: Public
*
* @param InputCtx: Context passed from StubCallTrmpln
*
* Description: Hook KiExecuteAllDpcs for controling _KDPC in DPC_QUEUE current Cpu-Core.
--*/
volatile VOID VsrKiExecuteAllDpcs(PINPUTCONTEXT_ICT pInputCtx)
{
    UCHAR ReasonDetect0[] = { "Unbacked DeferredRoutine" };

    UCHAR ReasonDetect1[] = { "PG DeferredContext" };

    UCHAR TypeDetect = 0UI8;

    PVOID pVoidRoutine = NULL;

    for (ULONG32 i = 0UI32; ; ++i)
    {
        if (*((PUCHAR)&VsrKiExecuteAllDpcs + i) == 0xC3UI8)
        { 
            pVoidRoutine = ((PUCHAR)&VsrKiExecuteAllDpcs + i);

            break;
        }
    }
   
    PVOID pDpcListHead[2UI8] = { 0 };

    CheckStatusTheiaCtx();

    pDpcListHead[DPC_NORMAL] = (PVOID)__readgsqword((g_pTheiaCtx->TheiaMetaDataBlock.KPCR_Prcb_OFFSET + g_pTheiaCtx->TheiaMetaDataBlock.KPRCB_DpcData0_OFFSET)); ///< Get address first node DPC_NORMAL_QUEUE.

    pDpcListHead[DPC_THREADED] = (PVOID)__readgsqword((g_pTheiaCtx->TheiaMetaDataBlock.KPCR_Prcb_OFFSET + g_pTheiaCtx->TheiaMetaDataBlock.KPRCB_DpcData1_OFFSET)); ///< Get address first node DPC_THREADED_QUEUE.

    PKDPC pCurrentKDPC = NULL;

    BOOLEAN FlagCurrentQueue = FALSE; ///< FALSE: DPC_NORMAL & TRUE: DPC_THREADED

    BOOLEAN LockCurrentQueue = FALSE;

    while (TRUE)
    {
        if (!FlagCurrentQueue && !LockCurrentQueue)
        {
            if (!pDpcListHead[DPC_NORMAL]) { FlagCurrentQueue = TRUE; }
            else { pCurrentKDPC = CONTAINING_RECORD(pDpcListHead[DPC_NORMAL], KDPC, DpcListEntry); LockCurrentQueue = TRUE; }
        }

        if (FlagCurrentQueue && !LockCurrentQueue)
        {
            if (!pDpcListHead[DPC_THREADED]) { break; }
            else { pCurrentKDPC = CONTAINING_RECORD(pDpcListHead[DPC_THREADED], KDPC, DpcListEntry); LockCurrentQueue = TRUE; }
        }

        if (_IsAddressSafe(pCurrentKDPC->DeferredRoutine)) { if (!(g_pTheiaCtx->pMmIsAddressValid(pCurrentKDPC->DeferredContext)) && (((ULONG64)pCurrentKDPC->DeferredContext) & ~0x03I64)) { goto DetectJmp; } }
        else { TypeDetect = 1UI8; goto DetectJmp; }

        if (g_VolatileNullByte)
        {
        DetectJmp:

            DbgLog("[TheiaPg <+>] VsrKiExecuteAllDpcs: Detect possibly PG-KDPC | Reason: %s | DeferredRoutine: 0x%I64X | DeferredContext: 0x%I64X | KDPC: 0x%I64X\n\n", TypeDetect ? ReasonDetect0 : ReasonDetect1, pCurrentKDPC->DeferredRoutine, pCurrentKDPC->DeferredContext, pCurrentKDPC);

            pCurrentKDPC->DeferredRoutine = pVoidRoutine;
        }

        if (!(pCurrentKDPC->DpcListEntry.Next) && !FlagCurrentQueue) { FlagCurrentQueue = TRUE; LockCurrentQueue = FALSE; continue; }

        else if (!(pCurrentKDPC->DpcListEntry.Next) && FlagCurrentQueue) { break; }

        else { pCurrentKDPC = CONTAINING_RECORD(pCurrentKDPC->DpcListEntry.Next, KDPC, DpcListEntry); }
    }

    return;
}

/*++
* Routine: VsrExAllocatePool2
*
* MaxIRQL: DISPATCH_LEVEL
*
* Public/Private: Public
*
* @param InputCtx: Context passed from StubCallTrmpln
*
* Description: Hook ExAllocatePool2 for controling callers/cpu-ctx, PG routine 25h2-w11 for memory allocation use ExAllocatePool2/MmAllocateIndependentPages(for re-allocation LocalPgCtx)).
--*/
volatile VOID VsrExAllocatePool2(IN OUT PINPUTCONTEXT_ICT pInputCtx)
{
    CheckStatusTheiaCtx();

    if (!(g_pTheiaCtx->pPsIsSystemThread((PETHREAD)__readgsqword(0x188UI32)))) { return; }
    
    PCONTEXT pInternalCtx = (PCONTEXT)g_pTheiaCtx->pMmAllocateIndependentPagesEx(PAGE_SIZE, -1I32, 0I64, 0I32);

    if (!pInternalCtx) { DbgLog("[TheiaPg <->] VsrExAllocatePool2: Bad alloc page for InternalCtx\n"); return; }

    pInternalCtx->ContextFlags = CONTEXT_CONTROL;
    pInternalCtx->Rax          = pInputCtx->rax;
    pInternalCtx->Rcx          = pInputCtx->rcx;
    pInternalCtx->Rdx          = pInputCtx->rdx;
    pInternalCtx->Rbx          = pInputCtx->rbx;
    pInternalCtx->Rsi          = pInputCtx->rsi;
    pInternalCtx->Rdi          = pInputCtx->rdi;
    pInternalCtx->R8           = pInputCtx->r8;
    pInternalCtx->R9           = pInputCtx->r9;
    pInternalCtx->R10          = pInputCtx->r10;
    pInternalCtx->R11          = pInputCtx->r11;
    pInternalCtx->R12          = pInputCtx->r12;
    pInternalCtx->R13          = pInputCtx->r13;
    pInternalCtx->R14          = pInputCtx->r14;
    pInternalCtx->R15          = pInputCtx->r15;
    pInternalCtx->Rbp          = pInputCtx->rbp;
    pInternalCtx->Rsp          = pInputCtx->rsp;
    pInternalCtx->Rip          = pInputCtx->rip;
    pInternalCtx->EFlags       = pInputCtx->Rflags;

    PULONG64 pRetAddrsTrace = (PULONG64)g_pTheiaCtx->pMmAllocateIndependentPagesEx(PAGE_SIZE, -1I32, 0I64, 0I32);

    if (!pRetAddrsTrace) { DbgLog("[TheiaPg <->] VsrExAllocatePool2: Bad alloc page for RetAddrsTrace\n"); return; }

    PVOID StackHigh, StackLow;

    PVOID pImageBase = NULL;

    PVOID pRuntimeFunction = NULL;

    PVOID pHandlerData = NULL;

    ULONG64 EstablisherFrame = 0UI64;

    PUCHAR pCurrentObjThread = (PUCHAR)__readgsqword(0x188UI32);

    PUSHORT pCurrentTID = (PUSHORT)(pCurrentObjThread + (g_pTheiaCtx->TheiaMetaDataBlock.ETHREAD_Cid_OFFSET + g_pTheiaCtx->TheiaMetaDataBlock.CLIENT_ID_UniqueThread_OFFSET));

    LONG64 Timeout = (-10000UI64 * 31536000000UI64); ///< 1 year.

    LONG32 SaveRel32Offset = 0I32;

    UCHAR RetOpcode = 0xC3UI8;

    PVOID pSearchSdbpCheckDllRWX = NULL;

    BOOLEAN IsSleep = FALSE;

    PVOID pPgCtx = NULL;

    StackHigh = *(PVOID*)(pCurrentObjThread + g_pTheiaCtx->TheiaMetaDataBlock.KTHREAD_InitialStack_OFFSET);

    StackLow = *(PVOID*)(pCurrentObjThread + g_pTheiaCtx->TheiaMetaDataBlock.KTHREAD_StackLimit_OFFSET);

    if (_IsAddressSafe(pInternalCtx->Rip))
    {
        for (ULONG32 i = 0; ; ++i)
        {
            pRuntimeFunction = g_pTheiaCtx->pRtlLookupFunctionEntry(pInternalCtx->Rip, &pImageBase, NULL);

            if (!pRuntimeFunction) ///< If the current routine leaf.
            {
                pInternalCtx->Rip = *(PVOID*)pInternalCtx->Rsp;

                pInternalCtx->Rsp += 8I64;
            }

            g_pTheiaCtx->pRtlVirtualUnwind(0UI32, pImageBase, pInternalCtx->Rip, pRuntimeFunction, pInternalCtx, &pHandlerData, &EstablisherFrame, NULL);

            if ((pInternalCtx->Rsp >= StackHigh) || (pInternalCtx->Rsp <= StackLow) || pInternalCtx->Rip < 0xFFFF800000000000UI64) { break; }

            if (!(_IsAddressSafe(pInternalCtx->Rip)))
            {             
                JmpDetectNonBackedStack:

                DbgLog("[TheiaPg <+>] VsrExAllocatePool2: Detect non-backed stack calls | TCB: 0x%I64X TID: 0x%hX\n", pCurrentObjThread, *pCurrentTID);

                JmpDetectPgCtxInCpuExecuteCtx:
                
                pRetAddrsTrace[i] = pInternalCtx->Rip;

                DbgLog("=================================================================\n");
                DbgLog("RAX: 0x%I64X\n", pInternalCtx->Rax);
                DbgLog("RCX: 0x%I64X\n", pInternalCtx->Rcx);
                DbgLog("RDX: 0x%I64X\n", pInternalCtx->Rdx);
                DbgLog("RBX: 0x%I64X\n", pInternalCtx->Rbx);
                DbgLog("RSI: 0x%I64X\n", pInternalCtx->Rsi);
                DbgLog("RDI: 0x%I64X\n", pInternalCtx->Rdi);
                DbgLog("R8:  0x%I64X\n", pInternalCtx->R8);
                DbgLog("R9:  0x%I64X\n", pInternalCtx->R9);
                DbgLog("R10: 0x%I64X\n", pInternalCtx->R10);
                DbgLog("R11: 0x%I64X\n", pInternalCtx->R11);
                DbgLog("R12: 0x%I64X\n", pInternalCtx->R12);
                DbgLog("R13: 0x%I64X\n", pInternalCtx->R13);
                DbgLog("R14: 0x%I64X\n", pInternalCtx->R14);
                DbgLog("R15: 0x%I64X\n", pInternalCtx->R15);
                DbgLog("RSP: 0x%I64X\n", pInternalCtx->Rsp);
                DbgLog("RBP: 0x%I64X\n", pInternalCtx->Rbp);
                DbgLog("RIP: 0x%I64X\n\n", pInternalCtx->Rip);
                DbgLog("RFLAGS: 0x%I64X\n", pInternalCtx->EFlags);
                DbgLog("================================================================\n");

                DbgText
                ( // {
                
                for (ULONG32 j = 0UI32; ; ++j)
                {
                    if (j == i) { DbgLog("%I32d frame: 0x%I64X <- unbacked\n\n", j, pRetAddrsTrace[j]); break; }

                    DbgLog("%I32d frame: 0x%I64X\n", j, pRetAddrsTrace[j]);
                }

                ) // }

                DbgLog("[TheiaPg <+>] VsrExAllocatePool2: Handling exit phase...\n\n");

                if (pInputCtx->rax) { ExFreePool(pInputCtx->rax); pInputCtx->rax = 0I64; }

                if ((!pPgCtx ? (pPgCtx = SearchPgCtx(pInternalCtx)) : pPgCtx))
                {
                    DbgLog("[TheiaPg <+>] VsrExAllocatePool2: Detect possibly PgCaller | pPgCtx: 0x%I64X\n\n", pPgCtx);

                    if (g_pTheiaCtx->pMmIsAddressValid(*(PVOID*)((PUCHAR)pPgCtx + 0x7f8))) ///< LocalPgCtxBase + 0x7f8: PgDpcRoutine
                    {
                        HrdIndpnRWVMemory(MEM_INDPN_RW_WRITE_OP_BIT, *(PVOID*)((PUCHAR)pPgCtx + 0x7f8), &RetOpcode, 1UI32);
                    }

                    if (g_pTheiaCtx->pMmIsAddressValid(*(PVOID*)((PUCHAR)pPgCtx + 0xA30))) ///< LocalPgCtxBase + 0xA30: PgApcRoutine (basically KiDispatchCallout)
                    {
                        HrdIndpnRWVMemory(MEM_INDPN_RW_WRITE_OP_BIT, *(PVOID*)((PUCHAR)pPgCtx + 0xA30), &RetOpcode, 1UI32);
                    }

                    //
                    // LocalPgCtxBase + 0x808: OffsetFirstRoutineCheck -> LocalPgCtxBase + OffsetFirstRoutineCheck: FirstRoutineCheck (Caller SdbpCheckDllRWX)
                    //
                    pSearchSdbpCheckDllRWX = ((PUCHAR)pPgCtx + (ULONG64)(*(PULONG32)((PUCHAR)pPgCtx + 0x808)));

                    for (ULONG32 j = 0UI32; ; ++j)
                    {
                        if (((PUCHAR)pSearchSdbpCheckDllRWX)[j] == 0xCC && ((PUCHAR)pSearchSdbpCheckDllRWX)[j + 1] == 0xCC && ((PUCHAR)pSearchSdbpCheckDllRWX)[j + 2] == 0xCC)
                        {
                            SaveRel32Offset = *(PLONG32)((PUCHAR)pSearchSdbpCheckDllRWX + (j - 4));

                            pSearchSdbpCheckDllRWX = (((ULONG64)pSearchSdbpCheckDllRWX + j) + ((SaveRel32Offset < 0I32) ? ((ULONG64)SaveRel32Offset | 0xffffffff00000000UI64) : (ULONG64)SaveRel32Offset));

                    /* Skip: 488b742430 mov rsi, qword ptr [rsp+30h] */
                            for (USHORT l = 5UI16; ; ++l)
                            {
                                if (((PUCHAR)pSearchSdbpCheckDllRWX)[l] == 0xff && ((PUCHAR)pSearchSdbpCheckDllRWX)[l + 1] == 0xe6) { break; }
                                else { ((PUCHAR)pSearchSdbpCheckDllRWX)[l] = 0x90UI8; }                    
                            }

                            DbgLog("[TheiaPg <+>] VsrExAllocatePool2: SdbpCheckDllRWX is found: 0x%I64X\n\n", pSearchSdbpCheckDllRWX);

                            break;
                        }
                    }

                    JmpToPossibleSleep:

                    if (__readcr8() < DISPATCH_LEVEL)
                    {
                        DbgLog("[TheiaPg <+>] VsrExAllocatePool2: Enter to dead sleep... | IRQL: 0x%02X\n\n", __readcr8());

                        // __debugbreak();

                        IsSleep = TRUE;

                        break;
                    }
                    else 
                    { 
                        DbgLog("[TheiaPg <+>] VsrExAllocatePool2: Unsuccessful enter to dead sleep... | IRQL: 0x%02X\n\n", __readcr8());

                        // __debugbreak();

                        //
                        // After an unsuccessful call to the ExAllocatePool2 routine by PG,
                        // it will attempt to increment the 32-bit counter of unsuccessful allocations in the context structure,
                        // so it is necessary to set the counter to -1 to perform an overflow of the 32-bit field to keep the counter at 0.
                        //
                        *(PULONG32)((PUCHAR)pPgCtx + 0xA60) = -1UI32; ///< Required as an alternative method to prevent the rescheduling of the PG check procedure execution in the case of (CurrIRQL > APC_LEVEL).

                        break; 
                    }
                }
                else
                {
                    DbgLog("[TheiaPg <+>] VsrExAllocatePool2: Detect possibly PgCaller | pPgCtx: Not-Found\n\n");

                    // __debugbreak();

                    goto JmpToPossibleSleep;
                }         
            }

            if (pPgCtx = SearchPgCtx(pInternalCtx))
            {
                DbgLog("[TheiaPg <+>] VsrExAllocatePool2: Detect PgCtx in CpuExecuteCtx | TCB: 0x%I64X TID: 0x%hX\n", pCurrentObjThread, *pCurrentTID);

                goto JmpDetectPgCtxInCpuExecuteCtx;
            }
                             
            pRetAddrsTrace[i] = pInternalCtx->Rip;
        }
    }
    else { goto JmpDetectNonBackedStack; }

    // if (!IsSleep)
    // {
    //     if (pInputCtx->rax)
    //     {
    //         if ((*(PULONG64)(HrdGetPteVa((PVOID)pInputCtx->rax)) & 0x10801UI64) == 0x801UI64) ///< Checking RWX PTE-Attributes.
    //         {
    //             DbgLog("[TheiaPg <+>] VsrExAllocatePool2: Detect attempt allocate RWX-Page | NoPgArtifacts\n\n");
    // 
    //             if (__readcr8() < DISPATCH_LEVEL) { IsSleep = TRUE; }
    //             else { ExFreePool(pInputCtx->rax); pInputCtx->rax = 0I64; }
    //         }
    //     }
    // }

    g_pTheiaCtx->pMmFreeIndependentPages(pInternalCtx, PAGE_SIZE, 0I64);

    g_pTheiaCtx->pMmFreeIndependentPages(pRetAddrsTrace, PAGE_SIZE, 0I64);

    if (IsSleep) { g_pTheiaCtx->pKeDelayExecutionThread(KernelMode, FALSE, &Timeout); }

    return;
}

/**
* Routine: VsrKiCustomRecurseRoutineX
*
* MaxIRQL: DISPATCH_LEVEL
*
* Public/Private: Public
*
* @param pInputCtx: Context passed from StubCallTrmpln
*
* Description: Similar to VsrExAllocatePool2, but for the most part aims to be called from the DISPATCH_LEVEL-ISR context.
* If VsrKiExecuteAllDpcs skips _KDPC-PG in the CURRENT-DPC-QUEUE, VsrKiCustomRecurseRoutineX is possibly to initiate a rebounce context execution.
*/
volatile VOID VsrKiCustomRecurseRoutineX(IN OUT PINPUTCONTEXT_ICT pInputCtx)
{
    CheckStatusTheiaCtx();

    PCONTEXT pInternalCtx = (PCONTEXT)g_pTheiaCtx->pMmAllocateIndependentPagesEx(PAGE_SIZE, -1I32, 0I64, 0I32);

    if (!pInternalCtx) { DbgLog("[TheiaPg <->] VsrKiCustomRecurseRoutineX: Bad alloc page for InternalCtx\n"); return; }

    pInternalCtx->ContextFlags = CONTEXT_CONTROL;
    pInternalCtx->Rax          = pInputCtx->rax;
    pInternalCtx->Rcx          = pInputCtx->rcx;
    pInternalCtx->Rdx          = pInputCtx->rdx;
    pInternalCtx->Rbx          = pInputCtx->rbx;
    pInternalCtx->Rsi          = pInputCtx->rsi;
    pInternalCtx->Rdi          = pInputCtx->rdi;
    pInternalCtx->R8           = pInputCtx->r8;
    pInternalCtx->R9           = pInputCtx->r9;
    pInternalCtx->R10          = pInputCtx->r10;
    pInternalCtx->R11          = pInputCtx->r11;
    pInternalCtx->R12          = pInputCtx->r12;
    pInternalCtx->R13          = pInputCtx->r13;
    pInternalCtx->R14          = pInputCtx->r14;
    pInternalCtx->R15          = pInputCtx->r15;
    pInternalCtx->Rbp          = pInputCtx->rbp;
    pInternalCtx->Rsp          = pInputCtx->rsp;
    pInternalCtx->Rip          = pInputCtx->rip;
    pInternalCtx->EFlags       = pInputCtx->Rflags;

    PVOID StackHigh, StackLow;

    PVOID pImageBase = NULL;

    PVOID pRuntimeFunction = NULL;

    PVOID pHandlerData = NULL;

    ULONG64 EstablisherFrame = 0UI64;

    PUCHAR pCurrentObjThread = (PUCHAR)__readgsqword(0x188UI32);

    PUSHORT pCurrentTID = (PUSHORT)(pCurrentObjThread + (g_pTheiaCtx->TheiaMetaDataBlock.ETHREAD_Cid_OFFSET + g_pTheiaCtx->TheiaMetaDataBlock.CLIENT_ID_UniqueThread_OFFSET));

    const LONG64 Timeout = (-10000UI64 * 31536000000UI64); ///< 1 year.

    LONG32 SaveRel32Offset = 0I32;

    PVOID pRetAddrCallerPgAccessRoutine = NULL;

    BOOLEAN IsSleep = FALSE;

    StackHigh = *(PVOID*)(pCurrentObjThread + g_pTheiaCtx->TheiaMetaDataBlock.KTHREAD_InitialStack_OFFSET);

    StackLow = *(PVOID*)(pCurrentObjThread + g_pTheiaCtx->TheiaMetaDataBlock.KTHREAD_StackLimit_OFFSET);

    //
    // 3 Iteration-unwind for "default" chain call PgRoutines from DISPATCH_LEVEL-ISR context.  
    // 
    //            2-iteration-unwind         1-iteration-unwind     0-iteration-unwind
    // example: KiProcessExpiredTimerList -> ExpTimerDpcRoutine -> KiCustomAccessRoutine0 -> KiCustomRecurseRoutine0Hook
    //
    for (UCHAR i = 0UI8; i < 3; ++i) 
    {    
        pRuntimeFunction = g_pTheiaCtx->pRtlLookupFunctionEntry(pInternalCtx->Rip, &pImageBase, NULL);

        if (!pRuntimeFunction) ///< If the current routine leaf.
        {
            pInternalCtx->Rip = *(PVOID*)pInternalCtx->Rsp;

            pInternalCtx->Rsp += 8I64;
        }

        g_pTheiaCtx->pRtlVirtualUnwind(0UI32, pImageBase, pInternalCtx->Rip, pRuntimeFunction, pInternalCtx, &pHandlerData, &EstablisherFrame, NULL);

        if (i == 1) { pRetAddrCallerPgAccessRoutine = pInternalCtx->Rip; }
    }

    DbgLog("[TheiaPg <+>] VsrKiCustomRecurseRoutineX: Detect PgCallChain | TCB: 0x%I64X TID: 0x%hX\n", pCurrentObjThread, *pCurrentTID);
    DbgLog("==============================================================\n");
    DbgLog("RAX: 0x%I64X\n", pInternalCtx->Rax);
    DbgLog("RCX: 0x%I64X\n", pInternalCtx->Rcx);
    DbgLog("RDX: 0x%I64X\n", pInternalCtx->Rdx);
    DbgLog("RBX: 0x%I64X\n", pInternalCtx->Rbx);
    DbgLog("RSI: 0x%I64X\n", pInternalCtx->Rsi);
    DbgLog("RDI: 0x%I64X\n", pInternalCtx->Rdi);
    DbgLog("R8:  0x%I64X\n", pInternalCtx->R8);
    DbgLog("R9:  0x%I64X\n", pInternalCtx->R9);
    DbgLog("R10: 0x%I64X\n", pInternalCtx->R10);
    DbgLog("R11: 0x%I64X\n", pInternalCtx->R11);
    DbgLog("R12: 0x%I64X\n", pInternalCtx->R12);
    DbgLog("R13: 0x%I64X\n", pInternalCtx->R13);
    DbgLog("R14: 0x%I64X\n", pInternalCtx->R14);
    DbgLog("R15: 0x%I64X\n", pInternalCtx->R15);
    DbgLog("RSP: 0x%I64X\n", pInternalCtx->Rsp);
    DbgLog("RBP: 0x%I64X\n", pInternalCtx->Rbp);
    DbgLog("RIP: 0x%I64X\n\n", pInternalCtx->Rip);
    DbgLog("RFLAGS: 0x%I64X\n", pInternalCtx->EFlags);
    DbgLog("==============================================================\n\n");

    DbgLog("[TheiaPg <+>] VsrKiCustomRecurseRoutineX: Handling exit phase...\n\n");

    DbgLog("[TheiaPg <+>] VsrKiCustomRecurseRoutineX: Return address CallerPgAccessRoutine: 0x%I64X\n\n", pRetAddrCallerPgAccessRoutine);

    //
    // If IRQL > DISPATCH_LEVEL then the current executor is APC or THREAD, you can enter the current APC/THREAD in the delay.
    //
    if (__readcr8() < DISPATCH_LEVEL)
    {
        DbgLog("[TheiaPg <+>] VsrKiCustomRecurseRoutineX: Enter to dead sleep... | IRQL: 0x%02X\n\n", __readcr8());

        IsSleep = TRUE;
    }
    else { DbgLog("[TheiaPg <+>] VsrKiCustomRecurseRoutineX: Rebound execution context... | IRQL: 0x%02X\n\n", __readcr8()); }
     
    g_pTheiaCtx->pMmFreeIndependentPages(pInternalCtx, PAGE_SIZE, 0I64);

    if (IsSleep) { g_pTheiaCtx->pKeDelayExecutionThread(KernelMode, FALSE, &Timeout); }

    //
    // When analyzing CallerAccessRoutinePg is called via cfg-jmp (__guard_retpoline_indirect_cfg_rax),
    // this means that the return address of CallerCallerPgAccessRoutine (example KiProcessExpiredTimerList) may not be located in stack-calls with cfg-jmp.
    //
    pInputCtx->rax    = pInternalCtx->Rsp;
    pInputCtx->rcx    = pInternalCtx->Rip;
    pInputCtx->rdx    = pInternalCtx->Rdx;
    pInputCtx->rbx    = pInternalCtx->Rbx;
    pInputCtx->rsi    = pInternalCtx->Rsi;
    pInputCtx->rdi    = pInternalCtx->Rdi;
    pInputCtx->r8     = pInternalCtx->R8;
    pInputCtx->r9     = pInternalCtx->R9;
    pInputCtx->r10    = pInternalCtx->R10;
    pInputCtx->r11    = pInternalCtx->R11;
    pInputCtx->r12    = pInternalCtx->R12;
    pInputCtx->r13    = pInternalCtx->R13;
    pInputCtx->r14    = pInternalCtx->R14;
    pInputCtx->r15    = pInternalCtx->R15;
    pInputCtx->rbp    = pInternalCtx->Rbp;
    pInputCtx->Rflags = pInternalCtx->EFlags;

    return;
}
