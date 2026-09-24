#include "Parser.h"

#include <cctype>
#include <stdexcept>

Parser::Parser(const std::string& text)
        : text(text), position(0) {
}

void Parser::skipSpaces() {
    while (!isEnd() &&
           std::isspace(
                   static_cast<unsigned char>(currentChar())
           )) {
        ++position;
    }
}

bool Parser::isEnd() const {
    return position >= text.size();
}

char Parser::currentChar() const {
    if (isEnd()) {
        return '\0';
    }

    return text[position];
}

bool Parser::isVariableStart(char c) const {
    return std::isupper(
            static_cast<unsigned char>(c)
    );
}

bool Parser::isNameChar(char c) const {
    return std::isalnum(
            static_cast<unsigned char>(c)
    ) || c == '_';
}

std::string Parser::parseName() {
    skipSpaces();

    if (isEnd()) {
        throw std::runtime_error(
                "Unexpected end of input while parsing name"
        );
    }

    if (!std::isalpha(
            static_cast<unsigned char>(currentChar()))
            ) {
        throw std::runtime_error(
                "Expected a name at position " +
                std::to_string(position)
        );
    }

    std::string name;

    while (!isEnd() && isNameChar(currentChar())) {
        name += currentChar();
        ++position;
    }

    return name;
}

std::shared_ptr<Term> Parser::parseTerm() {
    auto term = parseTermInternal();

    skipSpaces();

    if (!isEnd()) {
        throw std::runtime_error(
                "Unexpected character at position " +
                std::to_string(position)
        );
    }

    return term;
}

std::shared_ptr<Term> Parser::parseTermInternal() {
    skipSpaces();

    std::string name = parseName();

    skipSpaces();

    // Если после имени нет '(',
    // значит это переменная или константа.
    if (isEnd() || currentChar() != '(') {
        if (isVariableStart(name[0])) {
            return Term::Var(name);
        }

        return Term::Fun(name);
    }

    // Если есть '(',
    // значит это функциональный символ.
    ++position; // пропускаем '('

    std::vector<std::shared_ptr<Term>> arguments;

    skipSpaces();

    // Случай f()
    if (currentChar() == ')') {
        ++position;
        return Term::Fun(name, arguments);
    }

    while (true) {
        arguments.push_back(
                parseTermInternal()
        );

        skipSpaces();

        if (currentChar() == ')') {
            ++position;
            break;
        }

        if (currentChar() != ',') {
            throw std::runtime_error(
                    "Expected ',' or ')' at position " +
                    std::to_string(position)
            );
        }

        ++position; // пропускаем ','
    }

    return Term::Fun(name, arguments);
}

void Parser::expect(char c) {
    skipSpaces();

    if (isEnd() || currentChar() != c) {
        throw std::runtime_error(
                std::string("Expected '") +
                c +
                "' at position " +
                std::to_string(position)
        );
    }

    ++position;
}

Equation Parser::parseEquation() {
    skipSpaces();

    auto left = parseTermInternal();

    skipSpaces();

    if (isEnd() || currentChar() != '=') {
        throw std::runtime_error(
                "Expected '=' at position " +
                std::to_string(position)
        );
    }

    ++position; // пропускаем '='

    auto right = parseTermInternal();

    skipSpaces();

    if (!isEnd()) {
        throw std::runtime_error(
                "Unexpected character at position " +
                std::to_string(position)
        );
    }

    return {left, right};
}