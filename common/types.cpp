#include "types.h"

namespace tiny {

    Type::Struct * Type::getOrCreateStructType(Symbol name) {
        auto t = types();
        auto i = t.find(name.name());
        if (i == t.end())
            i = t.insert(std::make_pair(name.name(), new Type::Struct{name})).first;
        return dynamic_cast<Struct*>(i->second);
    }

    Type::Pointer * Type::getOrCreatePointerType(Type * name) {
        std::string typeName = "*" + name->toString();
        auto t = types();
        auto i = t.find(typeName);
        if (i == t.end())
            i = t.insert(std::make_pair(typeName, new Type::Pointer{name})).first;
        return dynamic_cast<Pointer*>(i->second);
    }

    Type::Array * Type::getOrCreateArrayType(Type * name) {
        std::string typeName = "[" + name->toString() + "]";
        auto t = types();
        auto i = t.find(typeName);
        if (i == t.end())
            i = t.insert(std::make_pair(typeName, new Type::Array{name})).first;
        return dynamic_cast<Array*>(i->second);
    }
    
    Type * Type::getType(Symbol name) 
    {
        auto t = types();
        auto i = t.find(name.name());
        if (i == t.end())
            return nullptr;
        if (Type::Alias * a = dynamic_cast<Type::Alias*>(i->second))
        {
            return a->base();
        }
        return i->second;
    }
    
    Type::Alias * Type::CreateAliasType(Symbol name, Type * t) 
    {
        Type::Alias * a = new Type::Alias(name,t);
        auto it = types();
        auto i = it.insert(std::make_pair(name.name(),a));
        if (!i.second)
            return nullptr;
        return a;
    }
    
    Type::Function * Type::getOrCreateFunctionType(Symbol name , Type * ret) 
    {
        auto t = types();
        auto i = t.find(name.name());
        if (i == t.end())
            i = t.insert(std::make_pair(name.name(), new Type::Function{name,ret})).first;
        return dynamic_cast<Function*>(i->second);
    }
    
    bool Type::isPOD(Type * t)
    {
        Type::POD * test = dynamic_cast<Type::POD*>(t);
        if (test != nullptr)
            return true;
        return false;
    }
    bool Type::isPointer(Type * t)
    {
        Type::Pointer * test = dynamic_cast<Type::Pointer*>(t);
        if (test != nullptr)
            return true;
        return false;
    }
    bool Type::isFunction(Type * t)
    {
        Type::Function * test = dynamic_cast<Type::Function*>(t);
        if (test != nullptr)
            return true;
        return false;
    }

    Type::POD * Type::BinaryResult(Type * right, Type * left)
    {
        if (right == doubleType() && isPOD(left))
            return doubleType();
        else if (left == doubleType() && isPOD(right))
            return doubleType();
        else if (right == charType() && left == charType())
            return charType();
        else if (isPOD(left) && isPOD(right))
            return intType();
        return nullptr;
    }


    void Type::EntryBlockFrame() 
    {
        frame now;
        now.m_endOfScope = false;
        now.returnType = m_frames.back().returnType;
        m_frames.push_back(std::move(now));
    }
    
    void Type::EntryFunctionFrame(Type * ret) 
    {
        frame now;
        now.m_endOfScope = true;
        now.returnType = ret;
        m_frames.push_back(std::move(now));
    }
    
    void Type::LeaveFrame() 
    {
        m_frames.pop_back();
    }
    
    Type * Type::GetVarType(const std::string & name) 
    {
        for (auto x = m_frames.rbegin(); x != m_frames.rend(); x++)
        {
            auto it = x->m_varMap.find(name);
            if (it != x->m_varMap.end())
                return it->second;
            if (x->m_endOfScope)
                break;
        }
        auto it = m_frames.front().m_varMap.find(name);
        if (it != m_frames.front().m_varMap.end())
            return it->second;
        return nullptr;
    }
    
    bool Type::SetVarType(const std::string & name,Type * t) 
    {
        auto it = m_frames.back().m_varMap.insert({name,t});
        return it.second;
    }
    
    Type * Type::GetCurrentReturnType() 
    {
        return m_frames.back().returnType;
    }

    std::unordered_map<std::string, Type *> Type::initialize_types() {
        std::unordered_map<std::string, Type *> result;
        voidType_ = new POD{Symbol::KwVoid};
        result.insert(std::make_pair("void", voidType_));
        intType_ = new POD{Symbol::KwInt};
        result.insert(std::make_pair("int", intType_));
        charType_ = new POD{Symbol::KwChar};
        result.insert(std::make_pair("char", charType_));
        doubleType_ = new POD{Symbol::KwDouble};
        result.insert(std::make_pair("double", doubleType_));
        return result;
    }   

    std::unordered_map<std::string, Type*> & Type::types() {
        static std::unordered_map<std::string, Type*> types = initialize_types();
        return types;
    }

    void Type::Struct::addField(Symbol name, Type * type, ASTBase * ast) {
        if (! type->isFullyDefined()) 
            throw ParserError{STR("Field " << name.name() << " has not fully defined type " << type->toString()), ast->location()};
        for (auto & f : fields_) 
            if (f.first == name)
                throw ParserError{STR("Field " << name.name() << " already defined "), ast->location()};
        fields_.push_back(std::make_pair(name, type));
    }
    
    Type * Type::Struct::getFieldType(Symbol name) 
    {
        for (auto & x : fields_)
        {
            if (x.first == name)
                return x.second;
        }
        return nullptr;
    }
    
    void Type::Function::addArg(Type * type, ASTBase * ast) 
    {
        if (! type->isFullyDefined()) 
            throw ParserError{STR("Arg " << type->toString() << " is not fully defined type "), ast->location()};
        m_args.push_back(type);
    }

    int Type::Function::sizeArgs()
    {
        return m_args.size();
    }

    Type * Type::Function::argType(int i)
    {
        return m_args[i];
    }

    Type * Type::Function::returnType()
    {
        return returnType_;
    }
}