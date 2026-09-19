#pragma once
#include "Token.h"
#include <string>

class Lexer {
public:
    explicit Lexer(const std::string& source);

    Token NextToken();
    int GetLine() const { return line; }

private:
    std::string source;
    size_t position;
    size_t readPosition;
    char currentChar;
    int line;

    void ReadChar();
    char PeekChar() const;
    void SkipWhitespace();
    void SkipComment();

    Token ReadNumber();
    Token ReadString();
    Token ReadIdentifier();

    static bool IsLetter(char c);
    static bool IsDigit(char c);

    Token MakeToken(TokenType type, const std::string& literal);
};