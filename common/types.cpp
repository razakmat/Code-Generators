#include "types.h"

namespace tiny {

    Type::Struct * Type::getOrCreateStructType(Symbol name) {
        auto t = types();
        auto i = t.find(name.name());
        if (i == t.end())
            i = t.insert(std::make_pair(name.name(), new Type::Struct{name})).first;
        return dynamic_cast<Struct*>(i->second);
    }

    std::unordered_map<std::string, Type *> Type::initialize_types() {
        std::unordered_map<std::string, Type *> result;
        voidType_ = new POD{Symbol::KwVoid};
        result.insert(std::make_pair("void", voidType_));
        intType_ = new POD{Symbol::KwInt};
        result.insert(std::make_pair("int", intType_));
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

}