#include "IRTot86.h"
namespace tinyc
{
    using namespace tiny::t86;

    IRTot86::IRTot86(int maxReg)
    :m_max_regs(maxReg), m_last_label(Label::empty()), m_jump_label(Label::empty())
    {
        for (int i = 0; i < maxReg; i++)
            m_regs.insert(i);
    }
    
    void IRTot86::visit(Fun_address * ir)
    {
        visit(ir->m_fun);
    }
    
    void IRTot86::visit(Call * ir)
    {
        InsertToReg(ir->m_fun_addr);
        m_last_label = m_pb.add(tiny::t86::CALL{Reg(ir->m_fun_addr->m_memVal)});

        m_pb.add(ADD{Sp(),(int64_t)ir->m_args.size()});
        if (ir->m_type != ResultType::Void){
            m_pb.add(PUSH{Reg(m_max_regs-1)});
            ir->m_memType = 'x';
        }

    }
    
    void IRTot86::visit(CallStatic * ir)
    {
        auto it = m_funs.find(ir->m_fun_addr->m_fun->m_name);
        m_last_label = m_pb.add(tiny::t86::CALL{it->second});

        m_pb.add(ADD{Sp(),(int64_t)ir->m_args.size()});

        if (ir->m_fun_addr->m_fun->m_res_type != ResultType::Void){
                m_pb.add(PUSH{Reg(m_max_regs-1)});
                ir->m_memType = 'x';
        }
    }
    
    void IRTot86::visit(LoadFun * ir)
    {
        auto it = m_funs.find(ir->m_address->m_fun->m_name);
        int regNum = NextReg();
        m_last_label = m_pb.add(MOV{Reg(regNum),(int64_t)it->second});
        ir->m_memType = 'r';
        ir->m_memVal = regNum;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Alloc_g * ir)
    {
        ir->m_memType = 'm';
        ir->m_memVal = m_global_counter++;
    }
    
    void IRTot86::visit(Alloc_l * ir)
    {
        ir->m_memType = 's';
        ir->m_memVal = ++m_alloc_counter;
    }
    
    void IRTot86::visit(Alloc_arg * ir)
    {
        ir->m_memType = 's';
        ir->m_memVal = m_arg_counter++;
    }
    
    void IRTot86::visit(Load * ir)
    {
        if (ir->m_address->m_memType == 's')
        {
            int regNum = NextReg();
            m_last_label = m_pb.add(MOV{Reg(regNum), Mem(Bp() - ir->m_address->m_memVal)});
            ir->m_memType = 'r';
            ir->m_memVal = regNum;
        }
        else
        {
            int regNum = NextReg();
            m_last_label = m_pb.add(MOV{Reg(regNum), Mem(ir->m_address->m_memVal)});
            ir->m_memType = 'r';
            ir->m_memVal = regNum;
        }
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Load_Imm_i * ir)
    {
        if (m_regs.size() <= 2){
            m_last_label = m_pb.add(tiny::t86::NOP{});
            ir->m_memType = 'i';
            ir->m_memVal = ir->m_value;
            return;
        }

        int regNum = NextReg();
        m_last_label = m_pb.add(MOV{Reg(regNum), ir->m_value});
        ir->m_memType = 'r';
        ir->m_memVal = regNum;
    }
    
    void IRTot86::visit(Load_Imm_c * ir)
    {
        
    }
    
    void IRTot86::visit(Load_Imm_d * ir)
    {
        
    }
    
    void IRTot86::visit(Store * ir)
    {
        InsertToReg(ir->m_value);

        if (ir->m_address->m_memType == 's')
            m_last_label = m_pb.add(MOV{Mem(Bp() - ir->m_address->m_memVal), Reg(ir->m_value->m_memVal)});
        else if (ir->m_address->m_memType == 'm')
            m_last_label = m_pb.add(MOV{Mem(ir->m_address->m_memVal), Reg(ir->m_value->m_memVal)});
        else{
            InsertToReg(ir->m_address);
            m_last_label = m_pb.add(MOV{Mem(Reg(ir->m_address->m_memVal)),Reg(ir->m_value->m_memVal)});
            m_regs.insert(ir->m_address->m_memVal);
        }

        ir->m_memType = ir->m_address->m_memType;
        ir->m_memVal = ir->m_address->m_memVal;
        m_regs.insert(ir->m_value->m_memVal);
    }
    
    void IRTot86::visit(LoadAddress * ir)
    {
        
        int regNum = NextReg();
        if (ir->m_address->m_memType == 's'){
            m_last_label = m_pb.add(LEA{Reg(regNum), Mem(Bp() - ir->m_address->m_memVal)});
            ir->m_memType = 'r';
            ir->m_memVal = regNum;
            CheckSpill(ir);
        }
        else{
            if (m_regs.size() < 2){
                ir->m_memType = 'i';
                ir->m_memVal = ir->m_address->m_memVal;
                m_regs.insert(regNum);
            }
            else{
                m_last_label = m_pb.add(MOV{Reg(regNum), ir->m_address->m_memVal});
                ir->m_memType = 'r';
                ir->m_memVal = regNum;
            }
        }
    }

    void IRTot86::visit(LoadDeref * ir)
    {
        InsertToReg(ir->m_address);
        int regNum = NextReg();

        m_last_label = m_pb.add(MOV{Reg(regNum), Mem(Reg(ir->m_address->m_memVal))});

        m_regs.insert(ir->m_address->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = regNum;
        CheckSpill(ir);
    }

    void IRTot86::visit(Return * ir)
    {
        if (ir->m_res != nullptr)
        {
            InsertToReg(ir->m_res);
            m_pb.add(MOV{Reg(m_max_regs-1),Reg(ir->m_res->m_memVal)});
            m_regs.insert(ir->m_res->m_memVal);
        }
        Label ret = m_pb.add(JMP{Label::empty()});
        m_rets.push_back(ret);
    }
    
    void IRTot86::visit(Jump_cond * ir)
    {
        m_patch_labels.push_back(std::make_pair(m_jump_label,ir->m_true));
        m_jump_label = m_pb.add(JMP{Label::empty()});
        m_patch_labels.push_back(std::make_pair(m_jump_label,ir->m_false));
        m_last_label = m_jump_label;
    }
    
    void IRTot86::visit(Jump * ir)
    {
        m_jump_label = m_pb.add(JMP{Label::empty()});
        m_patch_labels.push_back(std::make_pair(m_jump_label,ir->m_label));
        m_last_label = m_jump_label;
    }
    
    void IRTot86::visit(Mul * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(IMUL{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(Div * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(IDIV{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(Mod * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(MOD{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(Add * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(ADD{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(Sub * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(SUB{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(ShL * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(LSH{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(ShR * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(RSH{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(Gt * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(CMP{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        m_jump_label = m_pb.add(JG{Label::empty()});

        if (ir->GetCmpJump())
        {
            m_regs.insert(ir->m_left->m_memVal);
        }
        else
        {
            m_pb.add(XOR{Reg(ir->m_left->m_memVal),Reg(ir->m_left->m_memVal)});
            Label g = m_pb.add(JMP{Label::empty()});
            Label to = m_pb.add(MOV{Reg(ir->m_left->m_memVal),1});
            Label e = m_pb.add(tiny::t86::NOP{});
            m_pb.patch(m_jump_label,to);
            m_pb.patch(g,e);
            
            ir->m_memType = 'r';
            ir->m_memVal = ir->m_left->m_memVal;
            CheckSpill(ir);
        }
    }
    
    void IRTot86::visit(Gte * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(CMP{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        m_jump_label = m_pb.add(JGE{Label::empty()});

        if (ir->GetCmpJump())
        {
            m_regs.insert(ir->m_left->m_memVal);
        }
        else
        {
            m_pb.add(XOR{Reg(ir->m_left->m_memVal),Reg(ir->m_left->m_memVal)});
            Label g = m_pb.add(JMP{Label::empty()});
            Label to = m_pb.add(MOV{Reg(ir->m_left->m_memVal),1});
            Label e = m_pb.add(tiny::t86::NOP{});
            m_pb.patch(m_jump_label,to);
            m_pb.patch(g,e);

            ir->m_memType = 'r';
            ir->m_memVal = ir->m_left->m_memVal;
            CheckSpill(ir);
        }
        
    }
    
    void IRTot86::visit(Lt * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(CMP{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        m_jump_label = m_pb.add(JL{Label::empty()});
        
        if (ir->GetCmpJump())
        {
            m_regs.insert(ir->m_left->m_memVal);
        }
        else
        {
            m_pb.add(XOR{Reg(ir->m_left->m_memVal),Reg(ir->m_left->m_memVal)});
            Label g = m_pb.add(JMP{Label::empty()});
            Label to = m_pb.add(MOV{Reg(ir->m_left->m_memVal),1});
            Label e = m_pb.add(tiny::t86::NOP{});
            m_pb.patch(m_jump_label,to);
            m_pb.patch(g,e);
            
            ir->m_memType = 'r';
            ir->m_memVal = ir->m_left->m_memVal;
            CheckSpill(ir);
        }
    }
    
    void IRTot86::visit(Lte * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);
    
        m_last_label = m_pb.add(CMP{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        m_jump_label = m_pb.add(JLE{Label::empty()});
        
        if (ir->GetCmpJump())
        {
            m_regs.insert(ir->m_left->m_memVal);
        }
        else
        {
            m_pb.add(XOR{Reg(ir->m_left->m_memVal),Reg(ir->m_left->m_memVal)});
            Label g = m_pb.add(JMP{Label::empty()});
            Label to = m_pb.add(MOV{Reg(ir->m_left->m_memVal),1});
            Label e = m_pb.add(tiny::t86::NOP{});
            m_pb.patch(m_jump_label,to);
            m_pb.patch(g,e);
            
            ir->m_memType = 'r';
            ir->m_memVal = ir->m_left->m_memVal;
            CheckSpill(ir);
        }     
    }
    
    void IRTot86::visit(Eq * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(CMP{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        m_jump_label = m_pb.add(JE{Label::empty()});
        
        if (ir->GetCmpJump())
        {
            m_regs.insert(ir->m_left->m_memVal);
        }
        else
        {
            m_pb.add(XOR{Reg(ir->m_left->m_memVal),Reg(ir->m_left->m_memVal)});
            Label g = m_pb.add(JMP{Label::empty()});
            Label to = m_pb.add(MOV{Reg(ir->m_left->m_memVal),1});
            Label e = m_pb.add(tiny::t86::NOP{});
            m_pb.patch(m_jump_label,to);
            m_pb.patch(g,e);
            
            ir->m_memType = 'r';
            ir->m_memVal = ir->m_left->m_memVal;
            CheckSpill(ir);
        }     
    }
    
    void IRTot86::visit(NEq * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(CMP{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        m_jump_label = m_pb.add(JNE{Label::empty()});
        
        if (ir->GetCmpJump())
        {
            m_regs.insert(ir->m_left->m_memVal);
        }
        else
        {
            m_pb.add(XOR{Reg(ir->m_left->m_memVal),Reg(ir->m_left->m_memVal)});
            Label g = m_pb.add(JMP{Label::empty()});
            Label to = m_pb.add(MOV{Reg(ir->m_left->m_memVal),1});
            Label e = m_pb.add(tiny::t86::NOP{});
            m_pb.patch(m_jump_label,to);
            m_pb.patch(g,e);
            
            ir->m_memType = 'r';
            ir->m_memVal = ir->m_left->m_memVal;
            CheckSpill(ir);
        }
   
    }
    
    void IRTot86::visit(BitAnd * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(AND{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(BitOr * ir)
    {
        InsertToReg(ir->m_right);
        InsertToReg(ir->m_left);

        m_last_label = m_pb.add(OR{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_right->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;

        CheckSpill(ir);
    }
    
    void IRTot86::visit(And * ir)
    {
        InsertToReg(ir->m_right);

        m_pb.add(CMP{Reg(ir->m_right->m_memVal),0});
        m_pb.add(MOV{Reg(ir->m_right->m_memVal),Flags()});
        m_pb.add(RSH{Reg(ir->m_right->m_memVal),1});
        m_pb.add(XOR{Reg(ir->m_right->m_memVal),1});
        m_pb.add(AND{Reg(ir->m_right->m_memVal),1});

        InsertToReg(ir->m_left);

        m_pb.add(CMP{Reg(ir->m_left->m_memVal),0});
        m_pb.add(MOV{Reg(ir->m_left->m_memVal),Flags()});
        m_pb.add(RSH{Reg(ir->m_left->m_memVal),1});
        m_pb.add(XOR{Reg(ir->m_left->m_memVal),1});
        m_pb.add(AND{Reg(ir->m_left->m_memVal),1});

        m_pb.add(AND{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_left->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Or * ir)
    {
        InsertToReg(ir->m_right);

        m_pb.add(CMP{Reg(ir->m_right->m_memVal),0});
        m_pb.add(MOV{Reg(ir->m_right->m_memVal),Flags()});
        m_pb.add(RSH{Reg(ir->m_right->m_memVal),1});
        m_pb.add(XOR{Reg(ir->m_right->m_memVal),1});
        m_pb.add(AND{Reg(ir->m_right->m_memVal),1});

        InsertToReg(ir->m_left);

        m_pb.add(CMP{Reg(ir->m_left->m_memVal),0});
        m_pb.add(MOV{Reg(ir->m_left->m_memVal),Flags()});
        m_pb.add(RSH{Reg(ir->m_left->m_memVal),1});
        m_pb.add(XOR{Reg(ir->m_left->m_memVal),1});
        m_pb.add(AND{Reg(ir->m_left->m_memVal),1});

        m_pb.add(OR{Reg(ir->m_left->m_memVal),Reg(ir->m_right->m_memVal)});
        m_regs.insert(ir->m_left->m_memVal);
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Plus * ir)
    {
        ir->m_memType = ir->m_left->m_memType;
        ir->m_memVal = ir->m_left->m_memVal;
    }
    
    void IRTot86::visit(Minus * ir)
    {
        InsertToReg(ir->m_left);
        m_last_label = m_pb.add(NEG{Reg(ir->m_left->m_memVal)});
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Not * ir)
    {
        InsertToReg(ir->m_left);

        m_pb.add(CMP{Reg(ir->m_left->m_memVal),0});
        m_pb.add(MOV{Reg(ir->m_left->m_memVal),Flags()});
        m_pb.add(RSH{Reg(ir->m_left->m_memVal),1});
        m_pb.add(AND{Reg(ir->m_left->m_memVal),1});
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Neg * ir)
    {
        InsertToReg(ir->m_left);
        m_last_label = m_pb.add(NOT{Reg(ir->m_left->m_memVal)});
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;
        CheckSpill(ir);
    }

    void IRTot86::visit(Inc * ir)
    {
        InsertToReg(ir->m_left);
        m_last_label = m_pb.add(INC{Reg(ir->m_left->m_memVal)});
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Dec * ir)
    {
        InsertToReg(ir->m_left);
        m_last_label = m_pb.add(DEC{Reg(ir->m_left->m_memVal)});
        ir->m_memType = 'r';
        ir->m_memVal = ir->m_left->m_memVal;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Castctoi * ir)
    {
        ir->m_type = ResultType::Integer;
        ir->m_memType = ir->m_val->m_memType;
        ir->m_memVal = ir->m_val->m_memVal;
    }
    
    void IRTot86::visit(Castctod * ir)
    {
        InsertToReg(ir->m_val);
        m_last_label = m_pb.add(EXT{FReg(0),Reg(ir->m_val->m_memVal)});
        ir->m_memType = 'r';
        ir->m_memVal = 0;
        ir->m_type = ResultType::Double;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Castitod * ir)
    {
        InsertToReg(ir->m_val);
        m_last_label = m_pb.add(EXT{FReg(0),Reg(ir->m_val->m_memVal)});
        ir->m_memType = 'r';
        ir->m_memVal = 0;
        ir->m_type = ResultType::Double;
        CheckSpill(ir);
    }
    
    void IRTot86::visit(Castdtoi * ir)
    {
        InsertToReg(ir->m_val);
        m_last_label = m_pb.add(NRW{Reg(0),FReg(ir->m_val->m_memVal)});
        ir->m_memType = 'r';
        ir->m_memVal = 0;
        ir->m_type = ResultType::Integer;
        CheckSpill(ir);
    }
    
    void IRTot86::visitChild(Instruction * ir)
    {
        IRVisitor::visitChild(ir);
    }
    
    void IRTot86::visit(Function * fun)
    {
        Label f = m_pb.add(PUSH{Bp()});
        m_funs.insert(std::make_pair(fun->m_name,f));
        m_pb.add(MOV{Bp(),Sp()});

        m_arg_counter = -fun->m_args.size() - 1;
        for (auto & x : fun->m_args){
            visit(x);
        }
        m_alloc_counter = 0;
        for (auto & x : fun->m_allocs)
            visit(x);

        m_pb.add(SUB{Sp(), m_alloc_counter});


        for (auto & x : fun->m_blocks)
            visit(x);
        
        Label forRet = m_pb.add(ADD{Sp(), m_alloc_counter});
        for (auto & x : m_rets)
            m_pb.patch(x,forRet);
        m_rets.clear();

        m_pb.add(POP{Bp()});

        if (fun->m_name != "main")
            m_pb.add(RET{});
        else
            m_pb.add(HALT{});

        PatchLabels();        
    }
    
    void IRTot86::visit(Block * block)
    {
        bool first = true;
        for (auto & x : block->m_block){
            IRVisitor::visitChild(x);
            if (first)
            {
                AddLabel(block);
                first = false;
            }
        }
        
    }
    
    void IRTot86::visit(IRProgram * prg)
    {
        m_global_counter = 0;
        for (auto & x : prg->m_allocs_g)
            visit(x);


        Label start = m_pb.add(JMP{Label::empty()});

        for (auto & x : prg->m_funs)
           visit(x);        

        Label global = m_pb.add(tiny::t86::NOP{});
        m_pb.patch(start,global);
        for (auto & x : prg->m_decls)
            IRVisitor::visitChild(x);

        start = m_pb.add(JMP{Label::empty()});
        auto it = m_funs.find("main");
        m_pb.patch(start,it->second);

    }
    
    void IRTot86::visit(DebugWrite * ir)
    {
        InsertToReg(ir->m_val);
        m_last_label = m_pb.add(MOV{Reg(0),Reg(ir->m_val->m_memVal)});
        m_pb.add(DBG{ [](const Cpu & cpu){std::cout << cpu.getRegister(Reg(0)) << std::endl;}});
        m_regs.insert(ir->m_val->m_memVal);
    }
    
    void IRTot86::visit(NOP * ir)
    {
       m_last_label = m_pb.add(tiny::t86::NOP{});
    }
    
    void IRTot86::visit(BorderCall * ir)
    {
        if (ir->m_start)
        {
            m_last_label = m_pb.add(tiny::t86::NOP{});
            std::vector<int> spilled_regs;
            AllUsedRegs(spilled_regs);
            for (int i = 0; i < spilled_regs.size(); i++)
            {
                m_pb.add(PUSH{Reg(spilled_regs[i])});
                m_regs.insert(spilled_regs[i]);
            }
            m_spilled_regs.push_back(std::move(spilled_regs));        
        }
        else
        {
            std::vector<int> & spilled_regs = m_spilled_regs[m_spilled_regs.size()-1]; 

                for (auto x = spilled_regs.rbegin(); x != spilled_regs.rend(); ++x)
                {
                    m_pb.add(POP{Reg(*x)});
                    auto it = m_regs.find(*x);
                    if (it != m_regs.end())
                        m_regs.erase(it);
                }
            m_spilled_regs.pop_back();
        }
    }
    
    void IRTot86::visit(StoreParam * ir)
    {
        InsertToReg(ir->m_address);
        m_pb.add(PUSH{Reg(ir->m_address->m_memVal)});
        ir->m_address->m_memType = 'x';
        m_regs.insert(ir->m_address->m_memVal);
    }
    
    Program IRTot86::GetProgram()
    {
        return m_pb.program();
    }
    
    int IRTot86::NextReg()
    {
        return m_regs.extract(m_regs.begin()).value();
    }
    
    void IRTot86::AllUsedRegs(std::vector<int> & regs)
    {
        int current = 0;
        for (auto & x : m_regs){
            while (x > current){
                regs.push_back(current++);
            }
            current++;
        }
    }
    
    void IRTot86::ValuesToSpill(std::vector<int> & regs,std::vector<Instruction*> & args)
    {
        std::set<int> reg_args;
        for (auto & x : args)
            if (x->m_memType == 'r')
                reg_args.insert(x->m_memVal);
        int current = 0;
        for (auto & x : m_regs){
            while (x > current){
                auto it = reg_args.find(current);
                if (it == reg_args.end())
                    regs.push_back(current++);
                else
                    current++;
            }
            current++;
        }
    }
    
    void IRTot86::AddLabel(Block * block)
    {
        m_block_labels.insert(std::make_pair(block,m_last_label));
    }
    
    void IRTot86::PatchLabels()
    {
        for (auto & x : m_patch_labels)
        {
            auto it = m_block_labels.find(x.second);
            m_pb.patch(x.first,it->second);
        }
        m_patch_labels.clear();
        m_block_labels.clear();
    }
    
    void IRTot86::CheckSpill(Instruction * ir)
    {
        if (ir->m_memType == 'r' && ir->m_type == ResultType::Integer && m_regs.size() < 2){
            m_pb.add(PUSH{Reg(ir->m_memVal)});
            ir->m_memType = 'x';
            m_regs.insert(ir->m_memVal);
        }
    }
    
    void IRTot86::InsertToReg(Instruction * ir)
    {
        if (ir->m_memType == 'r')
            return;

        int regNum = NextReg();
        if (ir->m_memType == 'm'){
            m_pb.add(MOV{Reg(regNum),Mem(ir->m_memVal)});
        }
        else if (ir->m_memType == 'x'){
            m_pb.add(POP{Reg(regNum)});
        }
        else if (ir->m_memType == 's'){
            m_pb.add(MOV{Reg(regNum),Mem(Bp() - ir->m_memVal)});
        }
        else if (ir->m_memType == 'i'){
            m_pb.add(MOV{Reg(regNum),ir->m_memVal});
        }
        ir->m_memType = 'r';
        ir->m_memVal = regNum;
    }
    
    int IRTot86::FindMaxArgs(Function * fun)
    {
        int max = 0;
        for (auto & x : fun->m_blocks)
        {
            for (auto & y : x->m_block)
            {
                if (CallStatic * call = dynamic_cast<CallStatic*>(y))
                {
                    int size = call->m_args.size();
                    if (size > max)
                        max = size;
                }
            }
        }
        return max;
    }
}