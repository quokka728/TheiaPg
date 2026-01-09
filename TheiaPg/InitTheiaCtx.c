#include "LinkHeader.h"

#pragma section("INIT",read,execute)

/*++
* Routine: InitTheiaMetaDataBlock
*
* MaxIRQL: DISPATCH_LEVEL (If IRQL > DISPATCH_LEVEL then TheiaMetaDataBlock must be NonPaged)
* 
* Public/Private: Public
*
* @param pTheiaMetaDataBlock: TheiaMetaDataBlock
* 
* Description: Initializator of metadata KernelNT in TheiaCtx.
--*/
VOID InitTheiaMetaDataBlock(IN OUT PTHEIA_METADATA_BLOCK pTheiaMetaDataBlock)
{
    #define ERROR_INIT_META_DATA_BLOCK 0xdbdea4c3UI32

    #define ITMDB_LOCAL_CONTEXT_MMISADDRESSVALID 0

    #define ITMDB_LOCAL_CONTEXT_MMISNONPAGEDSYSTEMADDRESSVALID 1

    PVOID(__fastcall *ITMDBCtx[3])(PVOID, ...) = { 0 }; ///< Routine is critical, so it should not depend on gTheiaCtx.

    UNICODE_STRING StrMmIsAddressValid = { 0 };                                                                  
    
    StrMmIsAddressValid.Buffer = L"MmIsAddressValid";                                                            
    
    StrMmIsAddressValid.Length = (USHORT)((wcslen(StrMmIsAddressValid.Buffer)) * 2);                             
    
    StrMmIsAddressValid.MaximumLength = (StrMmIsAddressValid.Length + 2);      

    ITMDBCtx[ITMDB_LOCAL_CONTEXT_MMISADDRESSVALID] = MmGetSystemRoutineAddress(&StrMmIsAddressValid);
    
    UNICODE_STRING StrMmIsNonPagedSystemAddressValid = { 0 };                                                    
    
    StrMmIsNonPagedSystemAddressValid.Buffer = L"MmIsNonPagedSystemAddressValid";                                
    
    StrMmIsNonPagedSystemAddressValid.Length = (USHORT)((wcslen(StrMmIsNonPagedSystemAddressValid.Buffer)) * 2); 
    
    StrMmIsNonPagedSystemAddressValid.MaximumLength = (StrMmIsNonPagedSystemAddressValid.Length + 2);   

    ITMDBCtx[ITMDB_LOCAL_CONTEXT_MMISNONPAGEDSYSTEMADDRESSVALID] = MmGetSystemRoutineAddress(&StrMmIsNonPagedSystemAddressValid);

    if (!pTheiaMetaDataBlock || !((__readcr8() <= DISPATCH_LEVEL) ? ITMDBCtx[ITMDB_LOCAL_CONTEXT_MMISADDRESSVALID](pTheiaMetaDataBlock) : ITMDBCtx[ITMDB_LOCAL_CONTEXT_MMISNONPAGEDSYSTEMADDRESSVALID](pTheiaMetaDataBlock)))
    {
        DbgLog("[TheiaPg <->] InitMetaDataBlock: Invalid &TheiaMetaDataBlock\n");

        goto DIE_CALL_ERROR_INIT_META_DATA_BLOCK;
    }

    if (NtBuildNumber >= 26200UI32) ///< Windows 11 25h2
    {
        //
        // KOFFSETS ======================================================================================
        //
        pTheiaMetaDataBlock->KPCR_TssBase_OFFSET                       = 0x08UI32;
        pTheiaMetaDataBlock->KPCR_Prcb_OFFSET                          = 0x180UI32;

        pTheiaMetaDataBlock->KPRCB_CurrentThread_OFFSET                = 0x08UI32;
        pTheiaMetaDataBlock->KPRCB_IdleThread_OFFSET                   = 0x18UI32;
        pTheiaMetaDataBlock->KPRCB_DpcData0_OFFSET                     = 0x3840UI32;
        pTheiaMetaDataBlock->KPRCB_DpcData1_OFFSET                     = 0x3870UI32;

        pTheiaMetaDataBlock->ETHREAD_Cid_OFFSET                        = 0x508UI32;
        pTheiaMetaDataBlock->ETHREAD_Win32StartAddress_OFFSET          = 0x560UI32;

        pTheiaMetaDataBlock->CLIENT_ID_UniqueProcess_OFFSET            = 0x00UI32;
        pTheiaMetaDataBlock->CLIENT_ID_UniqueThread_OFFSET             = 0x08UI32;

        pTheiaMetaDataBlock->KTHREAD_InitialStack_OFFSET               = 0x28UI32;
        pTheiaMetaDataBlock->KTHREAD_StackLimit_OFFSET                 = 0x30UI32;
        pTheiaMetaDataBlock->KTHREAD_StackBase_OFFSET                  = 0x38UI32;
        pTheiaMetaDataBlock->KTHREAD_KernelStack_OFFSET                = 0x58UI32;
        pTheiaMetaDataBlock->KTHREAD_MiscFlags_OFFSET                  = 0x74UI32;
        pTheiaMetaDataBlock->KTHREAD_ApcState_OFFSET                   = 0x90UI32;
        pTheiaMetaDataBlock->KTHREAD_ContextSwitches_OFFSET            = 0x154UI32;
        pTheiaMetaDataBlock->KTHREAD_WaitTime_OFFSET                   = 0x1b4UI32;
        pTheiaMetaDataBlock->KTHREAD_KernelTime_OFFSET                 = 0x28cUI32;
        pTheiaMetaDataBlock->KTHREAD_CombinedApcDisable_OFFSET         = 0x1e4UI32;
        pTheiaMetaDataBlock->KTHREAD_ThreadListEntry_OFFSET            = 0x2f8UI32;

        pTheiaMetaDataBlock->KAPC_STATE_ApcListHead0_OFFSET            = 0x0UI32;
        pTheiaMetaDataBlock->KAPC_STATE_ApcListHead1_OFFSET            = 0x10UI32;

        pTheiaMetaDataBlock->EPROCESS_KPROCESS_OFFSET                  = 0x00UI32;
        pTheiaMetaDataBlock->EPROCESS_ActiveProcessLinks_OFFSET        = 0x1d8UI32;
        pTheiaMetaDataBlock->EPROCESS_Peb_OFFSET                       = 0x2e0UI32;
        pTheiaMetaDataBlock->EPROCESS_ImageFileName_OFFSET             = 0x338UI32;
        pTheiaMetaDataBlock->EPROCESS_ThreadListHead                   = 0x370UI32;
        pTheiaMetaDataBlock->EPROCESS_ProtectionEprocess_OFFSET        = 0x5faUI32;

        pTheiaMetaDataBlock->KPROCESS_DirectoryTableBase_OFFSET        = 0x28UI32;

        pTheiaMetaDataBlock->PEB_Ldr_OFFSET                            = 0x18UI32;

        pTheiaMetaDataBlock->PEB_LDR_DATA_InLoadOrderModuleList_OFFSET = 0x10UI32;

        pTheiaMetaDataBlock->KLDR_InLoadOrderList_OFFSET               = 0x00UI32;
        pTheiaMetaDataBlock->KLDR_DllBase_OFFSET                       = 0x30UI32;
        pTheiaMetaDataBlock->KLDR_DllName_OFFSET                       = 0x58UI32;

        pTheiaMetaDataBlock->LDR_InLoadOrderList_OFFSET                = 0x00UI32;
        pTheiaMetaDataBlock->LDR_DllBase_OFFSET                        = 0x30UI32;
        pTheiaMetaDataBlock->LDR_DllName_OFFSET                        = 0x58UI32;

        //
        // KROUTINES_SIG_MASK ============================================================================
        //
        pTheiaMetaDataBlock->KIEXECUTEALLDPCS_SIG = &_25h2_w11_KiExecuteAllDpcs_SIG;
        pTheiaMetaDataBlock->KIEXECUTEALLDPCS_MASK = &_25h2_w11_KiExecuteAllDpcs_MASK;

        pTheiaMetaDataBlock->MMALLOCATEINDEPENDENTPAGESEX_SIG = &_25h2_w11_MmAllocateIndependentPagesEx_SIG;
        pTheiaMetaDataBlock->MMALLOCATEINDEPENDENTPAGESEX_MASK = &_25h2_w11_MmAllocateIndependentPagesEx_MASK;

        pTheiaMetaDataBlock->MMFREEINDEPENDENTPAGESEX_SIG = &_25h2_w11_MmFreeIndependentPages_SIG;
        pTheiaMetaDataBlock->MMFREEINDEPENDENTPAGESEX_MASK = &_25h2_w11_MmFreeIndependentPages_MASK;

        pTheiaMetaDataBlock->KICUSTOMRECURSEROUTINEX_SIG = &_25h2_w11_KiCustomRecurseRoutineX_SIG;
        pTheiaMetaDataBlock->KICUSTOMRECURSEROUTINEX_MASK = &_25h2_w11_KiCustomRecurseRoutineX_MASK;

        pTheiaMetaDataBlock->KIDISPATCHCALLOUT_SIG = &_25h2_w11_KiDispatchCallout_SIG;
        pTheiaMetaDataBlock->KIDISPATCHCALLOUT_MASK = &_25h2_w11_KiDispatchCallout_MASK;
    }
    else
    {
        DbgLog("[TheiaPg <->] InitMetaDataBlock: OsVersion 0xI32X non-supported\n", NtBuildNumber);

        goto DIE_CALL_ERROR_INIT_META_DATA_BLOCK;
    }
          
    pTheiaMetaDataBlock->CompleteSignatureTMDB = COMPLETE_SIGNATURE_TMDB; ///< Signature writing of the status of TheiaMetaDataBlock

    if (g_VolatileNullByte) { DIE_CALL_ERROR_INIT_META_DATA_BLOCK: DieDispatchIntrnlError(ERROR_INIT_META_DATA_BLOCK); }
    
    return;
}

/*++
* Routine: InitTheiaContext
*
* MaxIRQL: DISPATCH_LEVEL
*
* Public/Private: Public
*
* @param NoParams
* 
* Description: Is currently engaged in the initialization TheiaCtx.
--*/
VOID InitTheiaContext(VOID)
{  
    #define ERROR_INIT_THEIA_CONTEXT 0x4a9d62b5UI32

    #define ERROR_DOUBLE_INIT_THEIA_CONTEXT 0x62c7bf9fUI32

    // OtherData =============================================================================================================++
                                                                                                                              //
    const UCHAR PgXorRoutineSig[52] = ///< The byte pattern of the top of PgCtx is needed for routines of PgCtx interceptors. //
    {                                                                                                                         //
     0x2e, 0x48, 0x31, 0x11,                                                                                                  //
     0x48, 0x31, 0x51, 0x08,                                                                                                  //
     0x48, 0x31, 0x51, 0x10,                                                                                                  //
     0x48, 0x31, 0x51, 0x18,                                                                                                  //
     0x48, 0x31, 0x51, 0x20,                                                                                                  //
     0x48, 0x31, 0x51, 0x28,                                                                                                  //
     0x48, 0x31, 0x51, 0x30,                                                                                                  //
     0x48, 0x31, 0x51, 0x38,                                                                                                  //
     0x48, 0x31, 0x51, 0x40,                                                                                                  //
     0x48, 0x31, 0x51, 0x48,                                                                                                  //
     0x48, 0x31, 0x51, 0x50,                                                                                                  //
     0x48, 0x31, 0x51, 0x58,                                                                                                  //
     0x48, 0x31, 0x51, 0x60                                                                                                   //
    };                                                                                                                        //
                                                                                                                              //
    PVOID(__stdcall*pExAllocatePool2)(POOL_FLAGS Flags, SIZE_T NumberOfBytes, ULONG Tag);                                     //
                                                                                                                              //
    // =======================================================================================================================++

    // RelatedData =====================++
                                        //
    ULONG64 RelatedDataSPII[4] = { 0 }; //
                                        //
    // =================================++    
                             
    // KiSwInterruptDispatch/MaxDataSize BlockVars =++
                                                    //
    UCHAR IDTR[10];                                 //
                                                    //
    PKIDTENTRY64 pSwKIDTENTRY64 = NULL;             //
                                                    //
    PVOID pKiSwInterruptDispatch = NULL;            //  
                                                    //
    LONG32 SaveRel32Offset = 0I32;                  //
                                                    //         
    // =============================================++
    
    // NamesRequiredRoutines ====================================================================================++
                                                                                                                 //
    UNICODE_STRING StrKeIpiGenericCall = { 0 };                                                                  //
                                                                                                                 //
    StrKeIpiGenericCall.Buffer = L"KeIpiGenericCall";                                                            //
                                                                                                                 //
    StrKeIpiGenericCall.Length = (USHORT)((wcslen(StrKeIpiGenericCall.Buffer)) * 2);                             //
                                                                                                                 //
    StrKeIpiGenericCall.MaximumLength = (StrKeIpiGenericCall.Length + 2);                                        //
                                                                                                                 //
    UNICODE_STRING StrMmGetPhysicalAddress = { 0 };                                                              //
                                                                                                                 //
    StrMmGetPhysicalAddress.Buffer = L"MmGetPhysicalAddress";                                                    //
                                                                                                                 //
    StrMmGetPhysicalAddress.Length = (USHORT)((wcslen(StrMmGetPhysicalAddress.Buffer)) * 2);                     //
                                                                                                                 //
    StrMmGetPhysicalAddress.MaximumLength = (StrMmGetPhysicalAddress.Length + 2);                                //
                                                                                                                 //
    UNICODE_STRING StrMmMapIoSpaceEx = { 0 };                                                                    //
                                                                                                                 //
    StrMmMapIoSpaceEx.Buffer = L"MmMapIoSpaceEx";                                                                //
                                                                                                                 //
    StrMmMapIoSpaceEx.Length = (USHORT)((wcslen(StrMmMapIoSpaceEx.Buffer)) * 2);                                 //
                                                                                                                 //
    StrMmMapIoSpaceEx.MaximumLength = (StrMmMapIoSpaceEx.Length + 2);                                            //
                                                                                                                 //
    UNICODE_STRING StrMmUnmapIoSpace = { 0 };                                                                    //
                                                                                                                 //
    StrMmUnmapIoSpace.Buffer = L"MmUnmapIoSpace";                                                                //
                                                                                                                 //
    StrMmUnmapIoSpace.Length = (USHORT)((wcslen(StrMmUnmapIoSpace.Buffer)) * 2);                                 //
                                                                                                                 //
    StrMmUnmapIoSpace.MaximumLength = (StrMmUnmapIoSpace.Length + 2);                                            //
                                                                                                                 //
    UNICODE_STRING StrRtlLookupFunctionEntry = { 0 };                                                            //
                                                                                                                 //
    StrRtlLookupFunctionEntry.Buffer = L"RtlLookupFunctionEntry";                                                //
                                                                                                                 //
    StrRtlLookupFunctionEntry.Length = (USHORT)((wcslen(StrRtlLookupFunctionEntry.Buffer)) * 2);                 //
                                                                                                                 //
    StrRtlLookupFunctionEntry.MaximumLength = (StrRtlLookupFunctionEntry.Length + 2);                            //
                                                                                                                 //
    UNICODE_STRING StrRtlVirtualUnwind = { 0 };                                                                  //
                                                                                                                 //
    StrRtlVirtualUnwind.Buffer = L"RtlVirtualUnwind";                                                            //
                                                                                                                 //
    StrRtlVirtualUnwind.Length = (USHORT)((wcslen(StrRtlVirtualUnwind.Buffer)) * 2);                             //
                                                                                                                 //
    StrRtlVirtualUnwind.MaximumLength = (StrRtlVirtualUnwind.Length + 2);                                        //
                                                                                                                 //
    UNICODE_STRING StrKeInitializeApc = { 0 };                                                                   //
                                                                                                                 //
    StrKeInitializeApc.Buffer = L"KeInitializeApc";                                                              //
                                                                                                                 //
    StrKeInitializeApc.Length = (USHORT)((wcslen(StrKeInitializeApc.Buffer)) * 2);                               //
                                                                                                                 //
    StrKeInitializeApc.MaximumLength = (StrKeInitializeApc.Length + 2);                                          //
                                                                                                                 //
    UNICODE_STRING StrKeInsertQueueApc = { 0 };                                                                  //
                                                                                                                 //
    StrKeInsertQueueApc.Buffer = L"KeInsertQueueApc";                                                            //
                                                                                                                 //
    StrKeInsertQueueApc.Length = (USHORT)((wcslen(StrKeInsertQueueApc.Buffer)) * 2);                             //
                                                                                                                 //
    StrKeInsertQueueApc.MaximumLength = (StrKeInsertQueueApc.Length + 2);                                        //
                                                                                                                 //
    UNICODE_STRING StrKeDelayExecutionThread = { 0 };                                                            //
                                                                                                                 //
    StrKeDelayExecutionThread.Buffer = L"KeDelayExecutionThread";                                                //
                                                                                                                 //
    StrKeDelayExecutionThread.Length = (USHORT)((wcslen(StrKeDelayExecutionThread.Buffer)) * 2);                 //
                                                                                                                 //
    StrKeDelayExecutionThread.MaximumLength = (StrKeDelayExecutionThread.Length + 2);                            //
                                                                                                                 //
    UNICODE_STRING StrMmIsAddressValid = { 0 };                                                                  //
                                                                                                                 //
    StrMmIsAddressValid.Buffer = L"MmIsAddressValid";                                                            //
                                                                                                                 //
    StrMmIsAddressValid.Length = (USHORT)((wcslen(StrMmIsAddressValid.Buffer)) * 2);                             //
                                                                                                                 //
    StrMmIsAddressValid.MaximumLength = (StrMmIsAddressValid.Length + 2);                                        //
                                                                                                                 //
    UNICODE_STRING StrMmIsNonPagedSystemAddressValid = { 0 };                                                    //
                                                                                                                 //
    StrMmIsNonPagedSystemAddressValid.Buffer = L"MmIsNonPagedSystemAddressValid";                                //
                                                                                                                 //
    StrMmIsNonPagedSystemAddressValid.Length = (USHORT)((wcslen(StrMmIsNonPagedSystemAddressValid.Buffer)) * 2); //
                                                                                                                 //
    StrMmIsNonPagedSystemAddressValid.MaximumLength = (StrMmIsNonPagedSystemAddressValid.Length + 2);            //
                                                                                                                 //
    UNICODE_STRING StrExAllocatePool2 = { 0 };                                                                   //
                                                                                                                 //
    StrExAllocatePool2.Buffer = L"ExAllocatePool2";                                                              //
                                                                                                                 //
    StrExAllocatePool2.Length = (USHORT)((wcslen(StrExAllocatePool2.Buffer)) * 2);                               //
                                                                                                                 //
    StrExAllocatePool2.MaximumLength = (StrExAllocatePool2.Length + 2);                                          //
                                                                                                                 //
    UNICODE_STRING StrKeInitializeCrashDumpHeader = { 0 };                                                       //
                                                                                                                 //
    StrKeInitializeCrashDumpHeader.Buffer = L"KeInitializeCrashDumpHeader";                                      //
                                                                                                                 //
    StrKeInitializeCrashDumpHeader.Length = (USHORT)((wcslen(StrKeInitializeCrashDumpHeader.Buffer)) * 2);       //
                                                                                                                 //
    StrKeInitializeCrashDumpHeader.MaximumLength = (StrKeInitializeCrashDumpHeader.Length + 2);                  //
                                                                                                                 //
    UNICODE_STRING StrIoCancelIrp = { 0 };                                                                       //
                                                                                                                 //
    StrIoCancelIrp.Buffer = L"IoCancelIrp";                                                                      //
                                                                                                                 //
    StrIoCancelIrp.Length = (USHORT)((wcslen(StrIoCancelIrp.Buffer)) * 2);                                       //
                                                                                                                 //
    StrIoCancelIrp.MaximumLength = (StrIoCancelIrp.Length + 2);                                                  //
                                                                                                                 //
    UNICODE_STRING StrKeBugCheckEx = { 0 };                                                                      //
                                                                                                                 //
    StrKeBugCheckEx.Buffer = L"KeBugCheckEx";                                                                    //
                                                                                                                 //
    StrKeBugCheckEx.Length = (USHORT)((wcslen(StrKeBugCheckEx.Buffer)) * 2);                                     //
                                                                                                                 //
    StrKeBugCheckEx.MaximumLength = (StrKeBugCheckEx.Length + 2);                                                //
                                                                                                                 //
    UNICODE_STRING StrPsIsSystemThread = { 0 };                                                                  //
                                                                                                                 //
    StrPsIsSystemThread.Buffer = L"PsIsSystemThread";                                                            //
                                                                                                                 //
    StrPsIsSystemThread.Length = (USHORT)((wcslen(StrPsIsSystemThread.Buffer)) * 2);                             //
                                                                                                                 //
    StrPsIsSystemThread.MaximumLength = (StrPsIsSystemThread.Length + 2);                                        //
                                                                                                                 //
    UNICODE_STRING StrPsLookupThreadByThreadId = { 0 };                                                          //
                                                                                                                 //
    StrPsLookupThreadByThreadId.Buffer = L"PsLookupThreadByThreadId";                                            //
                                                                                                                 //
    StrPsLookupThreadByThreadId.Length = (USHORT)((wcslen(StrPsLookupThreadByThreadId.Buffer)) * 2);             //
                                                                                                                 //
    StrPsLookupThreadByThreadId.MaximumLength = (StrPsLookupThreadByThreadId.Length + 2);                        //
                                                                                                                 //
    UNICODE_STRING StrObfDereferenceObject = { 0 };                                                              //
                                                                                                                 //
    StrObfDereferenceObject.Buffer = L"ObfDereferenceObject";                                                    //
                                                                                                                 //
    StrObfDereferenceObject.Length = (USHORT)((wcslen(StrObfDereferenceObject.Buffer)) * 2);                     //
                                                                                                                 //
    StrObfDereferenceObject.MaximumLength = (StrObfDereferenceObject.Length + 2);                                //
                                                                                                                 //
    // ==========================================================================================================++
                                                                                                                   
    // KdDebuggerBlock BlockVars ===================++                                                             
                                                    //                                                             
    PKDDEBUGGER_DATA64 pKdDebuggerDataBlock = NULL; //                                                             
                                                    //                                                             
    // =============================================++                                                             
   
    if (g_CompleteInitTheiaCtx)
    {
        DbgLog("[TheiaPg <->] InitTheiaContext: Attempt double init gTheiaCtx\n");

        goto DIE_CALL_ERROR_DOUBLE_INIT_THEIA_CONTEXT;
    }

    if (__readcr8() > DISPATCH_LEVEL)
    {
        DbgLog("[TheiaPg <->] InitTheiaContext: Inadmissible IRQL\n");

        goto DIE_CALL_ERROR_INIT_THEIA_CONTEXT;
    }

    // AllocateTheiaCtx ===============================================================================================================================================++
                                                                                                                                                                       //
    pExAllocatePool2 = (PVOID)MmGetSystemRoutineAddress(&StrExAllocatePool2);                                                                                          //
                                                                                                                                                                       //                                                                                                                                                          
    g_pTheiaCtx = (PTHEIA_CONTEXT)pExAllocatePool2(POOL_FLAG_NON_PAGED, (PAGE_SIZE * ((((0x1000 - 1) + sizeof(THEIA_CONTEXT)) & ~(0x1000 - 1)) / PAGE_SIZE)), 'UTR$'); //
                                                                                                                                                                       //       
    if (!g_pTheiaCtx)                                                                                                                                                  //
    {                                                                                                                                                                  //
        DbgLog("[TheiaPg <->] InitTheiaContext: Bad alloc page for g_pTheiaCtx\n");                                                                                    //
                                                                                                                                                                       //
        goto DIE_CALL_ERROR_INIT_THEIA_CONTEXT;                                                                                                                          //
    }                                                                                                                                                                  //
                                                                                                                                                                       //
    // ================================================================================================================================================================++

    //
    // Initialization A2-Block
    //
    InitTheiaMetaDataBlock(&g_pTheiaCtx->TheiaMetaDataBlock);

    //
    // Initialization A0-Block
    //
    g_pTheiaCtx->pKiExecuteAllDpcs = _SearchPatternInImg(NULL, SPII_NO_OPTIONAL, PsInitialSystemProcess, ".text", NULL, g_pTheiaCtx->TheiaMetaDataBlock.KIEXECUTEALLDPCS_SIG, g_pTheiaCtx->TheiaMetaDataBlock.KIEXECUTEALLDPCS_MASK);

    if (!g_pTheiaCtx->pKiExecuteAllDpcs)
    {
        DbgLog("[TheiaPg <->] InitTheiaContext: BaseVa KiExecuteAllDpcs not found\n");

        goto DIE_CALL_ERROR_INIT_THEIA_CONTEXT;
    }

    g_pTheiaCtx->pKiCustomRecurseRoutineX = _SearchPatternInImg(NULL, SPII_NO_OPTIONAL, PsInitialSystemProcess, ".text", NULL, g_pTheiaCtx->TheiaMetaDataBlock.KICUSTOMRECURSEROUTINEX_SIG, g_pTheiaCtx->TheiaMetaDataBlock.KICUSTOMRECURSEROUTINEX_MASK);

    if (!g_pTheiaCtx->pKiCustomRecurseRoutineX)
    {
        DbgLog("[TheiaPg <->] InitTheiaContext: BaseVa KiCustomRecurseRoutineX not found\n");

        goto DIE_CALL_ERROR_INIT_THEIA_CONTEXT;
    }

    g_pTheiaCtx->pKiDispatchCallout = _SearchPatternInImg(NULL, SPII_NO_OPTIONAL, PsInitialSystemProcess, ".text", NULL, g_pTheiaCtx->TheiaMetaDataBlock.KIDISPATCHCALLOUT_SIG, g_pTheiaCtx->TheiaMetaDataBlock.KIDISPATCHCALLOUT_MASK);

    if (!g_pTheiaCtx->pKiDispatchCallout)
    {
        DbgLog("[TheiaPg <->] InitTheiaContext: BaseVa KiDispatchCallout not found\n");

        goto DIE_CALL_ERROR_INIT_THEIA_CONTEXT;
    }

    g_pTheiaCtx->pMmAllocateIndependentPagesEx = _SearchPatternInImg(NULL, SPII_NO_OPTIONAL, PsInitialSystemProcess, "PAGE", NULL, g_pTheiaCtx->TheiaMetaDataBlock.MMALLOCATEINDEPENDENTPAGESEX_SIG, g_pTheiaCtx->TheiaMetaDataBlock.MMALLOCATEINDEPENDENTPAGESEX_MASK);

    if (!g_pTheiaCtx->pMmAllocateIndependentPagesEx)
    {
        DbgLog("[TheiaPg <->] InitTheiaContext: BaseVa MmAllocateIndependentPagesEx not found\n");

        goto DIE_CALL_ERROR_INIT_THEIA_CONTEXT;
    }

    g_pTheiaCtx->pMmFreeIndependentPages = _SearchPatternInImg(NULL, SPII_NO_OPTIONAL, PsInitialSystemProcess, ".text", NULL, g_pTheiaCtx->TheiaMetaDataBlock.MMFREEINDEPENDENTPAGESEX_SIG, g_pTheiaCtx->TheiaMetaDataBlock.MMFREEINDEPENDENTPAGESEX_MASK);

    if (!g_pTheiaCtx->pMmFreeIndependentPages)
    {
        DbgLog("[TheiaPg <->] InitTheiaContext: BaseVa MmFreeIndependentPages not found\n");

        goto DIE_CALL_ERROR_INIT_THEIA_CONTEXT;
    }

    __sidt(&IDTR);

    pSwKIDTENTRY64 = (PKIDTENTRY64)((PUCHAR)(*(PVOID*)(IDTR + 2)) + (0x10UI64 * 0x20UI64));

    pKiSwInterruptDispatch = (PVOID)((ULONG64)(pSwKIDTENTRY64->OffsetLow) | (((ULONG64)(pSwKIDTENTRY64->OffsetMiddle)) << 16UI16) | (((ULONG64)(pSwKIDTENTRY64->OffsetHigh)) << 32UI32));

    SaveRel32Offset = *(PLONG32)((PUCHAR)pKiSwInterruptDispatch + 0x3a0UI64);

    g_pTheiaCtx->pKiSwInterruptDispatch = (((ULONG64)pKiSwInterruptDispatch + 0x3a4UI64) + ((SaveRel32Offset < 0I32) ? ((LONG64)SaveRel32Offset | 0xffffffff00000000UI64) : (LONG64)SaveRel32Offset));

    SaveRel32Offset = *(PLONG32)((PUCHAR)g_pTheiaCtx->pKiSwInterruptDispatch + 0x20UI64);

    g_pTheiaCtx->ppMaxDataSize = (((ULONG64)g_pTheiaCtx->pKiSwInterruptDispatch + 0x24UI64) + ((SaveRel32Offset < 0I32) ? ((LONG64)SaveRel32Offset | 0xffffffff00000000UI64) : (LONG64)SaveRel32Offset));

    //
    // Initialization A3-Block
    //
    for(ULONG32 i = 0UI32; i < 52UI32; ++i) { g_pTheiaCtx->PgXorRoutineSig[i] = PgXorRoutineSig[i]; } 

    //
    // Initialization A4-Block
    //
    g_pTheiaCtx->pKeIpiGenericCall               = MmGetSystemRoutineAddress(&StrKeIpiGenericCall);

    g_pTheiaCtx->pMmGetPhysicalAddress           = MmGetSystemRoutineAddress(&StrMmGetPhysicalAddress);

    g_pTheiaCtx->pMmMapIoSpaceEx                 = MmGetSystemRoutineAddress(&StrMmMapIoSpaceEx);

    g_pTheiaCtx->pMmUnmapIoSpace                 = MmGetSystemRoutineAddress(&StrMmUnmapIoSpace);

    g_pTheiaCtx->pRtlLookupFunctionEntry         = MmGetSystemRoutineAddress(&StrRtlLookupFunctionEntry);

    g_pTheiaCtx->pPsLookupThreadByThreadId       = MmGetSystemRoutineAddress(&StrPsLookupThreadByThreadId);

    g_pTheiaCtx->pRtlVirtualUnwind               = MmGetSystemRoutineAddress(&StrRtlVirtualUnwind);

    g_pTheiaCtx->pKeInitializeApc                = MmGetSystemRoutineAddress(&StrKeInitializeApc);

    g_pTheiaCtx->pKeInsertQueueApc               = MmGetSystemRoutineAddress(&StrKeInsertQueueApc);

    g_pTheiaCtx->pKeDelayExecutionThread         = MmGetSystemRoutineAddress(&StrKeDelayExecutionThread);

    g_pTheiaCtx->pMmIsAddressValid               = MmGetSystemRoutineAddress(&StrMmIsAddressValid);

    g_pTheiaCtx->pMmIsNonPagedSystemAddressValid = MmGetSystemRoutineAddress(&StrMmIsNonPagedSystemAddressValid);

    g_pTheiaCtx->pExAllocatePool2                = pExAllocatePool2;

    g_pTheiaCtx->pPsIsSystemThread               = MmGetSystemRoutineAddress(&StrPsIsSystemThread);

    g_pTheiaCtx->pObfDereferenceObject           = MmGetSystemRoutineAddress(&StrObfDereferenceObject);

    //
    // Initialization A5-Block
    //
    g_pTheiaCtx->pKeInitializeCrashDumpHeader = MmGetSystemRoutineAddress(&StrKeInitializeCrashDumpHeader);

    g_pTheiaCtx->pIoCancelIrp                 = MmGetSystemRoutineAddress(&StrIoCancelIrp);

    g_pTheiaCtx->pKeBugCheckEx                = MmGetSystemRoutineAddress(&StrKeBugCheckEx);

    // Initialization A1/A6/A7-Blocks ==========================================================================================================================================================================================================++
    //                                                                                                                                                                                                                                          //
                                                                                                                                                                                                                                                //
    SaveRel32Offset = *(PLONG32)((PUCHAR)g_pTheiaCtx->pKeInitializeCrashDumpHeader + 0x68UI64);                                                                                                                                                 //
                                                                                                                                                                                                                                                //
    pKdDebuggerDataBlock = (((ULONG64)g_pTheiaCtx->pKeInitializeCrashDumpHeader + 0x6cUI64) + ((SaveRel32Offset < 0I32) ? ((ULONG64)SaveRel32Offset | 0xffffffff00000000UI64) : (ULONG64)SaveRel32Offset)); ///< Getting addr IoFillDumpHeader. //
                                                                                                                                                                                                                                                //
    pKdDebuggerDataBlock = ((PKDDEBUGGER_DATA64)(((ULONG64)pKdDebuggerDataBlock + 0xf9UI64) + (ULONG64)(*(PULONG32)((PUCHAR)pKdDebuggerDataBlock + 0xf5UI64)))); ///< Getting addr KdDebuggerDataBlock.                                         //
                                                                                                                                                                                                                                                //                                                                                                                                                                                                                                               
    //                                                                                                                                                                                                                                          //
    // Initialization A1-Block                                                                                                                                                                                                                  //
    //                                                                                                                                                                                                                                          //
    g_pTheiaCtx->pKernelBase = pKdDebuggerDataBlock->KernBase;                                                                                                                                                                                  //
                                                                                                                                                                                                                                                //
    //                                                                                                                                                                                                                                          //
    // Initialization A6-Block                                                                                                                                                                                                                  //
    //                                                                                                                                                                                                                                          //
    g_pTheiaCtx->pMmPfnDatabase = pKdDebuggerDataBlock->MmPfnDatabase;                                                                                                                                                                          //
                                                                                                                                                                                                                                                //
                                                                                                                                                                                                                                                //
    // Initialization A7-Block ==================================================================================++                                                                                                                             //
                                                                                                                 //                                                                                                                             //
    g_pTheiaCtx->pMmPteBase = pKdDebuggerDataBlock->PteBase;                                                     //                                                                                                                             //
                                                                                                                 //                                                                                                                             //
    g_pTheiaCtx->pMmPdeBase = g_pTheiaCtx->pMmPteBase + ((g_pTheiaCtx->pMmPteBase >> 9UI64) & 0x7FFFFFFFFFUI64); //                                                                                                                             //
                                                                                                                 //                                                                                                                             //
    g_pTheiaCtx->pMmPpeBase = g_pTheiaCtx->pMmPdeBase + ((g_pTheiaCtx->pMmPdeBase >> 9UI64) & 0x3FFFFFF8UI64);   //                                                                                                                             //
                                                                                                                 //                                                                                                                             //
    g_pTheiaCtx->pMmPxeBase = g_pTheiaCtx->pMmPpeBase + ((g_pTheiaCtx->pMmPpeBase >> 9UI64) & 0x1FFFFFUI64);     //                                                                                                                             //
                                                                                                                 //                                                                                                                             //
    g_pTheiaCtx->pMmPxeSelf = ((g_pTheiaCtx->pMmPxeBase >> 9UI64) & 0xFFFUI64);                                  //                                                                                                                             //
                                                                                                                 //                                                                                                                             //
    // ==========================================================================================================++                                                                                                                             //
                                                                                                                                                                                                                                                //
    //                                                                                                                                                                                                                                          //
    // =========================================================================================================================================================================================================================================++
    
    g_pTheiaCtx->CompleteSignatureTC = COMPLETE_SIGNATURE_TC;

    g_CompleteInitTheiaCtx = TRUE;

    if (g_VolatileNullByte) { DIE_CALL_ERROR_INIT_THEIA_CONTEXT: DieDispatchIntrnlError(ERROR_INIT_THEIA_CONTEXT); }

    if (g_VolatileNullByte) { DIE_CALL_ERROR_DOUBLE_INIT_THEIA_CONTEXT: DieDispatchIntrnlError(ERROR_DOUBLE_INIT_THEIA_CONTEXT); }

    return;
}

/*++
* Routine: CheckStatusTheiaCtx
*
* MaxIRQL: Any level
*
* Public/Private: Public
*
* @param NoParams
*
* Description: Checking current state gTheiaPg.
--*/
VOID CheckStatusTheiaCtx(VOID)
{
    #define ERROR_THEIA_CONTEXT_NOT_INIT 0xbb722de3UI32

    if (!g_pTheiaCtx)
    {
        DbgLog("[TheiaPg <->] CheckStatusTheiaCtx: gTheiaContext is not allocate\n");

        goto DIE_CALL_ERROR_THEIA_CONTEXT_NOT_INIT;
    }
    else if (g_pTheiaCtx->CompleteSignatureTC != COMPLETE_SIGNATURE_TC)
    {
        DbgLog("[TheiaPg <->] CheckStatusTheiaCtx: gTheiaContext is not complete\n");

        goto DIE_CALL_ERROR_THEIA_CONTEXT_NOT_INIT;
    }
    else if (g_pTheiaCtx->TheiaMetaDataBlock.CompleteSignatureTMDB != COMPLETE_SIGNATURE_TMDB)
    {
        DbgLog("[TheiaPg <->] CheckStatusTheiaCtx: gTheiaMetaDataBlock is not complete\n");

        goto DIE_CALL_ERROR_THEIA_CONTEXT_NOT_INIT;
    }
    else { VOID; } ///< For clarity.

    
    if (g_VolatileNullByte) { DIE_CALL_ERROR_THEIA_CONTEXT_NOT_INIT: DieDispatchIntrnlError(ERROR_THEIA_CONTEXT_NOT_INIT); }
    
    return;
}
