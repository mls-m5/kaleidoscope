#pragma once

#include "ast.h"
#include <llvm/IR/Value.h>
#include <memory>
#include <string>

/// LogError* - These are little helper functions for error handling.
inline std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

inline std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

// --------------- Code Generation ---------------------

inline llvm::Value *LogErrorV(const char *Str) {
    LogError(Str);
    return nullptr;
}
