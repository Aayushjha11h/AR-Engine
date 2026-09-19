#pragma once
#include <string>

enum class TokenType {
    END_OF_FILE,
    NUMBER,
    STRING,
    IDENTIFIER,
    HASH,
    COLON,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_PAREN,
    RIGHT_PAREN,
    NEWLINE,
    COMMA,
    ERROR
};

std::string TokenTypeToString(TokenType type);

struct Token {
    TokenType type;
    std::string literal;
    int line;

    Token() : type(TokenType::ERROR), literal(""), line(1) {}
    Token(TokenType t, const std::string& lit, int ln)
        : type(t), literal(lit), line(ln) {
    }

    std::string ToString() const;
};