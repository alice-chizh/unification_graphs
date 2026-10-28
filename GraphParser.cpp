#include "GraphParser.h"
#include <cctype>
#include <stdexcept>

GraphParser::GraphParser(const std::string& text) : text(text), position(0) {}

void GraphParser::skipSpaces() {
    while (!isEnd() && std::isspace(static_cast<unsigned char>(currentChar()))) {
        ++position;
    }
}

bool GraphParser::isEnd() const {
    return position >= text.size();
}

char GraphParser::currentChar() const {
    return isEnd() ? '\0' : text[position];
}

void GraphParser::expect(char c) {
    skipSpaces();
    if (currentChar() != c) {
        throw std::runtime_error(std::string("Expected '") + c + "' at position " + std::to_string(position));
    }
    ++position;
}

std::string GraphParser::parseName() {
    skipSpaces();
    if (!std::isalpha(static_cast<unsigned char>(currentChar()))) {
        throw std::runtime_error("Expected a name at position " + std::to_string(position));
    }
    std::string name;
    while (!isEnd() && (std::isalnum(static_cast<unsigned char>(currentChar())) || currentChar() == '_')) {
        name += currentChar();
        ++position;
    }
    return name;
}

std::string GraphParser::parseLabel() {
    expect('$');
    std::string label;
    while (!isEnd() && std::isalnum(static_cast<unsigned char>(currentChar()))) {
        label += currentChar();
        ++position;
    }
    if (label.empty()) {
        throw std::runtime_error("Empty label at position " + std::to_string(position));
    }
    return label;
}

// Вершина для метки. Если метка встречается впервые (ссылка вперёд), создаём пустую вершину, которую заполним при определении метки.
std::shared_ptr<Term> GraphParser::labelNode(const std::string& label) {
    auto it = labels.find(label);
    if (it != labels.end()) {
        return it->second;
    }
    auto node = Term::Fun("");
    labels[label] = node;
    defined[label] = false;
    return node;
}

std::shared_ptr<Term> GraphParser::parseTerm() { //Правило: терм := '$' метка [ ':' вершина ] | вершина
    skipSpaces();
    if (currentChar() != '$') {
        return parseNode();
    }
    std::string label = parseLabel();
    auto node = labelNode(label);
    skipSpaces();

    // $k — просто ссылка на вершину.
    if (currentChar() != ':') {
        return node;
    }

    // $k:терм — определение вершины.
    ++position;

    if (defined[label]) {
        throw std::runtime_error("Label $" + label + " is defined twice");
    }
    defined[label] = true;
    auto body = parseNode();
    node->type = body->type;
    node->name = body->name;
    node->args = body->args;

    // Переменная должна оставаться одной вершиной.
    if (node->isVariable()) {
        labels[label] = body;
        return body;
    }
    return node;
}

std::shared_ptr<Term> GraphParser::parseNode() {
    std::string name = parseName();
    bool isVariable = std::isupper(static_cast<unsigned char>(name[0]));
    skipSpaces();
    if (currentChar() != '(') {
        if (!isVariable) {
            return Term::Fun(name);
        }
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        auto variable = Term::Var(name);
        variables[name] = variable;
        return variable;
    }
    if (isVariable) {
        throw std::runtime_error("Variable " + name + " cannot have arguments");
    }
    ++position; // пропускаем '('
    std::vector<std::shared_ptr<Term>> arguments;
    skipSpaces();
    if (currentChar() == ')') {
        ++position;
        return Term::Fun(name, arguments);
    }

    while (true) {
        arguments.push_back(parseTerm());
        skipSpaces();
        if (currentChar() == ')') {
            ++position;
            break;
        }
        expect(',');
    }
    return Term::Fun(name, arguments);
}

std::vector<Equation> GraphParser::parseSystem() {
    std::vector<Equation> system;

    while (true) {
        auto left = parseTerm();
        expect('=');
        auto right = parseTerm();
        system.push_back({left, right});
        skipSpaces();
        if (isEnd()) {
            break;
        }
        expect(';');
    }

    for (const auto& [label, isDefined] : defined) {
        if (!isDefined) {
            throw std::runtime_error("Label $" + label + " is never defined");
        }
    }
    return system;
}