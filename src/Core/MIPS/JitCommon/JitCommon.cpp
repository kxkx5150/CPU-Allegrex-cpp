




#include <stdlib.h>

#include "Core/MIPS/JitCommon/JitCommon.h"
#include "Core/MIPS/JitCommon/NativeJit.h"
#include "Common/StringUtils.h"


namespace MIPSComp {
#if defined(ARM)
ArmJit *jit;
#else
Jit *jit;
#endif
void JitAt()
{
    jit->Compile(currentMIPS->pc);
}
}    // namespace MIPSComp


#ifndef ARM

const char *ppsspp_resolver(struct ud *, uint64_t addr, int64_t *offset)
{
    if (addr >= (uint64_t)(&currentMIPS->r[0]) && addr < (uint64_t)&currentMIPS->r[32]) {
        *offset = addr - (uint64_t)(&currentMIPS->r[0]);
        return "mips.r";
    }
    if (addr >= (uint64_t)(&currentMIPS->v[0]) && addr < (uint64_t)&currentMIPS->v[128]) {
        *offset = addr - (uint64_t)(&currentMIPS->v[0]);
        return "mips.v";
    }
    if (MIPSComp::jit->IsInSpace((u8 *)(intptr_t)addr)) {
        *offset = addr - (uint64_t)MIPSComp::jit->GetBasePtr();
        return "jitcode";
    }
    if (MIPSComp::jit->Asm().IsInSpace((u8 *)(intptr_t)addr)) {
        *offset = addr - (uint64_t)MIPSComp::jit->Asm().GetBasePtr();
        return "dispatcher";
    }

    return NULL;
}



#endif