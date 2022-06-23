#include "ASTtoIR.h"
#include "common/types.h"

namespace tinyc
{
    using Type = tiny::Type;
    ASTtoIR::ASTtoIR()
    {
        m_env = nullptr;
        Block::counter = 0;
        m_block = new Block();
        m_prg = new IRProgram();
    }
    
    ASTtoIR::~ASTtoIR()
    {
        delete m_block;
    }

    void ASTtoIR::visit(ASTInteger * ast)
    {
        Load_Imm_i * ptr = new Load_Imm_i();
        ptr->m_value = ast->value;
        ptr->m_type = ResultType::Integer;
        m_block->m_block.push_back(ptr);
        m_last = ptr;
    }
    
    void ASTtoIR::visit(ASTDouble * ast)
    {
        Load_Imm_d * ptr = new Load_Imm_d();
        ptr->m_value = ast->value;
        ptr->m_type = ResultType::Double;
        m_block->m_block.push_back(ptr);
        m_last = ptr;
    }
    
    void ASTtoIR::visit(ASTChar * ast)
    {
        Load_Imm_c * ptr = new Load_Imm_c();
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
        else if (t == Type::voidType())
            return ResultType::Void;
        else
            return ResultType::Integer;
    }
    
    void ASTtoIR::NewBlock()
    {
        if (m_block->m_block.empty())
            m_block->m_block.push_back(new NOP());
        m_fun->m_blocks.push_back(m_block);
        m_block = new Block();
    }
    
    void ASTtoIR::visit(ASTIdentifier * ast)
    {
        m_last = m_env->FindVar(ast->name);
        if (!m_leftValue)
        {
            Load * ptr = new Load();
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
        for (auto & x : ast->body)
            visitChild(x);
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
        Instruction * allocation;
        if (m_fun == nullptr){
            allocation = new Alloc_g();
            m_prg->m_allocs_g.push_back(dynamic_cast<Alloc_g*>(allocation));
        }
        else{
            allocation = new Alloc_l();
            m_fun->m_allocs.push_back(dynamic_cast<Alloc_l*>(allocation));
        }
        m_last = allocation;

        allocation->m_type = GetType(ast->type());

        m_env->AddVar(ast->name->name,allocation);

        if (ast->value == nullptr)
            m_last = allocation;
        else
        {
            visitChild(ast->value);
            Store * store = new Store();
            store->m_type = ResultType::Void;
            store->m_address = allocation;
            store->m_value = m_last;
            m_block->m_block.push_back(store);
        }
    }
    
    void ASTtoIR::visit(ASTFunDecl * ast)
    {
        for (auto & x : m_block->m_block){
            m_prg->m_decls.push_back(x);
        }
        m_block->m_block.clear();

        Function * fun = new Function(ast->name.name());
        Block * prev = m_block;
        m_block = new Block();
        
        Fun_address * fun_addr = new Fun_address(fun);
        fun->m_addr = fun_addr;

        m_env->AddVar(ast->name,fun_addr);

        EnterEnv();
        
        for (int i = 0; i < ast->args.size(); i++)
        {
            Alloc_arg * alloc = new Alloc_arg();
            alloc->m_type = GetType(ast->args[i].first.get()->type());
            fun->m_args.push_back(alloc);
            m_env->AddVar(ast->args[i].second->name,alloc);
        }
        m_fun = fun;

        visitChild(ast->body);

        if (m_block->m_block.empty())
            m_block->m_block.push_back(new NOP());
        fun->m_blocks.push_back(m_block);
        m_block = prev;

        fun->m_res_type = GetType(ast->body->type());

        m_prg->m_funs.push_back(fun);
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
    
    void ASTtoIR::ConditionAdjust()
    {
        if (CmpOp * cmpOp = dynamic_cast<CmpOp*>(m_last))
        {
            cmpOp->SetCmpJump(true);
        }
        else
        {
            if (m_last->m_type == ResultType::Integer){
                Load_Imm_i * ptr = new Load_Imm_i();
                ptr->m_value = 0;
                ptr->m_type = ResultType::Integer;
                m_block->m_block.push_back(ptr);
                NEq * cmp = new NEq(m_last,ptr,ResultType::Integer);
                cmp->SetCmpJump(true);
                m_block->m_block.push_back(cmp);
                m_last = cmp;
            }
        }
    }

    void ASTtoIR::visit(ASTIf * ast)
    {
        visitChild(ast->cond);

        ConditionAdjust();

        Jump_cond * cond_jump = new Jump_cond();
        cond_jump->m_cond = m_last;
        m_block->m_block.push_back(cond_jump);

        NewBlock();

        cond_jump->m_true = m_block;
        visitChild(ast->trueCase);
        Jump * jump_true = new Jump();
        m_block->m_block.push_back(jump_true);

        NewBlock();

        cond_jump->m_false = m_block;
        if (ast->falseCase != nullptr){
            visitChild(ast->falseCase);
            Jump * jump_false = new Jump();
            m_block->m_block.push_back(jump_false);
            NewBlock();
            jump_false->m_label = m_block;
        }

        jump_true->m_label = m_block;
    }
    
    void ASTtoIR::SwitchCond(int val)
    {
        Load_Imm_i * ptr = new Load_Imm_i();
        ptr->m_value = val;
        ptr->m_type = ResultType::Integer;
        m_block->m_block.push_back(ptr);
        Eq * cmp = new Eq(m_last,ptr,ResultType::Integer);
        cmp->SetCmpJump(true);
        m_block->m_block.push_back(cmp);
        m_last = cmp;
    }

    void ASTtoIR::visit(ASTSwitch * ast)
    {
        visitChild(ast->cond);

        Alloc_l * allocation = new Alloc_l();
        m_fun->m_allocs.push_back(allocation);
        allocation->m_type = m_last->m_type;
        Store * store = new Store();
        store->m_type = ResultType::Void;
        store->m_address = allocation;
        store->m_value = m_last;
        m_block->m_block.push_back(store);

        Jump * jump = new Jump();
        Jump * defaultJ = new Jump();

        std::vector<Jump_cond*> m_jumps;
        for (auto & x : ast->cases)
        {
            Load * load = new Load();
            load->m_address = allocation;
            load->m_type = allocation->m_type;
            m_block->m_block.push_back(load);
            m_last = load;
            SwitchCond(x.first);
            Jump_cond * j = new Jump_cond();
            j->m_cond = m_last;
            m_block->m_block.push_back(j);
            m_jumps.push_back(j);
            NewBlock();
            j->m_false = m_block;
        }
        if (ast->defaultCase != nullptr)
        {
            m_block->m_block.push_back(defaultJ);
        }
        else
            m_block->m_block.push_back(jump);

        NewBlock();

        for (int i = 0; i < ast->cases.size(); i++)
        {
            visitChild(ast->cases[i].second);
            m_block->m_block.push_back(jump);
            m_jumps[i]->m_true = m_block;
            NewBlock();
        }
        if (ast->defaultCase != nullptr)
        {
            visitChild(ast->defaultCase);
            m_block->m_block.push_back(jump);
            defaultJ->m_label = m_block;
            NewBlock();
        }
            
        jump->m_label = m_block;
    }
    
    void ASTtoIR::AddJumpsContinueBreak()
    {
        Jump * jumpcb = new Jump();
        m_continue.push_back(jumpcb);
        jumpcb = new Jump();
        m_break.push_back(jumpcb);
    }
    
    void ASTtoIR::RemJumpsContinueBreak(Block * c, Block * b)
    {
        m_continue.back()->m_label = c;
        m_continue.pop_back();
        m_break.back()->m_label = b;
        m_break.pop_back();
    }

    void ASTtoIR::visit(ASTWhile * ast)
    {
        AddJumpsContinueBreak();

        Jump * jump = new Jump();
        m_block->m_block.push_back(jump);

        NewBlock();

        jump->m_label = m_block;
        visitChild(ast->cond);

        ConditionAdjust();

        Jump_cond * jump_cond = new Jump_cond();
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

        Jump_cond * jump_cond = new Jump_cond();

        NewBlock();

        jump_cond->m_true = m_block;
        visitChild(ast->body);
        Jump * jump = new Jump();
        m_block->m_block.push_back(jump);

        NewBlock();

        jump->m_label = m_block;
        visitChild(ast->cond);

        ConditionAdjust();

        jump_cond->m_cond = m_last;
        m_block->m_block.push_back(jump_cond);

        NewBlock();

        jump_cond->m_false = m_block;

        RemJumpsContinueBreak(jump->m_label,m_block);
    }
    
    void ASTtoIR::visit(ASTFor * ast)
    {
        AddJumpsContinueBreak();

        Jump_cond * cond_jump = new Jump_cond();
        if (ast->init != nullptr)
            visitChild(ast->init);
        Jump * jump1 = new Jump();
        m_block->m_block.push_back(jump1);

        NewBlock();

        jump1->m_label = m_block;

        if (ast->cond != nullptr)
            visitChild(ast->cond);
        else{
            Load_Imm_i * load = new Load_Imm_i();
            load->m_type = ResultType::Integer;
            load->m_value = 1;
            m_block->m_block.push_back(load);
            m_last = load;
        }

        ConditionAdjust();

        cond_jump->m_cond = m_last;
        m_block->m_block.push_back(cond_jump);

        NewBlock();

        cond_jump->m_true = m_block;
        visitChild(ast->body);
        Jump * jump2 = new Jump();
        m_block->m_block.push_back(jump2);

        NewBlock();

        jump2->m_label = m_block;
        if (ast->increment != nullptr)
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
        Return * ret = new Return();
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
    
    Instruction * ASTtoIR::AssignCast(ResultType left, ResultType right, Instruction * ins)
    {
        if (left != right)
        {
            if (left == ResultType::Integer)
            {
                if (right == ResultType::Char)
                {
                    Castctoi * cast = new Castctoi(ins);
                    m_block->m_block.push_back(cast);
                    return cast;
                }
                else if (right == ResultType::Double)
                {
                    Castdtoi * cast = new Castdtoi(ins);
                    m_block->m_block.push_back(cast);
                    return cast;
                }
            }
            else if (left == ResultType::Double)
            {
                if (right == ResultType::Integer)
                {
                    Castitod * cast = new Castitod(ins);
                    m_block->m_block.push_back(cast);
                    return cast;
                }
                else if (right == ResultType::Char)
                {
                    Castctod * cast = new Castctod(ins);
                    m_block->m_block.push_back(cast);
                    return cast;
                }
            }
        }
        return ins;
    }

    void ASTtoIR::visit(ASTAssignment * ast)
    {
        Store * store = new Store();
        
        visitChild(ast->value);
        store->m_value = m_last;

        m_leftValue = true;
        visitChild(ast->lvalue);
        m_leftValue = false;

        store->m_address = m_last;
        store->m_type = GetType(ast->lvalue->type());

        store->m_value = AssignCast(store->m_type,store->m_value->m_type,store->m_value);

        m_block->m_block.push_back(store);
    }

    void ASTtoIR::visit(ASTBinaryOp * ast)
    {
        visitChild(ast->left);
        Instruction * left = m_last;

        visitChild(ast->right);
        Instruction * right = m_last;

        ResultType t = GetType(ast->type());

        if (ast->op == Symbol::Add || ast->op == Symbol::Sub ||
            ast->op == Symbol::Mul || ast->op == Symbol::Div ||
            ast->op == Symbol::Mod || ast->op == Symbol::BitAnd ||
            ast->op == Symbol::BitOr)
        {
            left = AssignCast(t,left->m_type,left);
            right = AssignCast(t,right->m_type,right);
        }
        else if (ast->op == Symbol::Eq || ast->op == Symbol::NEq ||
                ast->op == Symbol::Lt || ast->op == Symbol::Lte ||
                ast->op == Symbol::Gt || ast->op == Symbol::Gte)
        {
            t = GetType(ast->right->type());
        }
        else if (ast->op == Symbol::ShiftLeft || ast->op == Symbol::ShiftRight)
        {
            t = GetType(ast->right->type());
        }
        
        if (ast->op == Symbol::Add)
            m_last = new Add(left,right,t);
        else if (ast->op == Symbol::Sub)
            m_last = new Sub(left,right,t);
        else if (ast->op == Symbol::Mul)
            m_last = new Mul(left,right,t);
        else if (ast->op == Symbol::Div)
            m_last = new Div(left,right,t);
        else if (ast->op == Symbol::Mod)
            m_last = new Mod(left,right,t);
        else if (ast->op == Symbol::BitAnd)
            m_last = new BitAnd(left,right,t);
        else if (ast->op == Symbol::BitOr)
            m_last = new BitOr(left,right,t);
        else if (ast->op == Symbol::Eq)
            m_last = new Eq(left,right,t);
        else if (ast->op == Symbol::NEq)
            m_last = new NEq(left,right,t);
        else if (ast->op == Symbol::Lt)
            m_last = new Lt(left,right,t);
        else if (ast->op == Symbol::Lte)
            m_last = new Lte(left,right,t);
        else if (ast->op == Symbol::Gt)
            m_last = new Gt(left,right,t);
        else if (ast->op == Symbol::Gte)
            m_last = new Gte(left,right,t);
        else if (ast->op == Symbol::ShiftLeft)
            m_last = new ShL(left,right,t);
        else if (ast->op == Symbol::ShiftRight)
            m_last = new ShR(left,right,t);
        else if (ast->op == Symbol::And)
            m_last = new And(left,right,t);
        else if (ast->op == Symbol::Or)
            m_last = new Or(left,right,t);

        m_block->m_block.push_back(m_last);

    }
    
    void ASTtoIR::visit(ASTUnaryOp * ast)
    {
        visitChild(ast->arg);
        ResultType t = GetType(ast->type());

        Store * store = new Store();

        if (ast->op == Symbol::Add)
            m_last = new Plus(m_last,t);
        else if (ast->op == Symbol::Sub)
            m_last = new Minus(m_last,t);
        else if (ast->op == Symbol::Not)
            m_last = new Not(m_last,t);
        else if (ast->op == Symbol::Inc)
            m_last = new Inc(m_last,t);
        else if (ast->op == Symbol::Dec)
            m_last = new Dec(m_last,t);
        else if (ast->op == Symbol::Neg)
            m_last = new Neg(m_last,t);

        m_block->m_block.push_back(m_last);

        if (ast->op == Symbol::Dec || ast->op == Symbol::Inc)
        {
            store->m_value = m_last;

            m_leftValue = true;
            visitChild(ast->arg);
            m_leftValue = false;

            store->m_address = m_last;

            m_block->m_block.push_back(store);
        }
    }
    
    void ASTtoIR::visit(ASTUnaryPostOp * ast)
    {
        visitChild(ast->arg);
        ResultType t = GetType(ast->type());

        Store * store = new Store();


        if (ast->op == Symbol::Inc)
            m_last = new Inc(m_last,t);
        else if (ast->op == Symbol::Dec)
            m_last = new Dec(m_last,t);

        m_block->m_block.push_back(m_last);

        store->m_value = m_last;

        visitChild(ast->arg);
        Instruction * ins = m_last;

        m_leftValue = true;
        visitChild(ast->arg);
        m_leftValue = false;

        store->m_address = m_last;

        m_block->m_block.push_back(store);

        m_last = ins;
    }
    
    void ASTtoIR::visit(ASTAddress * ast)
    {
        m_leftValue = true;
        visitChild(ast->target);
        m_leftValue = false;

        if (Fun_address * addr = dynamic_cast<Fun_address*>(m_last))
        {
            LoadFun * ptr = new LoadFun();
            ptr->m_address = addr;
            m_block->m_block.push_back(ptr);
            m_last = ptr;
            return;
        }

        LoadAddress * ptr = new LoadAddress();
        ptr->m_address = m_last;
        ptr->m_type = GetType(ast->type());
        m_block->m_block.push_back(ptr);
        m_last = ptr;
        
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
            LoadDeref * ptr = new LoadDeref();
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
        Instruction * call;
        if (Type::isPointer(ast->function->type()))
            call = new Call();
        else
            call = new CallStatic();

        call->m_type = GetType(ast->type());

        BorderCall * start = new BorderCall();
        start->m_start = true;
        start->m_call = call;
        m_block->m_block.push_back(start);

        for (auto & x : ast->args)
        {
            visitChild(x);
            StoreParam * arg = new StoreParam();
            arg->m_address = m_last;
            m_block->m_block.push_back(arg);
            if (Call * c = dynamic_cast<Call*>(call))
                c->m_args.push_back(m_last);
            if (CallStatic * c = dynamic_cast<CallStatic*>(call))
                c->m_args.push_back(m_last);
        }

        m_leftValue = true;
        visitChild(ast->function);
        m_leftValue = false;

        if (Call * c = dynamic_cast<Call*>(call))
            c->m_fun_addr = m_last;
        if (CallStatic * c = dynamic_cast<CallStatic*>(call))
            c->m_fun_addr = dynamic_cast<Fun_address*>(m_last);

        m_block->m_block.push_back(call);
        m_last = call;
        BorderCall * end = new BorderCall();
        end->m_start = false;
        end->m_call = call;
        m_block->m_block.push_back(end);
    }
    
    void ASTtoIR::visit(ASTCast * ast)
    {
        visitChild(ast->value);
        AST * a = ast;
        ResultType t = GetType(a->type());

        if (t == ResultType::Integer && m_last->m_type == ResultType::Double)
            m_last = new Castdtoi(m_last);
        else if (t == ResultType::Double && m_last->m_type == ResultType::Integer)
            m_last = new Castitod(m_last);
        m_block->m_block.push_back(m_last);
        
    }
    
    void ASTtoIR::visit(ASTRead * ast)
    {
        //TODO
    }
    
    void ASTtoIR::visit(ASTWrite * ast)
    {
        visitChild(ast->value);
        DebugWrite * write = new DebugWrite(m_last);
        m_block->m_block.push_back(write);
        m_last = write;
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
    
    IRProgram * ASTtoIR::getIR() 
    {
        return m_prg;
    } 


    void ASTtoIR::EnterEnv()
    {
        Env * now = new Env();
        now->m_prev = m_env;
        m_env = now;
    }
    
    void ASTtoIR::LeaveEnv()
    {
        Env * now = m_env;
        m_env = m_env->m_prev;
        delete now;
    }
    
    void ASTtoIR::Env::AddVar(Symbol name, Instruction * ins)
    {
        auto it = m_map.insert(std::make_pair(name,ins));
        if (!it.second)
            throw std::runtime_error("Variable '" + name.name() + "' was already declared.");
    }


    Instruction * ASTtoIR::Env::FindVar(Symbol name)
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