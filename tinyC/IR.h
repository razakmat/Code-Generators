#pragma once

#include <vector>
#include <memory>
#include <string>
#include "ast.h"
#include "../common/types.h"

namespace tinyc {
    
    class Alloc_l;
    class Alloc_arg;
    class IRVisitor;
    enum class ResultType {
        Integer,
        Char,
        Double,
        Void
    };

    class Instruction {
        public:
            Instruction();
            Instruction(ResultType t);
            ResultType m_type;
            char m_memType; // 'r' - register, 's' - stack, 'm' - memory, 'x  - spill, 'i' - integer;
            int64_t m_memVal;
        protected:
            friend class IRVisitor;
            virtual void accept(IRVisitor * v) = 0;
    };

    class Block {
        public:
            Block();
            std::vector<Instruction*> m_block;
            int m_name;
            inline static int counter;
    };
    class Fun_address;
    class Function {
        public:
            Function(const std::string & name);
            std::string m_name;
            std::vector<Block*> m_blocks;
            std::vector<Alloc_arg*> m_args;
            std::vector<Alloc_l*> m_allocs;
            ResultType m_res_type;
            Fun_address * m_addr;
    };

    class Fun_address : public Instruction {
        public:
            Fun_address(Function * fun);
            Function * m_fun;
        protected:
            void accept(IRVisitor * v) override;
    };


    class BorderCall : public Instruction {
        public:
            bool m_start;
            Instruction * m_call;
        protected:
            void accept(IRVisitor * v) override;
    };

    class StoreParam : public Instruction {
        public:
            Instruction * m_address;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Call : public Instruction {
        public:
            Instruction * m_fun_addr;
            std::vector<Instruction*> m_args;
        protected:
            void accept(IRVisitor * v) override;
    };

    class CallStatic : public Instruction {
        public:
            Fun_address * m_fun_addr;
            std::vector<Instruction*> m_args;
            BorderCall * m_start;
        protected:
            void accept(IRVisitor * v) override;
    };

    class LoadFun : public Instruction {
        public:
            Fun_address * m_address;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Alloc_g : public Instruction {
        public:
            ResultType m_type;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Alloc_l : public Instruction {
        public:
            ResultType m_type;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Alloc_arg : public Instruction {
        public:
            ResultType m_type;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Load : public Instruction {
        public:
            Instruction *  m_address;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Load_Imm_i : public Instruction {
        public:
            int64_t m_value;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Load_Imm_d : public Instruction {
        public:
            double m_value;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Load_Imm_c : public Instruction {
        public:
            int8_t m_value;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Store : public Instruction {
        public:
            Instruction *  m_address;
            Instruction * m_value;
        protected:
            void accept(IRVisitor * v) override;
    };

    class LoadAddress : public Instruction {
        public:
            Instruction * m_address;
        protected:
            void accept(IRVisitor * v) override;
    };

    class LoadDeref : public Instruction {
        public:
            Instruction * m_address;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Return : public Instruction {
        public:
            Instruction * m_res;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Jump_cond : public Instruction {
        public:
            Instruction * m_cond;
            Block *  m_true;
            Block * m_false;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Jump : public Instruction {
        public:
            Block * m_label;
        protected:
            void accept(IRVisitor * v) override;
    };

    class BinaryOp : public Instruction {
        public:
            BinaryOp(Instruction * left, Instruction * right, ResultType t);
            Instruction * m_left;
            Instruction * m_right;
        protected:
            virtual void accept(IRVisitor * v);
    };

    class UnaryOp : public Instruction {
        public:
            UnaryOp(Instruction * ins,ResultType t);
            Instruction * m_left;
        protected:
            virtual void accept(IRVisitor * v);
    };



    class Mul : public BinaryOp {
        public:
            Mul(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Div : public BinaryOp {
        public:
            Div(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Mod : public BinaryOp {
        public:
            Mod(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Add : public BinaryOp {
        public:
            Add(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Sub : public BinaryOp {
        public:
            Sub(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class ShL : public BinaryOp {
        public:
            ShL(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class ShR : public BinaryOp {
        public:
            ShR(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class CmpOp : public BinaryOp {
        public:
            CmpOp(Instruction * left, Instruction * right, ResultType t);
            void SetCmpJump(bool b);
            bool GetCmpJump();
        protected:
            virtual void accept(IRVisitor * v) = 0;
            bool m_cpm_jump;
    };

    class Gt : public CmpOp {
        public:
            Gt(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Gte : public CmpOp {
        public:
            Gte(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Lt : public CmpOp {
        public:
            Lt(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Lte : public CmpOp {
        public:
            Lte(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Eq : public CmpOp {
        public:
            Eq(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class NEq : public CmpOp {
        public:
            NEq(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class BitAnd : public BinaryOp {
        public:
            BitAnd(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class BitOr : public BinaryOp {
        public:
            BitOr(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class And : public BinaryOp {
        public:
            And(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Or : public BinaryOp {
        public:
            Or(Instruction * left, Instruction * right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Plus : public UnaryOp {
        public:
            Plus(Instruction * ins, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Minus : public UnaryOp {
        public:
            Minus(Instruction * ins, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Not : public UnaryOp {
        public:
            Not(Instruction * ins, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Neg : public UnaryOp {
        public:
            Neg(Instruction * ins, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Inc : public UnaryOp {
        public:
            Inc(Instruction * ins, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Dec : public UnaryOp {
        public:
            Dec(Instruction * ins, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Castctoi : public Instruction {
        public:
            Castctoi(Instruction * val);
            Instruction * m_val;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Castctod : public Instruction {
        public:
            Castctod(Instruction * val);
            Instruction * m_val;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Castitod : public Instruction {
        public:
            Castitod(Instruction * val);
            Instruction * m_val;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Castdtoi : public Instruction {
        public:
            Castdtoi(Instruction * val);
            Instruction * m_val;
        protected:
            void accept(IRVisitor * v) override;
    };

    class NOP : public Instruction {
        public:
        protected:
            void accept(IRVisitor * v) override;
    };

    class DebugWrite : public Instruction {
        public:
            DebugWrite(Instruction * val);
            Instruction * m_val;
        protected:
            void accept(IRVisitor * v) override;
    };

    class IRProgram {
        public:
            ~IRProgram();
            std::vector<Function*> m_funs;
            std::vector<Instruction*> m_decls;
            std::vector<Alloc_g*> m_allocs_g;
    };

    class IRVisitor {
        public:
            virtual void visit(Fun_address * ir) = 0;
            virtual void visit(BorderCall * ir) = 0;
            virtual void visit(StoreParam * ir) = 0;
            virtual void visit(Call * ir) = 0;
            virtual void visit(CallStatic * ir) = 0;
            virtual void visit(LoadFun * ir) = 0;
            virtual void visit(Alloc_g * ir) = 0;
            virtual void visit(Alloc_l * ir) = 0;
            virtual void visit(Alloc_arg * ir) = 0;
            virtual void visit(Load * ir) = 0;
            virtual void visit(Load_Imm_i * ir) = 0;
            virtual void visit(Load_Imm_c * ir) = 0;
            virtual void visit(Load_Imm_d * ir) = 0;
            virtual void visit(Store * ir) = 0;
            virtual void visit(LoadAddress * ir) = 0;
            virtual void visit(LoadDeref * ir) = 0;
            virtual void visit(Return * ir) = 0;
            virtual void visit(Jump_cond * ir) = 0;
            virtual void visit(Jump * ir) = 0;
            virtual void visit(Mul * ir) = 0;
            virtual void visit(Div * ir) = 0;
            virtual void visit(Mod * ir) = 0;
            virtual void visit(Add * ir) = 0;
            virtual void visit(Sub * ir) = 0;
            virtual void visit(ShL * ir) = 0;
            virtual void visit(ShR * ir) = 0;
            virtual void visit(Gt * ir) = 0;
            virtual void visit(Gte * ir) = 0;
            virtual void visit(Lt * ir) = 0;
            virtual void visit(Lte * ir) = 0;
            virtual void visit(Eq * ir) = 0;
            virtual void visit(NEq * ir) = 0;
            virtual void visit(BitAnd * ir) = 0;
            virtual void visit(BitOr * ir) = 0;
            virtual void visit(And * ir) = 0;
            virtual void visit(Or * ir) = 0;
            virtual void visit(Plus * ir) = 0;
            virtual void visit(Minus * ir) = 0;
            virtual void visit(Not * ir) = 0;
            virtual void visit(Neg * ir) = 0;
            virtual void visit(Inc * ir) = 0;
            virtual void visit(Dec * ir) = 0;
            virtual void visit(Castctoi * ir) = 0;
            virtual void visit(Castctod * ir) = 0;
            virtual void visit(Castitod * ir) = 0;
            virtual void visit(Castdtoi * ir) = 0;
            virtual void visit(DebugWrite * ir) = 0;
            virtual void visit(NOP * ir) = 0;
        protected:
            void visitChild(Instruction * child) {
                child->accept(this);
            }
    };


    inline void Load_Imm_d::accept(IRVisitor * v) { v->visit(this); }
    inline void Load_Imm_i::accept(IRVisitor * v) { v->visit(this); }
    inline void Load_Imm_c::accept(IRVisitor * v) { v->visit(this); }
    inline void Load::accept(IRVisitor * v) { v->visit(this); }
    inline void Alloc_g::accept(IRVisitor * v) { v->visit(this); }
    inline void Alloc_l::accept(IRVisitor * v) { v->visit(this); }
    inline void Store::accept(IRVisitor * v) { v->visit(this); }
    inline void LoadAddress::accept(IRVisitor * v) { v->visit(this); }
    inline void LoadDeref::accept(IRVisitor * v) { v->visit(this); }
    inline void LoadFun::accept(IRVisitor * v) { v->visit(this); }
    inline void Alloc_arg::accept(IRVisitor * v) { v->visit(this); }
    inline void Jump_cond::accept(IRVisitor * v) { v->visit(this); }
    inline void Jump::accept(IRVisitor * v) { v->visit(this); }
    inline void Return::accept(IRVisitor * v) { v->visit(this); }
    inline void BorderCall::accept(IRVisitor * v) { v->visit(this); }
    inline void StoreParam::accept(IRVisitor * v) { v->visit(this); }
    inline void CallStatic::accept(IRVisitor * v) { v->visit(this); }
    inline void Call::accept(IRVisitor * v) { v->visit(this); }
    inline void Fun_address::accept(IRVisitor * v) { v->visit(this); }
    inline void BinaryOp::accept(IRVisitor * v) {  }
    inline void UnaryOp::accept(IRVisitor * v) {  }
    inline void Mul::accept(IRVisitor * v) { v->visit(this); }
    inline void Div::accept(IRVisitor * v) { v->visit(this); }
    inline void Mod::accept(IRVisitor * v) { v->visit(this); }
    inline void Add::accept(IRVisitor * v) { v->visit(this); }
    inline void Sub::accept(IRVisitor * v) { v->visit(this); }
    inline void Or::accept(IRVisitor * v) { v->visit(this); }
    inline void And::accept(IRVisitor * v) { v->visit(this); }
    inline void ShR::accept(IRVisitor * v) { v->visit(this); }
    inline void ShL::accept(IRVisitor * v) { v->visit(this); }
    inline void Gte::accept(IRVisitor * v) { v->visit(this); }
    inline void Gt::accept(IRVisitor * v) { v->visit(this); }
    inline void Lte::accept(IRVisitor * v) { v->visit(this); }
    inline void Lt::accept(IRVisitor * v) { v->visit(this); }
    inline void Eq::accept(IRVisitor * v) { v->visit(this); }
    inline void NEq::accept(IRVisitor * v) { v->visit(this); }
    inline void BitOr::accept(IRVisitor * v) { v->visit(this); }
    inline void BitAnd::accept(IRVisitor * v) { v->visit(this); }
    inline void Plus::accept(IRVisitor * v) { v->visit(this); }
    inline void Minus::accept(IRVisitor * v) { v->visit(this); }
    inline void Not::accept(IRVisitor * v) { v->visit(this); }
    inline void Neg::accept(IRVisitor * v) { v->visit(this); }
    inline void Inc::accept(IRVisitor * v) { v->visit(this); }
    inline void Dec::accept(IRVisitor * v) { v->visit(this); }
    inline void Castctoi::accept(IRVisitor * v) { v->visit(this); }
    inline void Castctod::accept(IRVisitor * v) { v->visit(this); }
    inline void Castitod::accept(IRVisitor * v) { v->visit(this); }
    inline void Castdtoi::accept(IRVisitor * v) { v->visit(this); }
    inline void DebugWrite::accept(IRVisitor * v) { v->visit(this); }
    inline void NOP::accept(IRVisitor * v) {v->visit(this); }
}