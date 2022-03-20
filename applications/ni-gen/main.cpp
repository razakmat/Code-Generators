#include <cstdlib>

#include "tinyC/ast.h"
#include "tinyC/frontend.h"

using namespace tiny;
using namespace tinyc;
using namespace colors;

char const * tests[] = {
    "void main() {}", // 0
};

int main(int argc, char * argv[]) {
    std::cout << color::gray << "NI-GEN 2022" << color::reset << std::endl;
    initializeTerminal();
    try {
        Frontend front;
        std::unique_ptr<AST> ast = front.parse(tests[0]);
        std::cout << "AST:" << std::endl;
        std::cout << ColorPrinter::colorize(*ast) << std::endl;

        // typecheck

        // translate to IR

        // optimalize

        // translate to target

    } catch (std::exception const & e) {
        std::cerr << color::red << "ERROR: " << color::reset << e.what() << std::endl;
    } catch (...) {
        std::cerr << color::red << "UNKNOWN ERROR. " << color::reset << std::endl;
    }
    return EXIT_SUCCESS;
}