#include "Lexer.h"

Lexer::Lexer(const std::string& source)
    : source(source), position(0), readPosition(0), currentChar(0), line(1) {
    ReadChar();
}

void Lexer::ReadChar() {
    if (readPosition >= source.length()) {
        currentChar = '\0';
    }
    else {
        currentChar = source[readPosition];
    }
    position = readPosition;
    readPosition++;
}

char Lexer::PeekChar() const {
    if (readPosition >= source.length()) {
        return '\0';
    }
    return source[readPosition];
}

void Lexer::SkipWhitespace() {
    while (currentChar == ' ' || currentChar == '\t' || currentChar == '\r') {
        ReadChar();
    }
}

void Lexer::SkipComment() {
    if (currentChar == '/' && PeekChar() == '/') {
        ReadChar(); // consume first '/'
        ReadChar(); // consume second '/'
        while (currentChar != '\n' && currentChar != '\0') {
            ReadChar();
        }
    }
}

Token Lexer::NextToken() {
    SkipWhitespace();
    SkipComment();

    Token token;

    switch (currentChar) {
    case '\0':
        token = MakeToken(TokenType::END_OF_FILE, "");
        break;

    case '\n':
        token = MakeToken(TokenType::NEWLINE, "\\n");
        line++;
        ReadChar();
        break;

    case '#':
        token = MakeToken(TokenType::HASH, "#");
        ReadChar();
        break;

    case ':':
        token = MakeToken(TokenType::COLON, ":");
        ReadChar();
        break;

    case '{':
        token = MakeToken(TokenType::LEFT_BRACE, "{");
        ReadChar();
        break;

    case '}':
        token = MakeToken(TokenType::RIGHT_BRACE, "}");
        ReadChar();
        break;

    case '(':
        token = MakeToken(TokenType::LEFT_PAREN, "(");
        ReadChar();
        break;

    case ')':
        token = MakeToken(TokenType::RIGHT_PAREN, ")");
        ReadChar();
        break;

    case ',':
        token = MakeToken(TokenType::COMMA, ",");
        ReadChar();
        break;

    case '"':
        token = ReadString();
        break;

    case '-':
        if (IsDigit(PeekChar())) {
            token = ReadNumber();
        }
        else {
            token = MakeToken(TokenType::ERROR, "-");
            ReadChar();
        }
        break;

    default:
        if (IsDigit(currentChar)) {
            token = ReadNumber();
        }
        else if (IsLetter(currentChar)) {
            token = ReadIdentifier();
        }
        else {
            token = MakeToken(TokenType::ERROR, std::string(1, currentChar));
            ReadChar();
        }
        break;
    }

    return token;
}

Token Lexer::ReadNumber() {
    size_t start = position;
    if (currentChar == '-') {
        ReadChar();
    }
    while (IsDigit(currentChar)) {
        ReadChar();
    }
    if (currentChar == '.') {
        ReadChar();
        while (IsDigit(currentChar)) {
            ReadChar();
        }
    }
    return MakeToken(TokenType::NUMBER, source.substr(start, position - start));
}

Token Lexer::ReadString() {
    ReadChar(); // consume opening quote
    size_t start = position;
    while (currentChar != '"' && currentChar != '\0') {
        ReadChar();
    }
    std::string value = source.substr(start, position - start);
    if (currentChar == '"') {
        ReadChar(); // consume closing quote
    }
    return MakeToken(TokenType::STRING, value);
}

Token Lexer::ReadIdentifier() {
    size_t start = position;
    while (IsLetter(currentChar) || IsDigit(currentChar) || currentChar == '_' || currentChar == '.') {
        ReadChar();
    }
    return MakeToken(TokenType::IDENTIFIER, source.substr(start, position - start));
}

bool Lexer::IsLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::IsDigit(char c) {
    return c >= '0' && c <= '9';
}

Token Lexer::MakeToken(TokenType type, const std::string& literal) {
    return Token(type, literal, line);
}