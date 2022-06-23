#include "Optimalization.h"
#include "IR.h"
#include "Peephole.h"
#include "Inlining.h"
#include <vector>

namespace tinyc
{
    Optimalization::Optimalization(IRProgram * prg)
    :m_prg(prg)
    {
    }
    
    Optimalization::~Optimalization()
    {
    }
    
    void Optimalization::StartAll()
    {
        PeepholeOptimalization();
        InliningOptimalization();
    }
    
    void Optimalization::PeepholeOptimalization()
    {
        Peephole peep;
        SimpleBinExpr(peep);
        StrengthReduction1(peep);
        StrengthReduction2(peep);
        peep.visit(m_prg);
    }
    
    void Optimalization::InliningOptimalization()
    {
        Inlining in(m_prg->m_funs);
        in.Start();
    }
    
    void Optimalization::SimpleBinExpr(Peephole & peep)
    {   
        std::vector<Instruction*> * pat = new std::vector<Instruction*>();
        Load_Imm_i * val1 = new Load_Imm_i();
        Load_Imm_i * val2 = new Load_Imm_i();
        BinaryOp * op1 = new BinaryOp(nullptr,nullptr,ResultType::Void);
        pat->push_back(val1);
        pat->push_back(val2);
        pat->push_back(op1);
        peep.InsertPattern(pat,FunSimplifyBinExpr);

    }
    
    void Optimalization::StrengthReduction2(Peephole & peep)
    {
        std::vector<Instruction*> * pat = new std::vector<Instruction*>();
        Load * val1 = new Load();
        Load_Imm_i * val2 = new Load_Imm_i();
        Div * op1 = new Div(nullptr,nullptr,ResultType::Integer);
        pat->push_back(val1);
        pat->push_back(val2);
        pat->push_back(op1);
        peep.InsertPattern(pat,FunStrengthReduction2);
    }
    
    void Optimalization::StrengthReduction1(Peephole & peep)
    {
        std::vector<Instruction*> * pat = new std::vector<Instruction*>();
        Load * val1 = new Load();
        Load_Imm_i * val2 = new Load_Imm_i();
        Mul * op1 = new Mul(nullptr,nullptr,ResultType::Integer);
        pat->push_back(val1);
        pat->push_back(val2);
        pat->push_back(op1);
        peep.InsertPattern(pat,FunStrengthReduction1);
    }

}