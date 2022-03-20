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
        class Struct;
        class FunPtr;
        class Alias;

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
            NOT_IMPLEMENTED;            
        }

        static Type::POD * doubleType() {
            NOT_IMPLEMENTED;            
        }

        static Type::Struct * getOrCreateStructType(Symbol name);

    private:
        virtual void toStream(std::ostream & s) const = 0;

        static std::unordered_map<std::string, Type *> initialize_types();

        static std::unordered_map<std::string, Type*> & types();

        static POD * voidType_;
        static POD * intType_;

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

    private:
        void toStream(std::ostream & s) const override {
            s << "*";
            base_->toStream(s);
        }

        Type const * base_;
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

    private:

        void toStream(std::ostream & s) const override {
            s << name_;
        }

        Symbol name_;
        bool defined_ = false;
        std::vector<std::pair<Symbol, Type*>> fields_;
    }; 

} // namespace tiny