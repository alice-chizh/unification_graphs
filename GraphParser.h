#ifndef GRAPH_PARSER_H
#define GRAPH_PARSER_H
#include "Term.h"
#include "Unifier.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class GraphParser {
public:
    explicit GraphParser(const std::string& text);
    std::vector<Equation> parseSystem();

private:
    std::string text;
    size_t position; // индекс текущего символа
    std::unordered_map<std::string, std::shared_ptr<Term>> labels; // таблица меток вершин (имя метки(например, 1), узел графа)
    std::unordered_map<std::string, bool> defined; // было ли уже определение метки
    std::unordered_map<std::string, std::shared_ptr<Term>> variables; // таблица переменных (имя, один узел-переменная)

    void skipSpaces();
    bool isEnd() const;
    char currentChar() const;
    void expect(char c); //обязан стоять символ с

    std::string parseName();
    std::string parseLabel();

    std::shared_ptr<Term> parseTerm();
    std::shared_ptr<Term> parseNode();
    std::shared_ptr<Term> labelNode(const std::string& label); // получение вершины по метке
};
#endif