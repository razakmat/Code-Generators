#include "Inlining.h"
namespace tinyc
{
    Inlining::Inlining(std::vector<Function*> & funs)
    :m_funs(funs)
    {
    }
    
    void Inlining::Start()
    {
        bool again = true;

        while (again)
        {
            again = false;
            m_fun_available.clear();

            for (auto & x : m_funs)
                if (Inlineable(x))
                    m_fun_available.insert(x);

            if (!m_fun_available.empty())
                for (auto & x : m_funs)
                    if (SearchFun(x))
                        again = true;         
        }
    }
    
    bool Inlining::SearchFun(Function * fun)
    {
        for (int i = 0; i < fun->m_blocks.size(); i++){
            for (int j = 0; j < fun->m_blocks[i]->m_block.size(); j++){
                if (CallStatic * call = dynamic_cast<CallStatic*>(fun->m_blocks[i]->m_block[j])){
                    if (call->m_type == ResultType::Void || 
                        dynamic_cast<Store*>(fun->m_blocks[i]->m_block[j+2]))
                    {
                        auto it = m_fun_available.find(call->m_fun_addr->m_fun);
                        if (it != m_fun_available.end()){
                            InlineFun(fun,i,j,call->m_fun_addr->m_fun);
                            m_map_allocs.clear();
                            m_map_ins.clear();
                            m_map_block.clear();
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
    
    bool Inlining::Inlineable(Function * fun)
    {
        if(GetSize(fun) > m_size_limit)
            return false;
        if (HasCall(fun))
            return false;
        return true;
    }
    
    int Inlining::GetSize(Function* fun)
    {
        int size = 0;
        for (auto & x : fun->m_blocks){
            size += x->m_block.size();
        }
        return size;
    }
    
    bool Inlining::HasCall(Function* fun)
    {
        for (auto & x : fun->m_blocks){
            for (auto & y : x->m_block){
                if (dynamic_cast<CallStatic*>(y))
                    return true;
            }
        }
        return false;
    }
    
    void Inlining::InlineFun(Function * caller,int block_index, int call_index, Function * callee)
    {
        PreparingInline(caller,callee,block_index,call_index);
        for (int i = 0; i < callee->m_blocks.size(); i++)
        {
            FillBlock(caller->m_blocks[block_index + i + 1],callee->m_blocks[i]);
            m_map_ins.clear();
        }
    }
    
    void Inlining::PreparingInline(Function * caller, Function * callee, int block_index, int & call_index)
    {
        ChangeToNewVars(caller,callee);
        StoreParamToArgs(caller,caller->m_blocks[block_index],call_index);
        EraseCallBorders(caller->m_blocks[block_index],call_index);
        CreateBlocks(caller,callee,block_index);
        ChangeBlockWithCall(caller,caller->m_blocks[block_index],call_index,block_index);        
    }
    
    void Inlining::ChangeToNewVars(Function * caller,Function * callee)
    {
        for (int i = 0; i < callee->m_allocs.size(); i++){
            Alloc_l * var = new Alloc_l();
            Alloc_l * old = callee->m_allocs[i];
            var->m_type = old->m_type;
            caller->m_allocs.push_back(var);  
            m_map_allocs.insert(std::make_pair(old,var));
        }

        for (int i = 0; i < callee->m_args.size(); i++){
            Alloc_l * var = new Alloc_l();
            Alloc_arg * old = callee->m_args[i];
            var->m_type = old->m_type;
            caller->m_allocs.push_back(var);    
            m_map_allocs.insert(std::make_pair(old,var));     
        }
    }
    
    void Inlining::StoreParamToArgs(Function * caller,Block * block, int & call_index)
    {
        CallStatic * call = dynamic_cast<CallStatic*>(block->m_block[call_index]);
        
        int alloc_index = caller->m_allocs.size() - 1;
        int j = call_index - 1;
        for (int i = call->m_args.size() - 1; i >= 0; i--)
        {
            while (block->m_block[j] != call->m_args[i]){
                j--;
            }
            Store * store = new Store();
            store->m_address = caller->m_allocs[alloc_index];
            store->m_value = block->m_block[j];
            delete block->m_block[j+1];
            block->m_block[j+1] = store;
            alloc_index--;
        }

    }
    
    void Inlining::EraseCallBorders(Block * block,int & call_index)
    {
        for (int i = block->m_block.size() - 1 ; i >= 0; i--)
        {
            if (BorderCall * c = dynamic_cast<BorderCall*>(block->m_block[i]))
            {
                if (c->m_call == block->m_block[call_index]){
                    delete c;
                    block->m_block.erase(block->m_block.begin() + i);
                }
            }
        }
        call_index--;
    }
    
    void Inlining::CreateBlocks(Function * caller,Function * callee, int block_index)
    {
        for (int i = 0; i < callee->m_blocks.size(); i++)
        {
            Block * block = new Block();
            caller->m_blocks.insert(caller->m_blocks.begin() + block_index + i + 1,block);
            m_map_block.insert(std::make_pair(callee->m_blocks[i],block));
        }
        m_return_block = new Block();
        caller->m_blocks.insert(caller->m_blocks.begin() + block_index + callee->m_blocks.size() + 1, m_return_block);
    }
    
    void Inlining::ChangeBlockWithCall(Function * caller,Block * block, int call_index,int block_index)
    {
        int index = call_index;
        if (Store * s = dynamic_cast<Store*>(block->m_block[call_index+1])){
            m_call_address = s->m_address;
            index += 2;
        }
        else{
            m_call_address = nullptr;
            index++;
        }
        for (int i = index; i < block->m_block.size(); i++){
            m_return_block->m_block.push_back(block->m_block[i]);
        }
        if (m_return_block->m_block.empty()){
            if (m_return_block == caller->m_blocks[caller->m_blocks.size()-1]){
                NOP * n = new NOP;
                m_return_block->m_block.push_back(n);
            }
            else
            {
                int i;
                for (i = 0; i < caller->m_blocks.size(); i++){
                    if (m_return_block == caller->m_blocks[i])
                        break;
                }
                caller->m_blocks.erase(caller->m_blocks.begin() + i + 1);
                m_return_block = caller->m_blocks[i];
            }
        }
        delete block->m_block[call_index];
        Jump * j = new Jump();
        j->m_label = caller->m_blocks[block_index+1];
        block->m_block[call_index] = j;
        if (block->m_block.size() - 1 > call_index)
            block->m_block.erase(block->m_block.begin() + call_index + 1,block->m_block.end());
    }

    void Inlining::FillBlock(Block * to, Block * from)
    {
        for (auto & x : from->m_block)
        {
            if (Call * ins = dynamic_cast<Call*>(x))
            {
                Call * in = new Call();
                in->m_fun_addr = ins->m_fun_addr;
                in->m_type = ins->m_type;
                for (auto & x : ins->m_args){
                    auto it = m_map_ins.find(x);
                    in->m_args.push_back(it->second);
                }
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (BorderCall * ins = dynamic_cast<BorderCall*>(x))
            {
                BorderCall * in = new BorderCall();
                auto it = m_map_ins.find(ins->m_call);
                in->m_call = it->second;
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (StoreParam * ins = dynamic_cast<StoreParam*>(x))
            {
                StoreParam * in = new StoreParam();
                auto it = m_map_ins.find(ins->m_address);
                in->m_address = it->second;
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (LoadFun * ins = dynamic_cast<LoadFun*>(x))
            {
                LoadFun * in = new LoadFun();
                in->m_type = ins->m_type;
                in->m_address = ins->m_address;
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Load * ins = dynamic_cast<Load*>(x))
            {
                Load * in = new Load();
                in->m_type = ins->m_type;
                auto it = m_map_allocs.find(ins->m_address);
                in->m_address = it->second;
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Load_Imm_i * ins = dynamic_cast<Load_Imm_i*>(x))
            {
                Load_Imm_i * in = new Load_Imm_i();
                in->m_type = ins->m_type;
                in->m_value = ins->m_value;
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Load_Imm_d * ins = dynamic_cast<Load_Imm_d*>(x))
            {
                Load_Imm_d * in = new Load_Imm_d();
                in->m_type = ins->m_type;
                in->m_value = ins->m_value;
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Store * ins = dynamic_cast<Store*>(x))
            {
                Store * in = new Store();
                auto it = m_map_allocs.find(ins->m_address);
                in->m_address = it->second;
                it = m_map_ins.find(ins->m_value);
                in->m_value = it->second;
                in->m_type = ins->m_type;
                to->m_block.push_back(in);
            }
            else if (LoadAddress * ins = dynamic_cast<LoadAddress*>(x))
            {
                LoadAddress * in = new LoadAddress();
                in->m_type = ins->m_type;
                auto it = m_map_allocs.find(ins->m_address);
                in->m_address = it->second;
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (LoadDeref * ins = dynamic_cast<LoadDeref*>(x))
            {
                Load * in = new Load();
                in->m_type = ins->m_type;
                auto it = m_map_ins.find(ins->m_address);
                in->m_address = it->second;
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Return * ins = dynamic_cast<Return*>(x))
            {
                if (ins->m_res != nullptr)
                {
                    Store * s = new Store();
                    s->m_address = m_call_address;
                    auto it = m_map_ins.find(ins->m_res);
                    s->m_value = it->second;
                    s->m_type = ins->m_type;
                    to->m_block.push_back(s);
                }
                Jump * in = new Jump();
                in->m_label = m_return_block;
                to->m_block.push_back(in);
            }
            else if (Jump_cond * ins = dynamic_cast<Jump_cond*>(x))
            {
                Jump_cond * in = new Jump_cond();
                auto it = m_map_ins.find(ins->m_cond);
                in->m_cond = it->second;
                auto it1 = m_map_block.find(ins->m_true);
                in->m_true = it1->second;
                it1 = m_map_block.find(ins->m_false);
                in->m_false = it1->second;
                to->m_block.push_back(in);
            }
            else if (Jump * ins = dynamic_cast<Jump*>(x))
            {
                Jump * in = new Jump();
                auto it = m_map_block.find(ins->m_label);
                in->m_label = it->second;
                to->m_block.push_back(in);
            }
            else if (Mul * ins = dynamic_cast<Mul*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Mul * in = new Mul(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Div * ins = dynamic_cast<Div*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Div * in = new Div(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Mod * ins = dynamic_cast<Mod*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Mod * in = new Mod(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Add * ins = dynamic_cast<Add*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Add * in = new Add(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Sub * ins = dynamic_cast<Sub*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Sub * in = new Sub(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (ShL * ins = dynamic_cast<ShL*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                ShL * in = new ShL(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (ShR * ins = dynamic_cast<ShR*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                ShR * in = new ShR(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Gt * ins = dynamic_cast<Gt*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Gt * in = new Gt(left->second,right->second,ins->m_type);
                in->SetCmpJump(ins->GetCmpJump());
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Gte * ins = dynamic_cast<Gte*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Gte * in = new Gte(left->second,right->second,ins->m_type);
                in->SetCmpJump(ins->GetCmpJump());
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Lt * ins = dynamic_cast<Lt*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Lt * in = new Lt(left->second,right->second,ins->m_type);
                in->SetCmpJump(ins->GetCmpJump());
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Lte * ins = dynamic_cast<Lte*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Lte * in = new Lte(left->second,right->second,ins->m_type);
                in->SetCmpJump(ins->GetCmpJump());
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Eq * ins = dynamic_cast<Eq*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Eq * in = new Eq(left->second,right->second,ins->m_type);
                in->SetCmpJump(ins->GetCmpJump());
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (NEq * ins = dynamic_cast<NEq*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                NEq * in = new NEq(left->second,right->second,ins->m_type);
                in->SetCmpJump(ins->GetCmpJump());
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (BitAnd * ins = dynamic_cast<BitAnd*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                BitAnd * in = new BitAnd(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (BitOr * ins = dynamic_cast<BitOr*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                BitOr * in = new BitOr(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (And * ins = dynamic_cast<And*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                And * in = new And(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Or * ins = dynamic_cast<Or*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                auto right = m_map_ins.find(ins->m_right);
                Or * in = new Or(left->second,right->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Plus * ins = dynamic_cast<Plus*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                Plus * in = new Plus(left->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Minus * ins = dynamic_cast<Minus*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                Minus * in = new Minus(left->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Not * ins = dynamic_cast<Not*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                Not * in = new Not(left->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Neg * ins = dynamic_cast<Neg*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                Neg * in = new Neg(left->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Inc * ins = dynamic_cast<Inc*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                Inc * in = new Inc(left->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Dec * ins = dynamic_cast<Dec*>(x))
            {
                auto left = m_map_ins.find(ins->m_left);
                Dec * in = new Dec(left->second,ins->m_type);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Castdtoi * ins = dynamic_cast<Castdtoi*>(x))
            {
                auto it = m_map_ins.find(ins->m_val);
                Castdtoi * in = new Castdtoi(it->second);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (Castitod * ins = dynamic_cast<Castitod*>(x))
            {
                auto it = m_map_ins.find(ins->m_val);
                Castitod * in = new Castitod(it->second);
                to->m_block.push_back(in);
                m_map_ins.insert(std::make_pair(ins,in));
            }
            else if (NOP * ins = dynamic_cast<NOP*>(x))
            {
                NOP * in = new NOP();
                to->m_block.push_back(in);
            }
            else if (DebugWrite * ins = dynamic_cast<DebugWrite*>(x))
            {
                auto it = m_map_ins.find(ins->m_val);
                DebugWrite * in = new DebugWrite(it->second);
                in->m_type = ins->m_type;
                to->m_block.push_back(in);
            }
        }
    }


}