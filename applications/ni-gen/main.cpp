#include <cstdlib>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

#include "tinyC/ast.h"
#include "tinyC/frontend.h"
#include "tinyC/IR.h"
#include "tinyC/backend.h"
#include "tinyC/Optimalization.h"

#include "common/config.h"

#include "tiny86/cpu.h"
#include "tiny86/program.h"
#include "tiny86/program/programbuilder.h"
#include "tiny86/program/helpers.h"

using namespace tiny::t86;
using namespace tiny;
using namespace tinyc;
using namespace colors;

namespace tiny {
    Config config;
}

int main(int argc, char * argv[]) {

    config.parse(argc,argv);
    tiny::t86::Cpu::Config::instance();

    config.setDefaultIfMissing("-o","0");

    std::ifstream file(config.input());
    std::string str((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());

    std::cout << color::gray << "NI-GEN 2022" << color::reset << std::endl;
    initializeTerminal();
    try {
        Frontend front;
        std::unique_ptr<AST> ast = front.parse(str);
        std::cout << "AST:" << std::endl;
        std::cout << ColorPrinter::colorize(*ast) << std::endl;

        front.typecheck(ast);
        // typecheck
        IRProgram * IRprg = nullptr;
        IRprg = front.astotir(ast);

        Optimalization optim(IRprg);


        if (config.get("-o") == "1")
            optim.StartAll();

        Backend back(IRprg,
                    std::stoul(config.get(tiny::t86::Cpu::Config::registerCountConfigString)),
                    std::stoul(config.get(tiny::t86::Cpu::Config::floatRegisterCountConfigString)));

        back.irtot86();

        back.Start();

        if (IRprg != nullptr)
            delete IRprg;


    } catch (std::exception const & e) {
        std::cerr << color::red << "ERROR: " << color::reset << e.what() << std::endl;
    } catch (...) {
        std::cerr << color::red << "UNKNOWN ERROR. " << color::reset << std::endl;
    }
    return EXIT_SUCCESS;
}