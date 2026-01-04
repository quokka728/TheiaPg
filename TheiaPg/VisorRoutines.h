#pragma once

#include "LinkHeader.h"

extern volatile VOID VsrKiExecuteAllDpcs(PINPUTCONTEXT_ICT pInputCtx);

extern volatile VOID VsrExAllocatePool2(IN OUT PINPUTCONTEXT_ICT pInputCtx);

extern volatile VOID VsrKiCustomRecurseRoutineX(IN OUT PINPUTCONTEXT_ICT pInputCtx);
