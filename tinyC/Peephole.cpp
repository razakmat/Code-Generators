#include "Peephole.h"
#include <climits>

namespace tinyc
{
    bool FunStrengthReduction1(std::vector<Instruction*>* block,int start,
        std::unordered_map<Instruction*,Instruction*>& new_ones)
    {
        Load * left = dynamic_cast<Load*>((*block)[start]);
        Load_Imm_i * right = dynamic_cast<Load_Imm_i*>((*block)[start+1]);
        Mul * op = dynamic_cast<Mul*>((*block)[start+2]);

        int cur = right->m_value;
        int val = 0;
        while (cur % 2 == 0)
        {
            val++;
            cur = cur / 2;
        }
        if (cur != 1)
            return false;

        right->m_value = val;
        ShL * op1 = new ShL(left,right,op->m_type);
        (*block)[start+2] = op1;
        new_ones.insert(std::make_pair(op,op1));

        return true;
    }

    bool FunStrengthReduction2(std::vector<Instruction*>* block,int start,
        std::unordered_map<Instruction*,Instruction*>& new_ones)
    {
        Load * left = dynamic_cast<Load*>((*block)[start]);
        Load_Imm_i * right = dynamic_cast<Load_Imm_i*>((*block)[start+1]);
        Div * op = dynamic_cast<Div*>((*block)[start+2]);

        int cur = right->m_value;
        int val = 0;
        while (cur % 2 == 0)
        {
            val++;
            cur = cur / 2;
        }
        if (cur != 1)
            return false;

        right->m_value = val;
        ShR * op1 = new ShR(left,right,op->m_type);
        (*block)[start+2] = op1;
        new_ones.insert(std::make_pair(op,op1));

        return true;
    }

    bool FunSimplifyBinExpr(std::vector<Instruction*>* block,int start,
        std::unordered_map<Instruction*,Instruction*>& new_ones)
    {
        Load_Imm_i * left = dynamic_cast<Load_Imm_i*>((*block)[start]);
        Load_Imm_i * right = dynamic_cast<Load_Imm_i*>((*block)[start+1]);
        BinaryOp * op = dynamic_cast<BinaryOp*>((*block)[start+2]);

        Load_Imm_i * now = new Load_Imm_i();
        now->m_type = ResultType::Integer;

        if (dynamic_cast<Mul*>(op))
            now->m_value = left->m_value * right->m_value;
        else if (dynamic_cast<Div*>(op))
            now->m_value = left->m_value / right->m_value;
        else if (dynamic_cast<Add*>(op))
            now->m_value = left->m_value + right->m_value;
        else if (dynamic_cast<Sub*>(op))
            now->m_value = left->m_value - right->m_value;
        else{
            delete now;
            return false;
        }
        
        block->erase(block->begin() + start);
        block->erase(block->begin() + start);
        (*block)[start] = now;
        new_ones.insert(std::make_pair(op,now));
        return true;
    }

    Peephole::Peephole()
    {
        m_high = 0;
        m_low = INT_MAX;
    }
    
    Peephole::~Peephole()
    {
        for (auto & x : m_patterns)
        {
            for (auto & y : (*x))
            {
                delete y;
            }
            delete x;
        }
    }
    
    Instruction * Peephole::Check(Instruction * ins)
    {
        auto it = m_new_ones.find(ins);
        if (it != m_new_ones.end())
        {
            return it->second;
        }
        return ins;
    }

    void Peephole::visit(Fun_address * ir)
    {
        
    }
    
    void Peephole::visit(Call * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty()){
            for(int i = 0; i < ir->m_args.size(); i++)
            ir->m_args[i] = Check(ir->m_args[i]);
        }
    }
    
    void Peephole::visit(CallStatic * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty()){
            for(int i = 0; i < ir->m_args.size(); i++)
                ir->m_args[i] = Check(ir->m_args[i]);
        }
    }
    
    void Peephole::visit(BorderCall * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(StoreParam * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty())
            ir->m_address = Check(ir->m_address);
    }
    
    void Peephole::visit(LoadFun * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(Alloc_g * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(Alloc_l * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(Alloc_arg * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(Load * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty())
            ir->m_address = Check(ir->m_address);
    }
    
    void Peephole::visit(Load_Imm_i * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(Load_Imm_c * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(Load_Imm_d * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(Store * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty()){
            ir->m_address = Check(ir->m_address);
            ir->m_value = Check(ir->m_value);
        }
    }
    
    void Peephole::visit(LoadAddress * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty())
            ir->m_address = Check(ir->m_address);
    }
    
    void Peephole::visit(LoadDeref * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty())
            ir->m_address = Check(ir->m_address);
    }
    
    void Peephole::visit(Return * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty())
            ir->m_res = Check(ir->m_res);
    }
    
    void Peephole::visit(Jump_cond * ir)
    {
        CheckType(ir);
        if (!m_new_ones.empty())
            ir->m_cond = Check(ir->m_cond);
    }
    
    void Peephole::visit(Jump * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visit(Mul * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Div * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Mod * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Add * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Sub * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(ShL * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(ShR * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Gt * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Gte * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Lt * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Lte * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Eq * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(NEq * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(BitAnd * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(BitOr * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(And * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Or * ir)
    {
        CheckTypeBinary(ir);
        if (!m_new_ones.empty()){
            ir->m_left = Check(ir->m_left);
            ir->m_right = Check(ir->m_right);
        }
    }
    
    void Peephole::visit(Plus * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_left = Check(ir->m_left);
    }
    
    void Peephole::visit(Minus * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_left = Check(ir->m_left);
    }
    
    void Peephole::visit(Not * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_left = Check(ir->m_left);
    }
    
    void Peephole::visit(Neg * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_left = Check(ir->m_left);
    }

    void Peephole::visit(Inc * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_left = Check(ir->m_left);
    }
    
    void Peephole::visit(Dec * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_left = Check(ir->m_left);
    }
    
    void Peephole::visit(Castctoi * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_val = Check(ir->m_val);
    }
    
    void Peephole::visit(Castctod * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_val = Check(ir->m_val);
    }
    
    void Peephole::visit(Castitod * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_val = Check(ir->m_val);
    }
    
    void Peephole::visit(Castdtoi * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_val = Check(ir->m_val);
    }
    
    void Peephole::visit(DebugWrite * ir)
    {
        CheckTypeUnary(ir);
        if (!m_new_ones.empty())
            ir->m_val = Check(ir->m_val);
    }
    
    void Peephole::visit(NOP * ir)
    {
        CheckType(ir);
    }
    
    void Peephole::visitChild(Instruction * ir)
    {
        IRVisitor::visitChild(ir);
    }
    
    void Peephole::visit(Function * fun)
    {
        for (auto x : fun->m_blocks)
            visit(x);
    }
    
    void Peephole::visit(Block * block)
    {
        m_block = &block->m_block;
        Start();
        for (auto it = block->m_block.rbegin(); 
            it != block->m_block.rbegin() + m_low - 1 &&
            it != block->m_block.rend(); it++)
            visitChild(*it);
        for (auto & x : m_new_ones){
            delete x.first;
        }
        m_new_ones.clear();
    }
    
    void Peephole::visit(IRProgram * prg)
    {
        for (int i = 0; i < m_patterns.size(); i++)
                m_correct.push_back(true);
        m_block = &prg->m_decls;
        Start();
        for (auto & x : prg->m_funs)
            visit(x);
    }
    
    template<typename T>
    void Peephole::CheckType(T * ir)
    {
        int count = 0;
        for (int i = 0; i < m_patterns.size(); i++)
        {
            if (m_correct[i] && m_offset < m_patterns[i]->size())
            {
                if (!dynamic_cast<T*>((*m_patterns[i])[m_offset]))
                {
                    m_correct[i] = false;
                }
            }
            count += m_correct[i];
        }
        if (count == 0)
            m_all_false = true;
    }
    
    template<typename T>
    void Peephole::CheckTypeBinary(T * ir)
    {
        int count = 0;
        for (int i = 0; i < m_patterns.size(); i++)
        {
            if (m_correct[i] && m_offset < m_patterns[i]->size())
            {
                if (dynamic_cast<BinaryOp*>((*m_patterns[i])[m_offset]))
                {
                    if ((*m_patterns[i])[m_offset]->m_type != ResultType::Void)
                    {
                        if (!dynamic_cast<T*>((*m_patterns[i])[m_offset]))
                            m_correct[i] = false;
                    }
                }
                else
                    m_correct[i] = false;
            }
            count += m_correct[i];
        }
        if (count == 0)
            m_all_false = true;
    }
    
    template<typename T>
    void Peephole::CheckTypeUnary(T * ir)
    {
        int count = 0;
        for (int i = 0; i < m_patterns.size(); i++)
        {
            if (m_correct[i] && m_offset < m_patterns[i]->size())
            {
                if (dynamic_cast<UnaryOp*>((*m_patterns[i])[m_offset]))
                {
                    if ((*m_patterns[i])[m_offset]->m_type != ResultType::Void)
                    {
                        if (!dynamic_cast<T*>((*m_patterns[i])[m_offset]))
                            m_correct[i] = false;
                    }
                }
                else
                    m_correct[i] = false;
            }
            count += m_correct[i];
        }
        if (count == 0)
            m_all_false = true;
    }
    
    void Peephole::InsertPattern(std::vector<Instruction*> * pat,
        bool(*fun)(std::vector<Instruction*>*,int,std::unordered_map<Instruction*,Instruction*>&))
    {
        m_patterns.push_back(pat);
        if (pat->size() < m_low)
            m_low = pat->size();
        if (pat->size() > m_high)
            m_high = pat->size();
        m_funs.push_back(fun);
    }
    
    void Peephole::Start()
    {
        m_current = 0;
        while (m_block->size() - m_current >= m_low)
        {
            for (int i = 0; i < m_patterns.size(); i++)
                m_correct[i] = true;
            m_all_false = false;
            m_offset = 0;
            while (m_offset < m_high)
            {
                if (m_current+m_offset >= m_block->size() || m_all_false)
                    break;
                visitChild((*m_block)[m_current+m_offset]);
                m_offset++;
            }

            for (int i = 0; i < m_patterns.size(); i++)
            {
                if (m_correct[i])
                {
                    bool change = m_funs[i](m_block,m_current,m_new_ones);
                    if (change){
                        m_current--;
                        break;
                    }
                }
            }
            m_current++;
        }

    }
}