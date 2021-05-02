#include "parser.h"

#include "debuginfo.h"
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

using namespace llvm;

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

static void HandleDefinition() {
    if (auto FnAST = ParseDefinition()) {
        if (auto *FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read function definition:");
            FnIR->print(errs());
            fprintf(stderr, "\n");
        }
    }
    else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (auto ProtoAST = ParseExtern()) {
        if (auto *FnIR = ProtoAST->codegen()) {
            fprintf(stderr, "Read extern: ");
            FnIR->print(errs());
            fprintf(stderr, "\n");
        }
    }
    else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (auto FnAST = ParseTopLevelExpr()) {
        if (!FnAST->codegen()) {
            fprintf(stderr, "Error generating code for top level expr");
        }
    }
    else {
        // Skip token for error recovery.
        getNextToken();
    }
}

/// top ::= definition | external | expression | ';'
static void MainLoop() {
    while (true) {
        fprintf(stderr, "ready> ");
        switch (CurTok) {
        case tok_eof:
            return;
        case ';': // ignore top-level semicolons.
            getNextToken();
            break;
        case tok_def:
            HandleDefinition();
            break;
        case tok_extern:
            HandleExtern();
            break;
        default:
            HandleTopLevelExpression();
            break;
        }
    }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
int main() {
    InitializeNativeTarget();
    InitializeAllTargetInfos();
    InitializeAllTargetMCs();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['='] = 2;
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40; // highest.

    // Prime the first token.
    getNextToken();

    //    TheJIT = ExitOnErr(KaleidoscopeJIT::Create());

    auto module = InitializeModule();

    // Add the current debug info version into the module.
    module->addModuleFlag(
        Module::Warning, "Debug Info Version", DEBUG_METADATA_VERSION);

    // Darwin only supports dwarf2.
    if (Triple(sys::getProcessTriple()).isOSDarwin())
        module->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);

    // Construct the DIBuilder, we do this here because we need the module.
    DBuilder = std::make_unique<DIBuilder>(*module);

    // Create the compile unit for the module.
    // Currently down as "fib.ks" as a filename since we're redirecting stdin
    // but we'd like actual source locations.
    KSDbgInfo.TheCU =
        DBuilder->createCompileUnit(dwarf::DW_LANG_C,
                                    DBuilder->createFile("fib.ks", "."),
                                    "Kaleidoscope Compiler",
                                    0,
                                    "",
                                    0);

    // Run the main "interpreter loop" now.
    MainLoop();

    // Finalize the debug info.
    DBuilder->finalize();

    // Print out all of the generated code.
    TheModule->print(errs(), nullptr);

    { // output .o file From part 8

        std::string error;

        auto targetTripple =
            sys::getDefaultTargetTriple(); // What architecture to compile for

        auto target = TargetRegistry::lookupTarget(targetTripple, error);

        // Print an error and exit if we couldn't find the requested target.
        // This generally occurs if we've forgotten to initialise the
        // TargetRegistry or we have a bogus target triple.
        if (!target) {
            errs() << error;
            return 1;
        }

        auto CPU = "generic";
        auto Features = "";
        TargetOptions opt;
        auto RM = Optional<Reloc::Model>{};
        auto TargetMachine =
            target->createTargetMachine(targetTripple, CPU, Features, opt, RM);

        // Not required but could add some performance apparently
        module->setDataLayout(TargetMachine->createDataLayout());
        module->setTargetTriple(targetTripple);

        auto filename = "output.o";
        std::error_code ec;

        raw_fd_ostream dest(filename, ec, sys::fs::OF_None);

        auto pass = legacy::PassManager{};
        auto fileType = CGFT_ObjectFile;

        if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
            errs() << "TargetMachine cant emit a file of this type";
            return 1;
        }

        pass.run(*module);
        dest.flush();
    }

    return 0;
}
