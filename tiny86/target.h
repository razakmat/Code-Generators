#pragma once

#include <iostream>

#include "utils/stats_logger.h"
#include "program.h"
#include "cpu.h"
#include "common/config.h"

namespace tiny {

    class Target {
    public:

        using EXE = t86::Program;

        /** Runs the given executable.

            The signature is fixed, so the CPU has to take the program as a const ref.
         */
        void execute(EXE exe) {
            t86::StatsLogger::instance().reset();

            t86::Cpu cpu;
            cpu.start(std::move(exe));

            while (!cpu.halted()) {
                cpu.tick();
            }
            t86::StatsLogger::instance().processBasicStats(std::cerr);
        }

    }; // Target
}