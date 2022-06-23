#pragma once

#include <string>
#include "IR.h"
#include "Peephole.h"

namespace tinyc {



    class Optimalization {
    public:

        Optimalization(IRProgram * prg);
        ~Optimalization();
        void StartAll();
        void PeepholeOptimalization();
        void InliningOptimalization();

    protected:
        void SimpleBinExpr(Peephole & peep);
        void StrengthReduction1(Peephole & peep);
        void StrengthReduction2(Peephole & peep);
        IRProgram * m_prg;
    };
}

