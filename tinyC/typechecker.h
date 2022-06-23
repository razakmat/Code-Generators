#pragma once

#include "ast.h"
#include "../common/types.h"

namespace tinyc {

    using Type = tiny::Type;

    class Typechecker : public ASTVisitor  {
        public:

        void visit(ASTInteger * ast) override {
            ast->setType(Type::intType());
        }

        void visit(ASTDouble * ast) override {
            ast->setType(Type::doubleType());
        }

        void visit(ASTChar * ast) override {
            ast->setType(Type::charType());
        }

        void visit(ASTString * ast) override {
            ast->setType(Type::getOrCreatePointerType(Type::charType()));
        }
        
        void visit(ASTIdentifier * ast) override {
            Type * t = Type::GetVarType(ast->name.name());
            if (t == nullptr)
                throw ParserError{STR("Variable " << ast->name.name() << " was not yet declared."),ast->location()};
            ast->setType(t);
        }

        void visit(ASTArrayValue * ast) override {
            Type * t = nullptr;
            for (auto & x : ast->values)
            {
                Type * t1 = visitChild(x);
                if (t != nullptr)
                {
                    if (t != t1)
                        throw ParserError(STR("Array elements has to be of the same type."),ast->location());
                    else
                        t = t1;
                }
            }
            ast->setType(Type::getOrCreateArrayType(t));
        }

        void visit(ASTPointerType * ast) override {
            ast->setType(Type::getOrCreatePointerType(visitChild(ast->base)));
        }
        void visit(ASTArrayType * ast) override {
            ast->setType(Type::getOrCreateArrayType(visitChild(ast->base)));
        }

        void visit(ASTNamedType * ast) override {
            Type * t = Type::getType(ast->name);
            if (t == nullptr)
                throw ParserError{STR("NamedType " << ast->name.name() << " was already used."),ast->location()};
            ast->setType(t);
        }




        void visit(ASTBinaryOp * ast) override {
            Type * left = visitChild(ast->left);
            Type * right = visitChild(ast->right);

            if (ast->op == Symbol::Mul || ast->op == Symbol::Div)
            {
                ast->setType(Type::BinaryResult(left,right));
            }
            else if (ast->op == Symbol::Mod)
            {
                if ((right == Type::intType() || right == Type::charType()) && 
                    (left == Type::intType() || left == Type::charType()))
                        ast->setType(right);
            }
            else if (ast->op == Symbol::Add || ast->op == Symbol::Sub)
            {
                ast->setType(Type::BinaryResult(left,right));
            }
            else if (ast->op == Symbol::ShiftLeft || ast->op == Symbol::ShiftRight)
            {
                if ((right == Type::intType() || right == Type::charType()) && 
                    (left == Type::intType() || left == Type::charType()))
                        ast->setType(left);
            }
            else if (ast->op == Symbol::Gte || ast->op == Symbol::Gt || 
                     ast->op == Symbol::Lt || ast->op == Symbol::Lte ||
                     ast->op == Symbol::Eq || ast->op == Symbol::NEq)
            {
                if (right == left && (Type::isPOD(right) || Type::isPointer(right)))
                    ast->setType(Type::intType());
            }
            else if (ast->op == Symbol::BitAnd || ast->op == Symbol::BitOr)
            {
                if ((right == left) && (right == Type::intType() || right == Type::charType()))
                    ast->setType(right);
            }
            else if (ast->op == Symbol::And || ast->op == Symbol::Or)
            {
                if (right->convertsToBoolean() && left->convertsToBoolean())
                    ast->setType(Type::intType());
            }
        }

        // Not, Inc, Dec, +, -, 
        void visit(ASTUnaryOp * ast) override {
            Type * t = visitChild(ast->arg);
            
            if (ast->op == Symbol::Add || ast->op == Symbol::Sub) {
                if (t == Type::intType() || t == Type::doubleType())
                    ast->setType(t);
                else
                    throw ParserError{STR("Type of + or - has to be int or double."),ast->location()};
                return;
            }
            if (ast->op == Symbol::Not || ast->op == Symbol::Neg) {
                if (t->convertsToBoolean())
                    ast->setType(Type::intType());
                else
                    throw ParserError{STR("Type has to be convertible to boolean."),ast->location()};
                return;
            }

            if (!ast->arg->hasAddress())
                throw ParserError{STR("Argument in unary operator ++ or -- has to have address."),ast->location()};
            
            if (ast->op == Symbol::Inc || ast->op == Symbol::Dec) {
                if (t == Type::intType() || t == Type::charType())
                    ast->setType(t);
                else
                    throw ParserError{STR("Type of Dec or Inc has to be int or char"),ast->location()};
            }
        }

        void visit(ASTUnaryPostOp * ast) override {
            Type * t = visitChild(ast->arg);
            if (!ast->arg->hasAddress())
                throw ParserError{STR("Argument in unary operator has to have address."),ast->location()};
            if (t == Type::intType() || t == Type::charType())
                ast->setType(t);
            else
                throw ParserError{STR("Unary post op has to be of type POD."),ast->location()};
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

        void visit(ASTSequence * ast) override {
            for (auto & x: ast->body){
                visitChild(x);
            }
            ast->setType(Type::voidType());
        }

        void visit(ASTBlock * ast) override {
            Type::EntryBlockFrame();
            Type * t = Type::voidType();
            for (auto & x: ast->body){
                Type * now = visitChild(x);
                if (dynamic_cast<ASTReturn*>(x.get()))
                    t = now;
            }
            Type::LeaveFrame();
            ast->setType(t);
        }

        void visit(ASTVarDecl * ast) override {
            Type * t = visitChild(ast->varType);
            if (!t->isFullyDefined())
                throw ParserError{STR("Type " << t->toString() << " is not yet fully defined."), ast->location()};
            if (ast->value != nullptr)
            {
                Type * t1 = visitChild(ast->value);
                if (t != t1)
                    throw ParserError{STR("Type " << t1->toString() << " is assigned to type." << t->toString()), ast->location()};
            }
            Type::SetVarType(ast->name->name.name(),t);
            visitChild(ast->name);
            ast->setType(t);
        }

        void visit(ASTFunDecl * ast) override {
            Type * ret = visitChild(ast->typeDecl);
            Type::Function * fun = new Type::Function(ret);
            for (auto & x : ast->args)
            {
                Type * t = visitChild(x.first);
                fun->addArg(t,x.second.get());  
            }

            fun = Type::getOrCreateFunctionType(fun);

            if ( Type::SetVarType(ast->name.name(),fun) == false)
                throw ParserError{STR("Name " << ast->name.name() << " is already used."), ast->location()};
            
            Type::EntryFunctionFrame(ret);
            for (auto & x : ast->args)
            {
                Type::SetVarType(x.second->name.name(),x.first->type());
            }
            Type * TypeBody = visitChild(ast->body);
            if (TypeBody != ret)
                throw ParserError{STR("Conflicting return type of function"), ast->location()};
            fun->markFullyDefined();
            Type::LeaveFrame();
            ast->setType(Type::voidType());
        }

        void visit(ASTFunPtrDecl * ast) override {
            Type * t = Type::getType(ast->name->name);
            if (t != nullptr)
                throw ParserError{STR("Type " << ast->name->name.name() << "was already declared."), ast->location()};
            Type * ret = visitChild(ast->returnType);

            Type::Function * fun = new Type::Function(ret);

            if (!ret->isFullyDefined())
                throw ParserError{STR("Return type " << ret->toString() << " is not yet fully defined."), ast->location()};

            for (int i = 0; i < ast->args.size(); i++)
            {
                t = visitChild(ast->args[i]);
                fun->addArg(t,ast);
            }

            fun = Type::getOrCreateFunctionType(fun);

            fun->markFullyDefined();
            Type::Pointer * p = Type::getOrCreatePointerType(fun);
            Type::Alias * a = Type::CreateAliasType(ast->name->name,p);
            ast->setType(a);
        }

        void visit(ASTSwitch * ast) override {
            if (! visitChild(ast->cond)->convertsToBoolean())
                throw ParserError{STR("Condition must convert to bool, but " << ast->cond->type()->toString() << " found"), ast->cond->location()};
            for (auto & x : ast->cases)
            {
                visitChild(x.second);
            }
            if (ast->defaultCase != nullptr)
                visitChild(ast->defaultCase);
            ast->setType(Type::voidType());
        }

        void visit(ASTWhile * ast) override {
            if (! visitChild(ast->cond)->convertsToBoolean())
                throw ParserError{STR("Condition must convert to bool, but " << ast->cond->type()->toString() << " found"), ast->cond->location()};
            visitChild(ast->body);
            ast->setType(Type::voidType());
        }

        void visit(ASTDoWhile * ast) override {
            if (! visitChild(ast->cond)->convertsToBoolean())
                throw ParserError{STR("Condition must convert to bool, but " << ast->cond->type()->toString() << " found"), ast->cond->location()};
            visitChild(ast->body);
            ast->setType(Type::voidType());
        }

        void visit(ASTFor * ast) override {
            if (ast->init != nullptr)
                visitChild(ast->init);
            if (ast->cond != nullptr)
                if (! visitChild(ast->cond)->convertsToBoolean())
                    throw ParserError{STR("Condition must convert to bool, but " << ast->cond->type()->toString() << " found"), ast->cond->location()};
            if (ast->increment != nullptr)
                visitChild(ast->increment);
            visitChild(ast->body);
            ast->setType(Type::voidType());
        }

        void visit(ASTBreak * ast) override {
            ast->setType(Type::voidType());
        }

        void visit(ASTContinue * ast) override {
            ast->setType(Type::voidType());
        }

        void visit(ASTReturn * ast) override {
            Type * t = Type::voidType();
            if (ast->value != nullptr)
                t = visitChild(ast->value);
            if (Type::GetCurrentReturnType() != t)
                throw ParserError{STR("Conflicting return type of function"), ast->location()};
            ast->setType(t);
        }

        void visit(ASTAssignment * ast) override {
            Type * left = visitChild(ast->lvalue);
            Type * right = visitChild(ast->value);
            if (!ast->lvalue->hasAddress())
                throw ParserError{STR("LValue of assignment has to have address."), ast->location()};
            if (left != right)
            {
                if (left == Type::intType() && right == Type::charType()){}
                else if (left == Type::intType() && right == Type::doubleType()){}
                else if (left == Type::doubleType() && right == Type::intType()){}
                else if (left == Type::doubleType() && right == Type::charType()){}
                else
                    throw ParserError{STR("Can assign only some different types"), ast->location()};
            }
            ast->setType(Type::voidType());
        }

        void visit(ASTAddress * ast) override {
            Type * t = visitChild(ast->target);
            if (!ast->target->hasAddress())
                throw ParserError{STR("Value does not have address."), ast->location()};
            ast->setType(Type::getOrCreatePointerType(t));
        }

        void visit(ASTDeref * ast) override {
            Type * t = visitChild(ast->target);
            Type::Pointer * point = dynamic_cast<Type::Pointer*>(t);
            if (t == nullptr)
                throw ParserError{STR("Cannot deref value that is not pointer"), ast->location()};
            t = point->base();
            ast->setType(t);
        }

        void visit(ASTIndex * ast) override {
            Type * t = visitChild(ast->base);
            Type::Array * arr = dynamic_cast<Type::Array*>(t);
            if (t == nullptr)
                throw ParserError{STR("Index can be used only with array type."), ast->location()};
            t = visitChild(ast->index);
            if (t != Type::intType())
                throw ParserError{STR("Index must be int."), ast->location()};
            ast->setType(arr->base());
        }
        
        void visit(ASTMember * ast) override {
            Type * t = visitChild(ast->base);
            Type::Struct * s = dynamic_cast<Type::Struct*>(t);
            if (t == nullptr || !s->isFullyDefined())
                throw ParserError{STR("Member can be used only with fully defined struct types."), ast->location()};
            t = s->getFieldType(ast->member);
            if (t == nullptr)
                throw ParserError{STR("Could not find member " << ast->member.name()), ast->location()};
            ast->setType(t);
        }

        void visit(ASTMemberPtr * ast) override {
            Type * t = visitChild(ast->base);
            Type::Pointer * p = dynamic_cast<Type::Pointer*>(t);
            if (p == nullptr)
                throw ParserError{STR("On left side of -> operator was not pointer."), ast->location()};
            Type::Struct * s = dynamic_cast<Type::Struct*>(p->base());
            if (s == nullptr)
                throw ParserError{STR("Only structs have members."), ast->location()};
            t = s->getFieldType(ast->member);
            if (t == nullptr)
                throw ParserError{STR("Member " << ast->member.name() << "was not found."), ast->location()};
            ast->setType(t);
        }

        void visit(ASTCall * ast) override {
            Type * t = visitChild(ast->function);
            Type::Function * p = Type::isFunPointer(t);
            if (p == nullptr)
                throw ParserError{STR("Call can be used only for type function."), ast->location()};
            if (p->sizeArgs() != ast->args.size())
                throw ParserError{STR("Number arguments does not match number of parameters."), ast->location()};
            for (int i = 0; i < ast->args.size(); i++)
            {
                t = visitChild(ast->args[i]);
                if (t != p->argType(i))
                    throw ParserError{STR("Types of arguments and parameters does not match."), ast->location()};
            }
            ast->setType(p->returnType());
        }

        void visit(ASTCast * ast) override {
            Type * value = visitChild(ast->value);
            Type * t = visitChild(ast->type);

            if (t == Type::intType())
            {
                if (dynamic_cast<Type::Pointer*>(value))
                    ast->setType(t);
                else if (dynamic_cast<Type::POD*>(value))
                    ast->setType(t);
            }
            if (Type::POD * p = dynamic_cast<Type::POD*>(t))
            {
                if (dynamic_cast<Type::POD*>(value))
                    ast->setType(t);
            }
            else if (Type::Pointer * p = dynamic_cast<Type::Pointer*>(t))
            {
                if (dynamic_cast<Type::Pointer*>(value))
                    ast->setType(t);
                if (value == Type::intType())
                    ast->setType(t);
            }
            AST * a = ast;
            t = a->type();
            if (t == nullptr)
                throw ParserError{STR("This type of cast is not allowed."), ast->location()};
        }

        void visit(ASTRead * ast) override {
            ast->setType(Type::voidType());
        }
        void visit(ASTWrite * ast) override {
            Type * value = visitChild(ast->value);
            ast->setType(value);
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