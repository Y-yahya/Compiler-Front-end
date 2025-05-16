#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <memory>
#include <sstream>

//--------------------------
// Token and Lexer
//--------------------------
enum class TokenType { Identifier, Number, Keyword, Symbol, EndOfFile, Unknown };

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

class Lexer {
public:
    Lexer(const std::string& source) : src(source), pos(0), line(1), column(1) {}

    Token nextToken() {
        while (pos < src.size() && isspace(src[pos])) advance();
        if (pos >= src.size()) return {TokenType::EndOfFile, "", line, column};

        if (isalpha(src[pos])) return identifier();
        if (isdigit(src[pos])) return number();
        if (ispunct(src[pos])) return symbol();

        return {TokenType::Unknown, std::string(1, src[pos++]), line, column};
    }

private:
    std::string src;
    size_t pos;
    int line, column;

    void advance() {
        if (src[pos] == '\n') { line++; column = 1; }
        else column++;
        pos++;
    }

    Token identifier() {
        int start = pos;
        while (pos < src.size() && isalnum(src[pos])) advance();
        std::string val = src.substr(start, pos - start);
        return { (val == "int" || val == "return") ? TokenType::Keyword : TokenType::Identifier, val, line, column };
    }

    Token number() {
        int start = pos;
        while (pos < src.size() && isdigit(src[pos])) advance();
        return {TokenType::Number, src.substr(start, pos - start), line, column};
    }

    Token symbol() {
        return {TokenType::Symbol, std::string(1, src[pos++]), line, column};
    }
};

//--------------------------------------
// AST Nodes
//--------------------------------------
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

using ASTPtr = std::shared_ptr<ASTNode>;

class NumberNode : public ASTNode {
public:
    int value;
    NumberNode(int val) : value(val) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Number: " << value << '\n';
    }
};

class IdentifierNode : public ASTNode {
public:
    std::string name;
    IdentifierNode(const std::string& n) : name(n) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Identifier: " << name << '\n';
    }
};

class DeclarationNode : public ASTNode {
public:
    std::string type;
    std::string name;
    ASTPtr value;
    DeclarationNode(const std::string& t, const std::string& n, ASTPtr v)
        : type(t), name(n), value(v) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Declaration: " << type << " " << name << '\n';
        if (value) value->print(indent + 2);
    }
};

//--------------------------------------
// Symbol Table
//--------------------------------------
class SymbolTable {
    std::map<std::string, std::string> symbols;
public:
    void declare(const std::string& name, const std::string& type) {
        symbols[name] = type;
    }
    bool exists(const std::string& name) const {
        return symbols.find(name) != symbols.end();
    }
    std::string typeOf(const std::string& name) const {
        auto it = symbols.find(name);
        return it != symbols.end() ? it->second : "";
    }
};

//--------------------------------------
// Parser (Handles simple declarations)
//--------------------------------------
class Parser {
    Lexer& lexer;
    Token current;

    void advance() { current = lexer.nextToken(); }

public:
    Parser(Lexer& lex) : lexer(lex) { advance(); }

    ASTPtr parseDeclaration() {
        if (current.type == TokenType::Keyword && current.value == "int") {
            std::string type = current.value;
            advance();

            if (current.type != TokenType::Identifier) {
                std::cerr << "Expected identifier after 'int'\n";
                return nullptr;
            }

            std::string name = current.value;
            advance();

            if (current.type != TokenType::Symbol || current.value != "=") {
                std::cerr << "Expected '=' after identifier\n";
                return nullptr;
            }
            advance();

            if (current.type != TokenType::Number) {
                std::cerr << "Expected number after '='\n";
                return nullptr;
            }

            int val = std::stoi(current.value);
            advance();

            if (current.type != TokenType::Symbol || current.value != ";") {
                std::cerr << "Expected ';' at the end of declaration\n";
                return nullptr;
            }
            advance();

            return std::make_shared<DeclarationNode>(type, name, std::make_shared<NumberNode>(val));
        }

        std::cerr << "Unexpected token: " << current.value << '\n';
        return nullptr;
    }
};

//--------------------------------------
// Main (Demo)
//--------------------------------------
int main() {
    std::string source = "int x = 42;";
    Lexer lexer(source);
    Parser parser(lexer);
    
    auto ast = parser.parseDeclaration();
    if (ast) ast->print();

    return 0;
}
