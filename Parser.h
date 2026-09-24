#ifndef PARSER_H
#define PARSER_H

#include "Term.h"
#include "Unifier.h"

#include <string>

class Parser {
public:
    explicit Parser(const std::string& text);

    std::shared_ptr<Term> parseTerm();
    Equation parseEquation();

private:
    std::string text;
    size_t position;

    void skipSpaces();

    bool isEnd() const;

    char currentChar() const;

    bool isVariableStart(char c) const;
    bool isNameChar(char c) const;

    std::string parseName();

    std::shared_ptr<Term> parseTermInternal();

    void expect(char c);
};

#endif
