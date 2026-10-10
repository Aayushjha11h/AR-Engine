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
    case TokenType::DOLLAR:       return "DOLLAR";
    case TokenType::EQUAL:        return "EQUAL";
    case TokenType::EQUAL_EQUAL:  return "EQUAL_EQUAL";
    case TokenType::NOT_EQUAL:    return "NOT_EQUAL";
    case TokenType::LESS:         return "LESS";
    case TokenType::GREATER:      return "GREATER";
    case TokenType::LESS_EQUAL:   return "LESS_EQUAL";
    case TokenType::GREATER_EQUAL:return "GREATER_EQUAL";
    case TokenType::PLUS:         return "PLUS";
    case TokenType::MINUS:        return "MINUS";
    case TokenType::STAR:         return "STAR";
    case TokenType::SLASH:        return "SLASH";
    case TokenType::PERCENT:      return "PERCENT";
    default:                      return "UNKNOWN";
    }
}

std::string Token::ToString() const {
    return TokenTypeToString(type) + "(" + literal + ") Line:" + std::to_string(line);
}