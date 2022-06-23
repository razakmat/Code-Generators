#pragma once

#include <vector>
#include <cstdint>

namespace tiny::t86 {

    class Instruction;

    /**
     * Simple wrapper for vector of instructions
     */
    class Program {
    public:
        Program(std::vector<Instruction*> instructions = {}, std::vector<int64_t> data = {})
            : instructions_(std::move(instructions)), data_(std::move(data)) {}

        Program(Program&& other)
            : instructions_(std::move(other.instructions_)), data_(std::move(other.data_)){}

        ~Program() {
            deleteInstructions();
        }

        Program& operator=(Program&& other) {
            deleteInstructions();
            instructions_ = std::move(other.instructions_);
            data_ = std::move(other.data_);
            return *this;
        }

        const Instruction* at(size_t index) const;

        const std::vector<int64_t>& data() const {
            return data_;
        }

        std::vector<Instruction*> moveInstructions() {
            return std::move(instructions_);
        }

    private:
        void deleteInstructions();

        std::vector<Instruction*> instructions_;

        std::vector<int64_t> data_;
    };
}
