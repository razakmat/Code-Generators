#include "IR.h"

namespace tinyc
{
    Instruction::Instruction()
    :m_type(ResultType::Void)
    {
        
    }

    Instruction::Instruction(ResultType t)
    :m_type(t)
    {
        
    }

    IRProgram::~IRProgram()
    {
        for (auto & x : m_decls)
            delete x;
        for (auto & x : m_allocs_g)
            delete x;
        for (auto & x : m_funs)
        {
            delete x->m_addr;
            for (auto & y : x->m_allocs)
                delete y;
            for (auto & y : x->m_args)
                delete y;
            for (auto & y : x->m_blocks)
            {
                for (auto & z : y->m_block){
                    if (!dynamic_cast<Jump*>(z))
                        delete z;
                }
            }
        }
    }

    Block::Block()
    :m_name(counter++)
    {
        
    }

    Function::Function(const std::string & name)
    :m_name(name)
    {
        
    }
    
    Fun_address::Fun_address(Function * fun)
    :m_fun(fun)
    {
        
    }
    
    BinaryOp::BinaryOp(Instruction * left, Instruction * right, ResultType t)
    :Instruction(t), m_left(left), m_right(right)
    {
    }
    
    UnaryOp::UnaryOp(Instruction * ins,ResultType t)
    :Instruction(t), m_left(ins)
    {
        
    }
    
    Mul::Mul(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Div::Div(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Mod::Mod(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Add::Add(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Sub::Sub(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    ShL::ShL(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    ShR::ShR(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    CmpOp::CmpOp(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        m_cpm_jump = false;
    }
    
    void CmpOp::SetCmpJump(bool b)
    {
        m_cpm_jump = b;
    }
    
    bool CmpOp::GetCmpJump()
    {
        return m_cpm_jump;
    }
    
    Gt::Gt(Instruction * left, Instruction * right, ResultType t)
    :CmpOp(left,right,t)
    {
        
    }
    
    Gte::Gte(Instruction * left, Instruction * right, ResultType t)
    :CmpOp(left,right,t)
    {
        
    }
    
    Lt::Lt(Instruction * left, Instruction * right, ResultType t)
    :CmpOp(left,right,t)
    {
        
    }
    
    Lte::Lte(Instruction * left, Instruction * right, ResultType t)
    :CmpOp(left,right,t)
    {
        
    }
    
    Eq::Eq(Instruction * left, Instruction * right, ResultType t)
    :CmpOp(left,right,t)
    {
        
    }
    
    NEq::NEq(Instruction * left, Instruction * right, ResultType t)
    :CmpOp(left,right,t)
    {
        
    }
    
    BitAnd::BitAnd(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    BitOr::BitOr(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    And::And(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Or::Or(Instruction * left, Instruction * right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Plus::Plus(Instruction * ins, ResultType t)
    :UnaryOp(ins,t)
    {
        
    }
    
    Minus::Minus(Instruction * ins, ResultType t)
    :UnaryOp(ins,t)
    {
        
    }
    
    Not::Not(Instruction * ins, ResultType t)
    :UnaryOp(ins,t)
    {
        
    }
    
    Neg::Neg(Instruction * ins, ResultType t)
    :UnaryOp(ins,t)
    {
        
    }
    
    Inc::Inc(Instruction * ins, ResultType t)
    :UnaryOp(ins,t)
    {
        
    }
    
    Dec::Dec(Instruction * ins, ResultType t)
    :UnaryOp(ins,t)
    {
        
    }
    
    Castctoi::Castctoi(Instruction * val)
    :Instruction(ResultType::Integer), m_val(val)
    {
    }
    
    Castctod::Castctod(Instruction * val)
    :Instruction(ResultType::Double), m_val(val)
    {
    }
    
    Castitod::Castitod(Instruction * val)
    :Instruction(ResultType::Double), m_val(val)
    {
    }
    
    Castdtoi::Castdtoi(Instruction * val)
    :Instruction(ResultType::Integer), m_val(val)
    {
    }
    
    DebugWrite::DebugWrite(Instruction * val)
    :m_val(val)
    {
    }

 
}