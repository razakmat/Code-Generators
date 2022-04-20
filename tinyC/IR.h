#pragma once

#include <vector>
#include <memory>
#include "ast.h"
#include "common/types.h"

namespace tinyc {
    
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
        protected:
            virtual void accept(IRVisitor * v) = 0;
    };

    class Block {
        public:
            Block();
            std::vector<std::shared_ptr<Instruction>> m_block;
            int m_name;
            inline static int counter;
    };

    class Function {
        public:
            Function(Symbol name);
            Symbol m_name;
            std::vector<std::shared_ptr<Block>> m_blocks;
            std::vector<std::shared_ptr<Instruction>> m_args;
            std::vector<std::shared_ptr<Instruction>> m_allocs;
    };

    class Fun_address : public Instruction {
        public:
            Fun_address(Symbol name);
            Symbol m_name;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Call : public Instruction {
        public:
            std::shared_ptr<Instruction> m_fun_addr;
            std::vector<std::shared_ptr<Instruction>> m_args;
        protected:
            void accept(IRVisitor * v) override;
    };

    class CallStatic : public Instruction {
        public:
            std::shared_ptr<Instruction> m_fun_addr;
            std::vector<std::shared_ptr<Instruction>> m_args;
        protected:
            void accept(IRVisitor * v) override;
    };

    class LoadFun : public Instruction {
        public:
            std::shared_ptr<Instruction> m_address;
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
            std::shared_ptr<Instruction> m_address;
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
            std::shared_ptr<Instruction> m_address;
            std::shared_ptr<Instruction> m_value;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Return : public Instruction {
        public:
            std::shared_ptr<Instruction> m_res;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Jump_cond : public Instruction {
        public:
            std::shared_ptr<Instruction> m_cond;
            std::shared_ptr<Block> m_true;
            std::shared_ptr<Block> m_false;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Jump : public Instruction {
        public:
            std::shared_ptr<Block> m_label;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Cond_switch : public Instruction {
        public:
            std::shared_ptr<Instruction> m_cond;
            std::unordered_map<int,std::shared_ptr<Block>> m_cases;
            std::shared_ptr<Block> m_default;
        protected:
            void accept(IRVisitor * v) override;
    };


    class BinaryOp : public Instruction {
        public:
            BinaryOp(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
            std::shared_ptr<Instruction> m_left;
            std::shared_ptr<Instruction> m_right;
        protected:
            virtual void accept(IRVisitor * v) = 0;
    };

    class UnaryOp : public Instruction {
        public:
            std::shared_ptr<Instruction> m_left;
        protected:
            virtual void accept(IRVisitor * v) = 0;
    };



    class Mul : public BinaryOp {
        public:
            Mul(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Div : public BinaryOp {
        public:
            Div(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Mod : public BinaryOp {
        public:
            Mod(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Add : public BinaryOp {
        public:
            Add(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Sub : public BinaryOp {
        public:
            Sub(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class ShL : public BinaryOp {
        public:
            ShL(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class ShR : public BinaryOp {
        public:
            ShR(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Gt : public BinaryOp {
        public:
            Gt(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Gte : public BinaryOp {
        public:
            Gte(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Lt : public BinaryOp {
        public:
            Lt(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Lte : public BinaryOp {
        public:
            Lte(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Eq : public BinaryOp {
        public:
            Eq(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class NEq : public BinaryOp {
        public:
            NEq(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class BitAnd : public BinaryOp {
        public:
            BitAnd(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class BitOr : public BinaryOp {
        public:
            BitOr(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class And : public BinaryOp {
        public:
            And(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Or : public BinaryOp {
        public:
            Or(std::shared_ptr<Instruction> & left, std::shared_ptr<Instruction> & right, ResultType t);
        protected:
            void accept(IRVisitor * v) override;
    };

    class Plus : public UnaryOp {
        public:
        protected:
            void accept(IRVisitor * v) override;
    };

    class Minus : public UnaryOp {
        public:
        protected:
            void accept(IRVisitor * v) override;
    };

    class Not : public UnaryOp {
        public:
        protected:
            void accept(IRVisitor * v) override;
    };

    class Inc : public UnaryOp {
        public:
        protected:
            void accept(IRVisitor * v) override;
    };

    class Dec : public UnaryOp {
        public:
        protected:
            void accept(IRVisitor * v) override;
    };

    class Castctoi : public Instruction {
        public:
            Castctoi(std::shared_ptr<Instruction> & val);
            std::shared_ptr<Instruction> m_val;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Castctod : public Instruction {
        public:
            Castctod(std::shared_ptr<Instruction> & val);
            std::shared_ptr<Instruction> m_val;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Castitod : public Instruction {
        public:
            Castitod(std::shared_ptr<Instruction> & val);
            std::shared_ptr<Instruction> m_val;
        protected:
            void accept(IRVisitor * v) override;
    };

    class Castdtoi : public Instruction {
        public:
            Castdtoi(std::shared_ptr<Instruction> & val);
            std::shared_ptr<Instruction> m_val;
        protected:
            void accept(IRVisitor * v) override;
    };


    class IRVisitor {
        public:
            virtual void visit(Instruction * ir) = 0;
            virtual void visit(Fun_address * ir) = 0;
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
            virtual void visit(Return * ir) = 0;
            virtual void visit(Jump_cond * ir) = 0;
            virtual void visit(Jump * ir) = 0;
            virtual void visit(Cond_switch * ir) = 0;
            virtual void visit(BinaryOp * ir) = 0;
            virtual void visit(UnaryOp * ir) = 0;
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
            virtual void visit(Inc * ir) = 0;
            virtual void visit(Dec * ir) = 0;
            virtual void visit(Castctoi * ir) = 0;
            virtual void visit(Castctod * ir) = 0;
            virtual void visit(Castitod * ir) = 0;
            virtual void visit(Castdtoi * ir) = 0;
    };


    inline void Load_Imm_d::accept(IRVisitor * v) { v->visit(this); }
    inline void Load_Imm_i::accept(IRVisitor * v) { v->visit(this); }
    inline void Load_Imm_c::accept(IRVisitor * v) { v->visit(this); }
    inline void Load::accept(IRVisitor * v) { v->visit(this); }
    inline void Alloc_g::accept(IRVisitor * v) { v->visit(this); }
    inline void Alloc_l::accept(IRVisitor * v) { v->visit(this); }
    inline void Store::accept(IRVisitor * v) { v->visit(this); }
    inline void Alloc_arg::accept(IRVisitor * v) { v->visit(this); }
    inline void Jump_cond::accept(IRVisitor * v) { v->visit(this); }
    inline void Jump::accept(IRVisitor * v) { v->visit(this); }
    inline void Cond_switch::accept(IRVisitor * v) { v->visit(this); }
    inline void Return::accept(IRVisitor * v) { v->visit(this); }
    inline void CallStatic::accept(IRVisitor * v) { v->visit(this); }
    inline void Call::accept(IRVisitor * v) { v->visit(this); }
    inline void Fun_address::accept(IRVisitor * v) { v->visit(this); }
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
    inline void Castctoi::accept(IRVisitor * v) { v->visit(this); }
    inline void Castctod::accept(IRVisitor * v) { v->visit(this); }
    inline void Castitod::accept(IRVisitor * v) { v->visit(this); }
    inline void Castdtoi::accept(IRVisitor * v) { v->visit(this); }
}