#include "frontend.h"
#include "ASTtoIR.h"
#include "ast.h"
#include "IR.h"

namespace tinyc {

    Frontend::Frontend() {
    }

    void Frontend::typecheck(AST * ast) {
        Typechecker ty;
        ty.visitChild(ast);
        
    }

    IRProgram * Frontend::astotir(AST * ast) {
        ASTtoIR pokus;
        pokus.visitChild(ast);
        return pokus.getIR();
    }

} // namespace tiny

