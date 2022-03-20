#pragma once

#include "ast.h"
#include "common/types.h"

namespace tinyc {

    using Type = tiny::Type;

    class Typechecker : public ASTVisitor  {


        void visit(ASTInteger * ast) override {
            ast->setType(Type::intType());
        }

        // Not, Inc, Dec, +, -, 
        void visit(ASTUnaryOp * ast) override {
            Type * t = visitChild(ast->arg);
            if (ast->op == Symbol::Add || ast->op == Symbol::Sub) {
                // TODO overim si jestli to je ok typ a kdyz jo tak:
                ast->setType(t);
            } else if (ast->op == Symbol::Not) {
                if (t->convertsToBoolean())
                    ast->setType(Type::intType());
            }
            

        }

        void visit(ASTIf * ast) override { 
            if (! visitChild(ast->cond)->convertsToBoolean())
                throw ParserError{STR("Condition must convert to bool, but " << ast->cond->type()->toString() << " found"), ast->cond->location()};
            visitChild(ast->trueCase);
            if (ast->falseCase != nullptr) {
                visitChild(ast->falseCase);
            }
            ast->setType(Type::voidType());
        }

        void visit(ASTStructDecl * ast) override {
            Type::Struct * t = Type::getOrCreateStructType(ast->name);
            if (t == nullptr)
                throw ParserError{STR("Type " << ast->name << " already defined and not a struct"), ast->location()};
            if (ast->isDefinition) {
                if (t->isFullyDefined())
                    throw ParserError{STR("Struct " << ast->name << " already fully defined"), ast->location()};
                for (auto & i : ast->fields) {
                    Type * fieldType = visitChild(i.second);
                    t->addField(i.first->name, fieldType, i.second.get());
                }
            }
            t->markFullyDefined();
            ast->setType(t);
        }

        Type * visitChild(AST * ast) {
            ASTVisitor::visitChild(ast);
            return ast->type();
        }

        template<typename T>
        Type * visitChild(std::unique_ptr<T> const & ptr) {
            return visitChild(ptr.get());
        }

    }; // Typechecker
}