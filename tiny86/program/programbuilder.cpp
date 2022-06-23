#include "programbuilder.h"

namespace tiny::t86 {
    template<>
    Label ProgramBuilder::add(const DBG& instruction) {
        if (!release_) {
            instructions_.push_back(new DBG(instruction));
        }
        return Label(instructions_.size() - 1);
    }
}
