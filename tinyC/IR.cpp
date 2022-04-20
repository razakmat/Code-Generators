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

    Block::Block()
    :m_name(counter++)
    {
        
    }

    Function::Function(Symbol name)
    :m_name(name)
    {
        
    }
    
    Fun_address::Fun_address(Symbol name)
    :m_name(name)
    {
        
    }
    
    BinaryOp::BinaryOp(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :Instruction(t), m_left(left), m_right(right)
    {
    }
    
    Mul::Mul(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Div::Div(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Mod::Mod(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Add::Add(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Sub::Sub(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    ShL::ShL(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    ShR::ShR(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Gt::Gt(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Gte::Gte(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Lt::Lt(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Lte::Lte(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Eq::Eq(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    NEq::NEq(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    BitAnd::BitAnd(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    BitOr::BitOr(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    And::And(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Or::Or(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t)
    :BinaryOp(left,right,t)
    {
        
    }
    
    Castctoi::Castctoi(std::shared_ptr<Instruction> & val)
    :Instruction(ResultType::Integer), m_val(val)
    {
    }
    
    Castctod::Castctod(std::shared_ptr<Instruction> & val)
    :Instruction(ResultType::Double), m_val(val)
    {
    }
    
    Castitod::Castitod(std::shared_ptr<Instruction> & val)
    :Instruction(ResultType::Double), m_val(val)
    {
    }
    
    Castdtoi::Castdtoi(std::shared_ptr<Instruction> & val)
    :Instruction(ResultType::Integer), m_val(val)
    {
    }

 
}