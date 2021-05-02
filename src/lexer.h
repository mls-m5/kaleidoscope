#pragma once

#include <llvm/IR/DIBuilder.h>
#include <string>

/// The lexer returns tokens[0 - 255] if it is an unknown character, otherwise
/// one of these for known things.
enum Token {
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5,

    // control
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,
    tok_for = -9,
    tok_in = -10,

    // operators
    tok_binary = -11,
    tok_unary = -12,

    // var definition
    tok_var = -13
};

inline std::string getTokName(int Tok) {
    switch (Tok) {
    case tok_eof:
        return "eof";
    case tok_def:
        return "def";
    case tok_extern:
        return "extern";
    case tok_identifier:
        return "identifier";
    case tok_number:
        return "number";
    case tok_if:
        return "if";
    case tok_then:
        return "then";
    case tok_else:
        return "else";
    case tok_for:
        return "for";
    case tok_in:
        return "in";
    case tok_binary:
        return "binary";
    case tok_unary:
        return "unary";
    case tok_var:
        return "var";
    }
    return std::string(1, (char)Tok);
}

class PrototypeAST;
class ExprAST;

struct SourceLocation {
    int Line;
    int Col;
};

inline SourceLocation CurLoc;
inline SourceLocation LexLoc = {1, 0};

static int advance() {
    int LastChar = getchar();

    if (LastChar == '\n' || LastChar == '\r') {
        LexLoc.Line++;
        LexLoc.Col = 0;
    }
    else
        LexLoc.Col++;
    return LastChar;
}

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number

/// gettok - Return the next token from standard input.
static int gettok() {
    static int LastChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar))
        LastChar = advance();

    CurLoc = LexLoc;

    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (isalnum((LastChar = advance())))
            IdentifierStr += LastChar;

        if (IdentifierStr == "def")
            return tok_def;
        if (IdentifierStr == "extern")
            return tok_extern;
        if (IdentifierStr == "if")
            return tok_if;
        if (IdentifierStr == "then")
            return tok_then;
        if (IdentifierStr == "else")
            return tok_else;
        if (IdentifierStr == "for")
            return tok_for;
        if (IdentifierStr == "in")
            return tok_in;
        if (IdentifierStr == "binary")
            return tok_binary;
        if (IdentifierStr == "unary")
            return tok_unary;
        if (IdentifierStr == "var")
            return tok_var;
        return tok_identifier;
    }

    if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = advance();
        } while (isdigit(LastChar) || LastChar == '.');

        NumVal = strtod(NumStr.c_str(), nullptr);
        return tok_number;
    }

    if (LastChar == '#') {
        // Comment until end of line.
        do
            LastChar = advance();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok();
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    LastChar = advance();
    return ThisChar;
}
