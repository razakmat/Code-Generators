#pragma once

#include "../program.h"
#include "../instruction.h"
#include "label.h"

#include <cassert>
#include <vector>

namespace tiny::t86 {
    class ProgramBuilder {
    public:
        ProgramBuilder(bool release = false) : release_(release) {}

        ProgramBuilder(Program program, bool release = false)
                : instructions_(program.moveInstructions()),
                  data_(program.data()),
                  release_(release) {}

        template<typename T>
        Label add(const T& instruction) {
            instruction.validate();
            instructions_.push_back(new T(instruction));
            return Label(instructions_.size() - 1);
        }

        Label add(const DBG& instruction) {
            if (!release_) {
                instructions_.push_back(new DBG(instruction));
                return instructions_.size() - 1;
            }
            // This returns the next added instruction
            // if the DBG was last, you will get into a infinite NOP loop
            // but you would get there with DBG as last anyways
            return instructions_.size();
        }

        DataLabel addData(int64_t data) {
            data_.push_back(data);
            return data_.size() - 1;
        }

        DataLabel addData(const std::string& str) {
            DataLabel label = data_.size();
            for (char c : str) {
                addData(c);
            }
            addData('\0');
            return label;
        }

        void patch(Label instruction, Label destination) {
            Instruction* ins = instructions_.at(instruction);
            auto* jmpInstruction = dynamic_cast<PatchableJumpInstruction*>(ins);
            assert(jmpInstruction && "You can patch only jump instructions");
            jmpInstruction->setDestination(destination);
        }

        Program program() {
            return Program(std::move(instructions_), std::move(data_));
        }

    private:
        std::vector<Instruction*> instructions_;

        std::vector<int64_t> data_;

        bool release_;
    };
}
