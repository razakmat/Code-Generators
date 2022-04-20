#include "frontend.h"
#include "ASTtoIR.h"
#include "ast.h"

namespace tinyc {

    Frontend::Frontend() {
    }

    void Frontend::typecheck(AST * ast) {
        NOT_IMPLEMENTED;
        Typechecker ty;
        ty.visitChild(ast);
        
    }

    void Frontend::astotir(AST * ast) {
        ASTtoIR pokus;
        pokus.visitChild(ast);
    }

} // namespace tiny

