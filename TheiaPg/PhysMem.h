#pragma once

#include "LinkHeader.h"

#define MEM_INDPN_RW_READ_OP_BIT   1UI64 // 0-bit

#define MEM_INDPN_RW_WRITE_OP_BIT  2UI64 // 2-bit

extern VOID HrdIndpnRWVMemory(IN ULONG64 FlagsExecute, IN OUT PVOID pVa, IN OUT PVOID pInputBuffer, IN ULONG32 LengthRW);

extern VOID HrdPatchAttributesInputPte(IN ULONG64 AndMask, IN ULONG64 OrMask, IN OUT PVOID pVa);

extern PMMPTE_HARDWARE HrdGetPteInputVa(IN PVOID pVa);
