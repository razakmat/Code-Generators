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
            ~ASTtoIR();
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
            IRProgram * getIR();
        protected:
            ResultType GetType(Type * t);
            void NewBlock();
            Instruction * AssignCast(ResultType left, ResultType right, Instruction * ins);
            void ConditionAdjust();
            void SwitchCond(int val);
            void AddJumpsContinueBreak();
            void RemJumpsContinueBreak(Block * c, Block * b);
            void EnterEnv();
            void LeaveEnv();
            struct Env {
                Instruction * FindVar(Symbol name);
                void AddVar(Symbol name, Instruction * ins);
                std::unordered_map<Symbol, Instruction*> m_map;
                Env * m_prev = nullptr;
            };
            Env * m_env;
            Function * m_fun = nullptr;
            Block * m_block = nullptr;
            Instruction * m_last;
            bool m_leftValue = false;
            std::vector<Jump*> m_continue;
            std::vector<Jump*> m_break;
            IRProgram * m_prg = nullptr;
    };
}