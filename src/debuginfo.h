#pragma once
#include "ast.h"
#include "globals.h"
#include <llvm/IR/DIBuilder.h>

//===----------------------------------------------------------------------===//
// Debug Info Support
//===----------------------------------------------------------------------===//

struct DebugInfo {
    llvm::DICompileUnit *TheCU;
    llvm::DIType *DblTy;
    std::vector<llvm::DIScope *> LexicalBlocks;

    void emitLocation(ExprAST *AST);
    llvm::DIType *getDoubleTy();
};

inline DebugInfo KSDbgInfo;

inline std::unique_ptr<llvm::DIBuilder> DBuilder;

inline llvm::DIType *DebugInfo::getDoubleTy() {
    if (DblTy)
        return DblTy;

    DblTy = DBuilder->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
    return DblTy;
}

inline void DebugInfo::emitLocation(ExprAST *AST) {
    if (!AST)
        return Builder->SetCurrentDebugLocation(llvm::DebugLoc());
    llvm::DIScope *Scope;
    if (LexicalBlocks.empty())
        Scope = TheCU;
    else
        Scope = LexicalBlocks.back();
    Builder->SetCurrentDebugLocation(llvm::DILocation::get(
        Scope->getContext(), AST->getLine(), AST->getCol(), Scope));
}

inline llvm::DISubroutineType *CreateFunctionType(unsigned NumArgs,
                                                  llvm::DIFile *Unit) {
    llvm::SmallVector<llvm::Metadata *, 8> EltTys;
    llvm::DIType *DblTy = KSDbgInfo.getDoubleTy();

    // Add the result type.
    EltTys.push_back(DblTy);

    for (unsigned i = 0, e = NumArgs; i != e; ++i)
        EltTys.push_back(DblTy);

    return DBuilder->createSubroutineType(
        DBuilder->getOrCreateTypeArray(EltTys));
}
