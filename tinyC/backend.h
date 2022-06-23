#pragma once

#include <string>

#include "IR.h"
#include "IRTot86.h"

namespace tinyc {


    class Backend {
    public:

        Backend(IRProgram * prg,int regs = 10, int fregs = 5);
        ~Backend();
        
        void irtot86();
        void Start();

    protected:
        IRProgram * m_prg;
        IRTot86 * target;
    }; // tinyc::BackEnd
}

