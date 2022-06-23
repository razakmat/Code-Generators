#pragma once

#include "IR.h"
#include <vector>
#include <unordered_map>

namespace tinyc {
    bool FunSimplifyBinExpr(std::vector<Instruction*>*,int,std::unordered_map<Instruction*,Instruction*>&);
    bool FunStrengthReduction1(std::vector<Instruction*>*,int,std::unordered_map<Instruction*,Instruction*>&);
    bool FunStrengthReduction2(std::vector<Instruction*>*,int,std::unordered_map<Instruction*,Instruction*>&);


    class Peephole : public IRVisitor{
        public:
            Peephole();
            ~Peephole();
            void visit(Fun_address * ir);
            void visit(BorderCall * ir);
            void visit(StoreParam * ir);
            void visit(Call * ir);
            void visit(CallStatic * ir);
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
            template<typename T>
            void CheckType(T * ir);
            template<typename T>
            void CheckTypeBinary(T * ir);
            template<typename T>
            void CheckTypeUnary(T * ir);
            void InsertPattern(std::vector<Instruction*> * pat,
                    bool(*fun)(std::vector<Instruction*>*,int,std::unordered_map<Instruction*,Instruction*>&));
        protected:
            void Start();
            Instruction * Check(Instruction * ins);
            std::vector<Instruction*> * m_block;
            std::vector<std::vector<Instruction*>*> m_patterns;
            std::vector<bool(*)(std::vector<Instruction*>*,int,std::unordered_map<Instruction*,Instruction*>&)> m_funs;
            std::vector<bool> m_correct;
            std::unordered_map<Instruction*,Instruction*> m_new_ones;
            int m_current;
            int m_offset;
            int m_low;
            int m_high;
            bool m_all_false;
    };
}