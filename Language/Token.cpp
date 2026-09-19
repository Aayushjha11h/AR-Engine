#include "Token.h"

std::string TokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::END_OF_FILE:  return "EOF";
    case TokenType::NUMBER:       return "NUMBER";
    case TokenType::STRING:       return "STRING";
    case TokenType::IDENTIFIER:   return "IDENTIFIER";
    case TokenType::HASH:         return "HASH";
    case TokenType::COLON:        return "COLON";
    case TokenType::LEFT_BRACE:   return "LEFT_BRACE";
    case TokenType::RIGHT_BRACE:  return "RIGHT_BRACE";
    case TokenType::LEFT_PAREN:   return "LEFT_PAREN";
    case TokenType::RIGHT_PAREN:  return "RIGHT_PAREN";
    case TokenType::NEWLINE:      return "NEWLINE";
    case TokenType::COMMA:        return "COMMA";
    case TokenType::ERROR:        return "ERROR";
    default:                      return "UNKNOWN";
    }
}

std::string Token::ToString() const {
    return TokenTypeToString(type) + "(" + literal + ") Line:" + std::to_string(line);
}