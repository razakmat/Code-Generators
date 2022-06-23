#pragma once

#include "IR.h"
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <vector>
#include <string>

#include "../tiny86/cpu.h"
#include "../tiny86/program.h"
#include "../tiny86/program/programbuilder.h"
#include "../tiny86/program/helpers.h"

namespace tinyc {

    using namespace tiny::t86;

    class IRTot86 : public IRVisitor {
        public:
            IRTot86(int maxReg = 10);
            void visit(Fun_address * ir);
            void visit(Call * ir);
            void visit(CallStatic * ir);
            void visit(BorderCall * ir);
            void visit(StoreParam * ir);
            void visit(LoadFun * ir);
            void visit(Alloc_g * ir);
            void visit(Alloc_l * ir);
            void visit(Alloc_arg * ir);
            void visit(Load * ir);
            void visit(Load_Imm_i * ir);
            void visit(Load_Imm_c * ir);
            void visit(Load_Imm_d * ir);
            void visit(Store * ir);
            void visit(LoadAddress * ir);
            void visit(LoadDeref * ir);
            void visit(Return * ir);
            void visit(Jump_cond * ir);
            void visit(Jump * ir);
            void visit(Mul * ir);
            void visit(Div * ir);
            void visit(Mod * ir);
            void visit(Add * ir);
            void visit(Sub * ir);
            void visit(ShL * ir);
            void visit(ShR * ir);
            void visit(Gt * ir);
            void visit(Gte * ir);
            void visit(Lt * ir);
            void visit(Lte * ir);
            void visit(Eq * ir);
            void visit(NEq * ir);
            void visit(BitAnd * ir);
            void visit(BitOr * ir);
            void visit(And * ir);
            void visit(Or * ir);
            void visit(Plus * ir);
            void visit(Minus * ir);
            void visit(Not * ir);
            void visit(Neg * ir);
            void visit(Inc * ir);
            void visit(Dec * ir);
            void visit(Castctoi * ir);
            void visit(Castctod * ir);
            void visit(Castitod * ir);
            void visit(Castdtoi * ir);
            void visit(DebugWrite * ir);
            void visit(NOP * ir);
            void visitChild(Instruction * ir);
            void visit(Function * fun);
            void visit(Block * block);
            void visit(IRProgram * prg);
            Program GetProgram();
        protected:
            int NextReg();
            void AllUsedRegs(std::vector<int> & regs);
            void ValuesToSpill(std::vector<int> & regs,std::vector<Instruction*> & args);
            void AddLabel(Block * block);
            void PatchLabels();
            void CheckSpill(Instruction * ir);
            void InsertToReg(Instruction * ir);
            int FindMaxArgs(Function * fun);
            ProgramBuilder m_pb;
            int m_alloc_counter;
            int m_global_counter;
            int m_arg_counter;
            int m_max_regs;
            std::set<int> m_regs;
            std::unordered_map<Block*,Label> m_block_labels;
            std::vector<std::pair<Label,Block*>> m_patch_labels;
            std::unordered_map<std::string,Label> m_funs;
            std::vector<Label> m_rets;
            std::vector<std::vector<int>> m_spilled_regs;
            Label m_last_label;
            Label m_jump_label;
            

    };
}