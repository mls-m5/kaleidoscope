#pragma once

#include "ast.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

inline std::unique_ptr<llvm::LLVMContext> TheContext;
inline std::unique_ptr<llvm::Module> TheModule;
inline std::unique_ptr<llvm::IRBuilder<>> Builder;
// inline llvm::ExitOnError ExitOnErr;

inline std::map<std::string, llvm::AllocaInst *> NamedValues;
// std::unique_ptr<KaleidoscopeJIT> TheJIT;
inline std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
