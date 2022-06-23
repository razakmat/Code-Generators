#include "backend.h"

#include "IR.h"
#include "IRTot86.h"
#include "../tiny86/target.h"


namespace tinyc
{
    using namespace tiny;

    Backend::Backend(IRProgram * prg,int regs, int fregs)
    :m_prg(prg)
    {
        target = new IRTot86(regs);
    }
    
    Backend::~Backend()
    {
        delete target;
    }
    
    void Backend::irtot86()
    {
        target->visit(m_prg);
    }
    
    void Backend::Start()
    {
        Target ex;
        ex.execute(target->GetProgram());
    }



    
}

