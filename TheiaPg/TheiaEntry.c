#include "LinkHeader.h"

const UCHAR THEIA_ENTRY_DATA_KIEXECUTEALLDPCS_SIG[] =
{
  0x4c,0x8d, 0x80, 0x2c, 0x01, 0x00, 0x00, // lea     r8, [rax+12Ch]
  0x4f, 0x8d, 0x04, 0x40,                  // lea     r8, [r8+r8*2]
  0x49, 0xc1, 0xe0, 0x04                   // shl     r8, 4
};
const UCHAR THEIA_ENTRY_DATA_KIEXECUTEALLDPCS_MASK[sizeof THEIA_ENTRY_DATA_KIEXECUTEALLDPCS_SIG] = { "xxxxxxxxxxxxxxx" };

const UCHAR THEIA_ENTRY_DATA_EXALLOCATEPOOL2_SIG[] =
{
  0x41, 0x5f,                              // pop     r15
  0x41, 0x5e,                              // pop     r14 
  0x41, 0x5d,                              // pop     r13
  0x41, 0x5c,                              // pop     r12
  0x5f,                                    // pop     rdi
  0x5e,                                    // pop     rsi
  0x5d,                                    // pop     rbp
  0xc3                                     // ret
};
const UCHAR THEIA_ENTRY_DATA_EXALLOCATEPOOL2_MASK[sizeof THEIA_ENTRY_DATA_EXALLOCATEPOOL2_SIG] = { "xxxxxxxxxxxx" };

#define ALLIGNMENT_ICT_KIEXECUTEALLDPCS 2I8

#define ALLIGNMENT_ICT_EXALLOCATEPOOL2  0I8

/**
* Routine: TheiaEntry
*
* MaxIRQL: DISPATCH_LEVEL
*
* Public/Private: Public
*
* @param NoParams
*
* Description: MainEntry routine.
*/
VOID TheiaEntry(VOID)
{
    #define ERROR_EXECUTE_THEIA_ENTRY 0xd1baa81aUI32

    const UCHAR HandlerVsrKiExecuteAllDpcs[] =
    {
       0x4c,0x8d, 0x80, 0x2c, 0x01, 0x00, 0x00, // lea     r8, [rax+12Ch]
       0x4f, 0x8d, 0x04, 0x40,                  // lea     r8, [r8+r8*2]
       0x49, 0xc1, 0xe0, 0x04                   // shl     r8, 4
    };

    const UCHAR HandlerVsrExAllocatePool2[] =
    {     
       0x48, 0x83, 0xc4, 0x10,                 // add     rsp, 010h
       0x41, 0x5f,                             // pop     r15
       0x41, 0x5e,                             // pop     r14 
       0x41, 0x5d,                             // pop     r13
       0x41, 0x5c,                             // pop     r12
       0x5f,                                   // pop     rdi
       0x5e,                                   // pop     rsi
       0x5d,                                   // pop     rbp
       0xc3                                    // ret
    };

    const UCHAR HandlerVsrKiCustomRecurseRoutineX[] =
    {
      0x48, 0x89, 0xc4,                        // mov    rsp,rax
      0xff, 0xe1,                              // jmp    rcx
      0xcc,                                    // int3
      0xcc,                                    // int3
      0xcc                                     // int3
    };

    const UCHAR RetByte = 0xC3UI8;
  
    const UCHAR NopOpcode = 0x90UI8;

    const CHAR StopSig[4] = { 0xCC,0xCC,0xCC,0xCC };

    ICT_DATA_RELATED RelatedDataICT;

    UNICODE_STRING PageNonLarge = { 0 };

    RtlCreateUnicodeString(&PageNonLarge, L"\xff");

    g_DieNonLargePage = PageNonLarge.Buffer;

    RtlCreateUnicodeString(&PageNonLarge, L"\xff");

    g_pSpiiNonLargePage = PageNonLarge.Buffer;

    RtlCreateUnicodeString(&PageNonLarge, L"\xff");

    g_pSpirNonLargePage = PageNonLarge.Buffer;

    InitTheiaContext();

    HrdIndpnRWVMemory(MEM_INDPN_RW_WRITE_OP_BIT, g_pTheiaCtx->pKiSwInterruptDispatch, &RetByte, 1UI32);

    DbgLog("[TheiaPg <+>] TheiaEntry: FixKiSwInterruptDispatch\n");
    
    //
    // Nulling MaxDataSize (g_pPgCtx) 
    //
    *(PULONG64)g_pTheiaCtx->ppMaxDataSize = NULL; ///< pp: pointer to pointer.
    
    DbgLog("[TheiaPg <+>] TheiaEntry: FixMaxDataSize\n");

    HrdIndpnRWVMemory(MEM_INDPN_RW_WRITE_OP_BIT, g_pTheiaCtx->pKiDispatchCallout, &RetByte, 1UI32);

    DbgLog("[TheiaPg <+>] TheiaEntry: FixKiDispatchCallout\n");

    RelatedDataICT.pHookRoutine = &VsrKiExecuteAllDpcs;
    RelatedDataICT.pBasePatch = _SearchPatternInRegion(NULL, SPIR_NO_OPTIONAL, g_pTheiaCtx->pKiExecuteAllDpcs, THEIA_ENTRY_DATA_KIEXECUTEALLDPCS_SIG, THEIA_ENTRY_DATA_KIEXECUTEALLDPCS_MASK, &StopSig, sizeof StopSig);

    if (!RelatedDataICT.pBasePatch)
    {
        DbgLog("[TheiaPg <->] TheiaEntry: Base for trampoline to stub VsrKiExecuteAllDpcs not found\n");

        goto DIE_CALL_ERROR_EXECUTE_THEIA_ENTRY;
    }
    else { DbgLog("[TheiaPg <+>] TheiaEntry: VsrKiExecuteAllDpcs is init\n"); }
    
    RelatedDataICT.pHandlerHook = &HandlerVsrKiExecuteAllDpcs;
    RelatedDataICT.LengthHandler = sizeof HandlerVsrKiExecuteAllDpcs;
    RelatedDataICT.LengthAllignment = ALLIGNMENT_ICT_KIEXECUTEALLDPCS;

    HkInitCallTrmpln(&RelatedDataICT);

    RelatedDataICT.pHookRoutine = &VsrExAllocatePool2; 
    RelatedDataICT.pBasePatch = _SearchPatternInRegion(NULL, SPIR_NO_OPTIONAL, g_pTheiaCtx->pExAllocatePool2, THEIA_ENTRY_DATA_EXALLOCATEPOOL2_SIG, THEIA_ENTRY_DATA_EXALLOCATEPOOL2_MASK, &StopSig, sizeof StopSig);

    if (!RelatedDataICT.pBasePatch)
    {
        DbgLog("[TheiaPg <->] TheiaEntry: Base for trampoline to stub VsrExAllocatePool2 not found\n");

        goto DIE_CALL_ERROR_EXECUTE_THEIA_ENTRY;
    }
    else { DbgLog("[TheiaPg <+>] TheiaEntry: VsrExAllocatePool2 is init\n"); }

    RelatedDataICT.pHandlerHook = &HandlerVsrExAllocatePool2;
    RelatedDataICT.LengthHandler = sizeof HandlerVsrExAllocatePool2;
    RelatedDataICT.LengthAllignment = ALLIGNMENT_ICT_EXALLOCATEPOOL2;

    HkInitCallTrmpln(&RelatedDataICT);

    do
    {
        LONG32 SaveRel32Offset = 0I32;

        PVOID pCurrentRecurseRoutine = NULL;

        for (BOOLEAN i = FALSE; ; )
        {
            if (!i)
            {
                i = TRUE;

                RelatedDataICT.pHookRoutine = &VsrKiCustomRecurseRoutineX;
                RelatedDataICT.pBasePatch = ((PUCHAR)g_pTheiaCtx->pKiCustomRecurseRoutineX + 4);
                RelatedDataICT.pHandlerHook = &HandlerVsrKiCustomRecurseRoutineX;
                RelatedDataICT.LengthHandler = sizeof HandlerVsrKiCustomRecurseRoutineX;
                RelatedDataICT.LengthAllignment = 0UI32;

                pCurrentRecurseRoutine = g_pTheiaCtx->pKiCustomRecurseRoutineX;

                SaveRel32Offset = *(PLONG32)((PUCHAR)pCurrentRecurseRoutine + 9);

                pCurrentRecurseRoutine = (PVOID)(((ULONG64)pCurrentRecurseRoutine + 13) + ((SaveRel32Offset < 0I32) ? ((ULONG64)SaveRel32Offset | 0xffffffff00000000UI64) : (ULONG64)SaveRel32Offset));
            }
            else
            {
                if (pCurrentRecurseRoutine == g_pTheiaCtx->pKiCustomRecurseRoutineX) { break; }

                RelatedDataICT.pBasePatch = ((PUCHAR)pCurrentRecurseRoutine + 4);

                SaveRel32Offset = *(PLONG32)((PUCHAR)pCurrentRecurseRoutine + 9);

                pCurrentRecurseRoutine = (PVOID)(((ULONG64)pCurrentRecurseRoutine + 13) + ((SaveRel32Offset < 0I32) ? ((ULONG64)SaveRel32Offset | 0xffffffff00000000UI64) : (ULONG64)SaveRel32Offset));
            }

            HkInitCallTrmpln(&RelatedDataICT);
        };

        DbgLog("[TheiaPg <+>] TheiaEntry: VsrKiCustomRecurseRoutineX is init\n\n");

    } while (FALSE);

    InitSearchPgSysThread(); ///< Calling initalizer sys-threads-walk.

    if (g_VolatileNullByte) { DIE_CALL_ERROR_EXECUTE_THEIA_ENTRY: DieDispatchIntrnlError(ERROR_EXECUTE_THEIA_ENTRY); }

    return;
}
