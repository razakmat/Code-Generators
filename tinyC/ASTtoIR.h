#pragma once

#include "ast.h"
#include "common/types.h"
#include "IR.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace tinyc {

    class ASTtoIR : public ASTVisitor {
        public:
            ASTtoIR();
            void visit(ASTInteger * ast);
            void visit(ASTDouble * ast);
            void visit(ASTChar * ast);
            void visit(ASTString * ast);
            void visit(ASTIdentifier * ast);
            void visit(ASTArrayValue * ast);
            void visit(ASTPointerType * ast);
            void visit(ASTArrayType * ast);
            void visit(ASTNamedType * ast);
            void visit(ASTSequence * ast);
            void visit(ASTBlock * ast);
            void visit(ASTVarDecl * ast);
            void visit(ASTFunDecl * ast);
            void visit(ASTStructDecl * ast);
            void visit(ASTFunPtrDecl * ast);
            void visit(ASTIf * ast);
            void visit(ASTSwitch * ast);
            void visit(ASTWhile * ast);
            void visit(ASTDoWhile * ast);
            void visit(ASTFor * ast);
            void visit(ASTBreak * ast);
            void visit(ASTContinue * ast);
            void visit(ASTReturn * ast);
            void visit(ASTBinaryOp * ast);
            void visit(ASTAssignment * ast);
            void visit(ASTUnaryOp * ast);
            void visit(ASTUnaryPostOp * ast);
            void visit(ASTAddress * ast);
            void visit(ASTDeref * ast);
            void visit(ASTIndex * ast);
            void visit(ASTMember * ast);
            void visit(ASTMemberPtr * ast);
            void visit(ASTCall * ast);
            void visit(ASTCast * ast);
            void visit(ASTRead * ast);
            void visit(ASTWrite * ast);

            void visitChild(AST * ast);
            template<typename T>
            void visitChild(std::unique_ptr<T> const & ptr);
        protected:
            ResultType GetType(Type * t);
            void NewBlock();
            std::shared_ptr<Instruction> AssignCast(ResultType left, ResultType right, std::shared_ptr<Instruction> & ins);
            void AddJumpsContinueBreak();
            void RemJumpsContinueBreak(std::shared_ptr<Block> & c, std::shared_ptr<Block> & b);
            void EnterEnv();
            void LeaveEnv();
            struct Env {
                std::shared_ptr<Instruction> FindVar(Symbol name);
                void AddVar(Symbol name, std::shared_ptr<Instruction> ins);
                std::unordered_map<Symbol, std::shared_ptr<Instruction>> m_map;
                std::shared_ptr<Env> m_prev = nullptr;
            };
            std::unordered_map<Symbol,std::shared_ptr<Function>> m_funs;
            std::shared_ptr<Env> m_env;
            std::shared_ptr<Function> m_fun = nullptr;
            std::shared_ptr<Block> m_block = nullptr;
            std::shared_ptr<Instruction> m_last;
            std::vector<std::shared_ptr<Instruction>> m_allocs_g;
            bool m_leftValue = false;
            std::vector<std::shared_ptr<Jump>> m_continue;
            std::vector<std::shared_ptr<Jump>> m_break;

    };
}