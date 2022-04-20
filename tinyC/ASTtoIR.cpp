#include "ASTtoIR.h"
#include "common/types.h"

namespace tinyc
{
    using Type = tiny::Type;
    ASTtoIR::ASTtoIR()
    {
        m_env = nullptr;
        m_block = std::make_shared<Block>();
        Block::counter = 0;
    }

    void ASTtoIR::visit(ASTInteger * ast)
    {
        std::shared_ptr<Load_Imm_i> ptr = std::make_shared<Load_Imm_i>();
        ptr->m_value = ast->value;
        ptr->m_type = ResultType::Integer;
        m_block->m_block.push_back(ptr);
        m_last = ptr;
    }
    
    void ASTtoIR::visit(ASTDouble * ast)
    {
        std::shared_ptr<Load_Imm_d> ptr = std::make_shared<Load_Imm_d>();
        ptr->m_value = ast->value;
        ptr->m_type = ResultType::Double;
        m_block->m_block.push_back(ptr);
        m_last = ptr;
    }
    
    void ASTtoIR::visit(ASTChar * ast)
    {
        std::shared_ptr<Load_Imm_c> ptr = std::make_shared<Load_Imm_c>();
        ptr->m_value = ast->value;
        ptr->m_type = ResultType::Char;
        m_block->m_block.push_back(ptr);
        m_last = ptr;
    }
    
    void ASTtoIR::visit(ASTString * ast)
    {
        //TODO
    }

    ResultType ASTtoIR::GetType(Type * t)
    {
        if (t == Type::charType())
            return ResultType::Char;
        else if (t == Type::doubleType())
            return ResultType::Double;
        else
            return ResultType::Integer;
    }
    
    void ASTtoIR::NewBlock()
    {
        m_fun->m_blocks.push_back(m_block);
        m_block = std::make_shared<Block>();
    }

    
    void ASTtoIR::visit(ASTIdentifier * ast)
    {
        m_last = m_env->FindVar(ast->name);
        if (!m_leftValue)
        {
            std::shared_ptr<Load> ptr = std::make_shared<Load>();
            ptr->m_address = m_last;
            ptr->m_type = GetType(ast->type());
            m_block->m_block.push_back(ptr);
            m_last = ptr;
        }
    }
    
    void ASTtoIR::visit(ASTArrayValue * ast)
    {
        //TODO
    }
    void ASTtoIR::visit(ASTPointerType * ast)
    {
        //EMPTY
    }
    void ASTtoIR::visit(ASTArrayType * ast)
    {
        //EMPTY
    }
    void ASTtoIR::visit(ASTNamedType * ast)
    {
        //EMPTY
    }


    void ASTtoIR::visit(ASTSequence * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTBlock * ast)
    {
        EnterEnv();
        for (auto & x : ast->body)
            visitChild(x);
        LeaveEnv();
    }
    
    void ASTtoIR::visit(ASTVarDecl * ast)
    {
        std::shared_ptr<Instruction> allocation;
        if (m_fun == nullptr){
            allocation = std::make_shared<Alloc_g>();
            m_allocs_g.push_back(allocation);
        }
        else{
            allocation = std::make_shared<Alloc_l>();
            m_fun->m_allocs.push_back(allocation);
        }
        m_last = allocation;

        allocation->m_type = GetType(ast->type());

        m_env->AddVar(ast->name->name,allocation);

        if (ast->value == nullptr)
            m_last = allocation;
        else
        {
            visitChild(ast->value);
            std::shared_ptr<Store> store = std::make_shared<Store>();
            store->m_type = ResultType::Void;
            store->m_address = allocation;
            store->m_value = m_last;
            m_block->m_block.push_back(store);
        }
    }
    
    void ASTtoIR::visit(ASTFunDecl * ast)
    {
        std::shared_ptr<Function> fun = std::make_shared<Function>(ast->name);
        std::shared_ptr<Block> prev = m_block;
        m_block = std::make_shared<Block>();
        
        std::shared_ptr<Fun_address> fun_addr = std::make_shared<Fun_address>(ast->name);

        m_env->AddVar(ast->name,fun_addr);

        EnterEnv();

        for (int i = 0; i < ast->args.size(); i++)
        {
            std::shared_ptr<Alloc_arg> alloc = std::make_shared<Alloc_arg>();
            alloc->m_type = GetType(ast->args[i].first.get()->type());
            fun->m_args.push_back(alloc);
            m_env->AddVar(ast->args[i].second->name,alloc);
        }
        m_fun = fun;

        visitChild(ast->body);

        fun->m_blocks.push_back(m_block);
        m_block = prev;

        m_funs.insert(std::make_pair(m_fun->m_name,fun));
        m_fun = nullptr;

        LeaveEnv();
    }
    
    void ASTtoIR::visit(ASTStructDecl * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTFunPtrDecl * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTIf * ast)
    {
        visitChild(ast->cond);
        std::shared_ptr<Jump_cond> cond_jump = std::make_shared<Jump_cond>();
        cond_jump->m_cond = m_last;
        m_block->m_block.push_back(cond_jump);

        NewBlock();

        cond_jump->m_true = m_block;
        visitChild(ast->trueCase);
        std::shared_ptr<Jump> jump_true = std::make_shared<Jump>();
        m_block->m_block.push_back(jump_true);

        NewBlock();

        cond_jump->m_false = m_block;
        visitChild(ast->falseCase);
        std::shared_ptr<Jump> jump_false = std::make_shared<Jump>();
        m_block->m_block.push_back(jump_false);

        NewBlock();

        jump_true->m_label = m_block;
        jump_false->m_label = m_block;
    }
    
    void ASTtoIR::visit(ASTSwitch * ast)
    {
        visitChild(ast->cond);
        std::shared_ptr<Cond_switch> switch_stmt = std::make_shared<Cond_switch>();
        std::shared_ptr<Jump> jump = std::make_shared<Jump>();

        NewBlock();

        for (auto & x : ast->cases)
        {
            switch_stmt->m_cases.insert(std::make_pair(x.first,m_block));
            visitChild(x.second);
            m_block->m_block.push_back(jump);
            NewBlock();
        }
        if (ast->defaultCase != nullptr)
        {
            switch_stmt->m_default = m_block;
            visitChild(ast->defaultCase);
            m_block->m_block.push_back(jump);
            NewBlock();
        }
        jump->m_label = m_block;
    }
    
    void ASTtoIR::AddJumpsContinueBreak()
    {
        std::shared_ptr<Jump> jumpcb = std::make_shared<Jump>();
        m_continue.push_back(jumpcb);
        jumpcb = std::make_shared<Jump>();
        m_break.push_back(jumpcb);
    }
    
    void ASTtoIR::RemJumpsContinueBreak(std::shared_ptr<Block> & c, std::shared_ptr<Block> & b)
    {
        m_continue.back()->m_label = c;
        m_continue.pop_back();
        m_break.back()->m_label = b;
        m_break.pop_back();
    }

    void ASTtoIR::visit(ASTWhile * ast)
    {
        AddJumpsContinueBreak();

        std::shared_ptr<Jump> jump = std::make_shared<Jump>();
        m_block->m_block.push_back(jump);

        NewBlock();

        jump->m_label = m_block;
        visitChild(ast->cond);
        std::shared_ptr<Jump_cond> jump_cond = std::make_shared<Jump_cond>();
        jump_cond->m_cond = m_last;
        m_block->m_block.push_back(jump_cond);
        
        NewBlock();

        jump_cond->m_true = m_block;
        visitChild(ast->body);
        m_block->m_block.push_back(jump);

        NewBlock();
        
        jump_cond->m_false = m_block;

        RemJumpsContinueBreak(jump->m_label,m_block);
    }
    
    void ASTtoIR::visit(ASTDoWhile * ast)
    {
        AddJumpsContinueBreak();

        std::shared_ptr<Jump_cond> jump_cond = std::make_shared<Jump_cond>();

        NewBlock();

        jump_cond->m_true = m_block;
        visitChild(ast->body);
        std::shared_ptr<Jump> jump = std::make_shared<Jump>();
        m_block->m_block.push_back(jump);

        NewBlock();

        jump->m_label = m_block;
        visitChild(ast->cond);
        jump_cond->m_cond = m_last;
        m_block->m_block.push_back(jump_cond);

        NewBlock();

        jump_cond->m_false = m_block;

        RemJumpsContinueBreak(jump->m_label,m_block);
    }
    
    void ASTtoIR::visit(ASTFor * ast)
    {
        AddJumpsContinueBreak();

        std::shared_ptr<Jump_cond> cond_jump = std::make_shared<Jump_cond>();
        visitChild(ast->init);
        std::shared_ptr<Jump> jump1 = std::make_shared<Jump>();
        m_block->m_block.push_back(jump1);

        NewBlock();

        jump1->m_label = m_block;
        visitChild(ast->cond);
        cond_jump->m_cond = m_last;
        m_block->m_block.push_back(cond_jump);

        NewBlock();

        cond_jump->m_true = m_block;
        visitChild(ast->body);
        std::shared_ptr<Jump> jump2 = std::make_shared<Jump>();
        m_block->m_block.push_back(jump2);

        NewBlock();

        jump2->m_label = m_block;
        visitChild(ast->increment);
        m_block->m_block.push_back(jump1);

        NewBlock();

        cond_jump->m_false = m_block;

        RemJumpsContinueBreak(jump2->m_label,m_block);
    }
    
    void ASTtoIR::visit(ASTBreak * ast)
    {
        m_block->m_block.push_back(m_break.back());
    }
    
    void ASTtoIR::visit(ASTContinue * ast)
    {
        m_block->m_block.push_back(m_continue.back());
    }
    
    void ASTtoIR::visit(ASTReturn * ast)
    {
        std::shared_ptr<Return> ret = std::make_shared<Return>();
        if (ast->value != nullptr)
        {
            visitChild(ast->value);
            ret->m_res = m_last;
        }
        else
        {
            ret->m_res = nullptr;
        }
        m_block->m_block.push_back(ret);
    }
    
    std::shared_ptr<Instruction> ASTtoIR::AssignCast(ResultType left, ResultType right, std::shared_ptr<Instruction> & ins)
    {
        if (left != right)
        {
            if (left == ResultType::Integer)
            {
                if (right == ResultType::Char)
                {
                    std::shared_ptr<Castctoi> cast = std::make_shared<Castctoi>(ins);
                    return cast;
                }
                else if (right == ResultType::Double)
                {
                    std::shared_ptr<Castdtoi> cast = std::make_shared<Castdtoi>(ins);
                    return cast;
                }
            }
            else if (left == ResultType::Double)
            {
                if (right == ResultType::Integer)
                {
                    std::shared_ptr<Castitod> cast = std::make_shared<Castitod>(ins);
                    return cast;
                }
                else if (right == ResultType::Char)
                {
                    std::shared_ptr<Castctod> cast = std::make_shared<Castctod>(ins);
                    return cast;
                }
            }
        }
        return ins;
    }

    void ASTtoIR::visit(ASTAssignment * ast)
    {
        std::shared_ptr<Store> store = std::make_shared<Store>();
        
        visitChild(ast->value);
        store->m_value = m_last;

        m_leftValue = true;
        visitChild(ast->lvalue);
        m_leftValue = false;

        store->m_address = m_last;

        AssignCast(store->m_address->m_type,store->m_value->m_type,store->m_value);

        m_block->m_block.push_back(store);
    }

    void ASTtoIR::visit(ASTBinaryOp * ast)
    {
        visitChild(ast->left);
        std::shared_ptr<Instruction> left = m_last;

        visitChild(ast->right);
        std::shared_ptr<Instruction> right = m_last;

        ResultType t = GetType(ast->type());

        if (ast->op == Symbol::Add || ast->op == Symbol::Sub ||
            ast->op == Symbol::Mul || ast->op == Symbol::Div ||
            ast->op == Symbol::Mod || ast->op == Symbol::BitAnd ||
            ast->op == Symbol::BitOr)
        {
            left = AssignCast(t,m_last->m_type,left);
            right = AssignCast(t,m_last->m_type,right);
        }
        else if (ast->op == Symbol::Eq || ast->op == Symbol::NEq ||
                ast->op == Symbol::Lt || ast->op == Symbol::Lte ||
                ast->op == Symbol::Gt || ast->op == Symbol::Gte)
        {
            ///TODO
            t = GetType(ast->right->type());
        }
        
        if (ast->op == Symbol::Add)
            m_last = std::make_shared<Add>(left,right,t);
        else if (ast->op == Symbol::Sub)
            m_last = std::make_shared<Sub>(left,right,t);
        else if (ast->op == Symbol::Mul)
            m_last = std::make_shared<Mul>(left,right,t);
        else if (ast->op == Symbol::Div)
            m_last = std::make_shared<Div>(left,right,t);
        else if (ast->op == Symbol::Mod)
            m_last = std::make_shared<Mod>(left,right,t);
        else if (ast->op == Symbol::BitAnd)
            m_last = std::make_shared<BitAnd>(left,right,t);
        else if (ast->op == Symbol::BitOr)
            m_last = std::make_shared<BitOr>(left,right,t);
        else if (ast->op == Symbol::Eq)
            m_last = std::make_shared<Eq>(left,right,t);
        else if (ast->op == Symbol::NEq)
            m_last = std::make_shared<NEq>(left,right,t);
        else if (ast->op == Symbol::Lt)
            m_last = std::make_shared<Lt>(left,right,t);
        else if (ast->op == Symbol::Lte)
            m_last = std::make_shared<Lte>(left,right,t);
        else if (ast->op == Symbol::Gt)
            m_last = std::make_shared<Gt>(left,right,t);
        else if (ast->op == Symbol::Gte)
            m_last = std::make_shared<Gte>(left,right,t);
        else if (ast->op == Symbol::ShiftLeft)
            m_last = std::make_shared<ShL>(left,right,t);
        else if (ast->op == Symbol::ShiftRight)
            m_last = std::make_shared<ShR>(left,right,t);
        else if (ast->op == Symbol::And)
            m_last = std::make_shared<And>(left,right,t);
        else if (ast->op == Symbol::Or)
            m_last = std::make_shared<Or>(left,right,t);

        m_block->m_block.push_back(m_last);

        //TODO cast
    }
    
    void ASTtoIR::visit(ASTUnaryOp * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTUnaryPostOp * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTAddress * ast)
    {
        m_leftValue = true;
        visitChild(ast->target);
        m_leftValue = false;
    }
    
    void ASTtoIR::visit(ASTDeref * ast)
    {
        if (m_leftValue){
            m_leftValue = false;
            visitChild(ast->target);
            m_leftValue = true;
        }
        else{
            visitChild(ast->target);
            std::shared_ptr<Load> ptr = std::make_shared<Load>();
            ptr->m_address = m_last;
            ptr->m_type = GetType(ast->type());
            m_block->m_block.push_back(ptr);
            m_last = ptr;
        }
    }
    
    void ASTtoIR::visit(ASTIndex * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTMember * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTMemberPtr * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTCall * ast)
    {
        std::shared_ptr<CallStatic> call = std::make_shared<CallStatic>();
        call->m_type = GetType(ast->type());

        for (auto & x : ast->args)
        {
            visitChild(x);
            call->m_args.push_back(m_last);
        }
        visitChild(ast->function);
        call->m_fun_addr = m_last;
        m_block->m_block.push_back(call);
        m_last = call;
    }
    
    void ASTtoIR::visit(ASTCast * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTRead * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTWrite * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visitChild(AST * ast)
    {
        ASTVisitor::visitChild(ast);
    }
    
    template<typename T>
    void ASTtoIR::visitChild(std::unique_ptr<T> const & ptr)
    {
        visitChild(ptr.get());
    } 


    void ASTtoIR::EnterEnv()
    {
        std::shared_ptr<Env> m_now = std::make_shared<Env>();
        m_now->m_prev = m_env;
        m_env = m_now;
    }
    
    void ASTtoIR::LeaveEnv()
    {
        m_env = m_env->m_prev;
    }
    
    void ASTtoIR::Env::AddVar(Symbol name, std::shared_ptr<Instruction> ins)
    {
        m_map.insert(std::make_pair(name,ins));
    }


    std::shared_ptr<Instruction> ASTtoIR::Env::FindVar(Symbol name)
    {
        auto it = m_map.find(name);
        if (it != m_map.end())
            return it->second;
        else if (m_prev != nullptr)
            return m_prev->FindVar(name);
        else
            return nullptr;
    }
    

}