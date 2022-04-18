#pragma once

#include <string>
#include <cassert>
#include <unordered_map>

#include "symbol.h"
#include "helpers.h"
#include "ast.h"

namespace tiny {

    /** Base class for all types in tinyverse. 
     */
    class Type {
    public:

        class POD;
        class Pointer;
        class Array;
        class Struct;
        class Alias;
        class Function;

        virtual ~Type() = default;

        /** Converts the type to a printable string.
         */
        std::string toString() const {
            std::stringstream ss;
            toStream(ss);
            return ss.str();
        }

        virtual size_t size() const = 0;

        virtual bool convertsToBoolean() const { return false; }

        /** Returns whether the type is fully defined. 
         */
        virtual bool isFullyDefined() const { return true; }

        static Type::POD * voidType() {
            return voidType_;
        }

        static Type::POD * intType() {
            return intType_;
        }

        static Type::POD * charType() {
            return charType_;            
        }

        static Type::POD * doubleType() {
            return doubleType_;            
        }

        static Type::Struct * getOrCreateStructType(Symbol name);
        static Type::Pointer * getOrCreatePointerType(Type * name);
        static Type::Array * getOrCreateArrayType(Type * name);
        static Type * getType(Symbol name);
        static Type::Alias * CreateAliasType(Symbol name, Type * t);
        static Type::Function * getOrCreateFunctionType(Symbol name, Type * ret);

        static bool isPOD(Type * t);
        static bool isPointer(Type * t);
        static bool isFunction(Type * t);
        static Type::POD * BinaryResult(Type * right, Type * left);

        static void EntryBlockFrame();
        static void EntryFunctionFrame(Type * ret);
        static void LeaveFrame();
        static Type * GetVarType(const std::string & name);
        static bool SetVarType(const std::string & name,Type * t);
        static Type * GetCurrentReturnType();
    private:
        struct frame{
            std::unordered_map<std::string,Type*> m_varMap;
            bool m_endOfScope;
            Type * returnType  = nullptr;
        };
        static std::vector<frame> m_frames;

        virtual void toStream(std::ostream & s) const = 0;

        static std::unordered_map<std::string, Type *> initialize_types();

        static std::unordered_map<std::string, Type*> & types();

        static POD * voidType_;
        static POD * intType_;
        static POD * charType_;
        static POD * doubleType_;

    }; // tiny::Type

    class Type::POD : public Type {
    public:
        POD(Symbol name): 
            name_{name} {
            assert(name_ == Symbol::KwInt || name_ == Symbol::KwChar || name_ == Symbol::KwDouble);
        }

        Symbol name() const { return name_; }

        size_t size() const override {
            if (name_ == Symbol::KwChar) {
                return 1;
            } else if (name_ == Symbol::KwDouble) {
                return 8;
            } else {
                return 8;
            } 
        }

    private:

        void toStream(std::ostream & s) const override {
            s << name_;
        }

        Symbol name_;
    };

    class Type::Pointer : public Type {
    public:

        Pointer(Type * base):
            base_{base} {
        }

        size_t size() const override { return 8; }
        Type * base() const { return base_;}
    private:
        void toStream(std::ostream & s) const override {
            s << "*";
            base_->toStream(s);
        }

        Type * base_;
    }; 

    class Type::Array : public Type {
    public:

        Array(Type * base):
            base_{base} {
        }

        size_t size() const override { return 8; }
        Type * base() const { return base_;}
    private:
        void toStream(std::ostream & s) const override {
            s << "[";
            base_->toStream(s);
            s << "]";
        }

        Type * base_;
    }; 

    class Type::Struct : public Type {
    public:
        Struct(Symbol name):
            name_{name} {
        }

        size_t size() const override { 
            NOT_IMPLEMENTED;
        }

        bool isFullyDefined() const override { return defined_; }

        void markFullyDefined() { defined_ = true; }

        void addField(Symbol name, Type * type, ASTBase * ast);
        Type * getFieldType(Symbol name);

    private:

        void toStream(std::ostream & s) const override {
            s << name_;
        }

        Symbol name_;
        bool defined_ = false;
        std::vector<std::pair<Symbol, Type*>> fields_;
    };

    class Type::Alias : public Type {
    public:
        Alias(Symbol name, Type * base):
            name_{name}, base_{base} {
        }

        size_t size() const override { 
            NOT_IMPLEMENTED;
        }

        bool isFullyDefined() const override { return base_->isFullyDefined(); }

        Type * base() const { return base_;}

    private:

        void toStream(std::ostream & s) const override {
            s << name_;
        }

        Symbol name_;
        Type * base_;
    }; 



    class Type::Function : public Type {
    public:
        Function(Symbol name, Type * returnType):
            name_{name}, returnType_{returnType} {
        }

        size_t size() const override { 
            NOT_IMPLEMENTED;
        }

        bool isFullyDefined() const override { return defined_; }

        void markFullyDefined() { defined_ = true; }

        void addArg(Type * type, ASTBase * ast);

        int sizeArgs();
        Type * argType(int i);
        Type * returnType();

    private:

        void toStream(std::ostream & s) const override {
            // TODO:
        }

        Symbol name_;
        bool defined_ = false;
        std::vector<Type*> m_args;
        Type * returnType_;
    };

} // namespace tiny