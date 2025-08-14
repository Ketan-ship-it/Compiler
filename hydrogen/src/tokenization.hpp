#pragma once

#include <string>
#include <vector>
#include <optional>
#include <iostream>
#define _GLIBCXX_NODISCARD [[__nodiscard__]]

enum class TOKENTYPE{
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    iden,
    eq,
    let,
    plus,
    star,
    sub,
    div
};

std::optional<int> is_Bin_Op(TOKENTYPE type){
    switch (type){
    case TOKENTYPE::plus:
    case TOKENTYPE::sub:
        return 1;
    case TOKENTYPE::star:        
        return 2;
    case TOKENTYPE::div:
        return 3;
    default:
        return {};
    }
}

struct TOKEN{
    TOKENTYPE type;
    std::optional<std::string> value{};
};


class Tokenizer
{
private:
    _GLIBCXX_NODISCARD inline std::optional<char> peek(int mov_ahead = 1) const{
        if(m_index + mov_ahead > m_src.length()){
            return {};
        }else{
            return m_src.at(m_index+mov_ahead-1);
        }
    }

    inline char consume(){
        return m_src.at(m_index++);
    }

   const std::string m_src;
   int m_index;
public:
    inline explicit Tokenizer(std::string src)
        :m_src(std::move(src)),m_index(0)
    {
    }

    inline std::vector<TOKEN> tokenize(){
        std::vector<TOKEN> tokens{};
        std::string buf;

        // std::cout << "Tokenizing source: \"" << m_src << "\"\n";

        while (peek().has_value()) {
            // std::cout << "Current character: '" << peek().value() << "', index: " << m_index << "\n";

            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if (buf == "exit") {
                    tokens.push_back({.type = TOKENTYPE::exit});
                    buf.clear();
                }else if(buf == "let"){
                    tokens.push_back({.type = TOKENTYPE::let});
                    buf.clear();
                } else {
                    tokens.push_back({.type = TOKENTYPE::iden , .value = buf});
                    buf.clear();
                }
            } else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({.type = TOKENTYPE::int_lit, .value = buf});
                buf.clear();
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TOKENTYPE::open_paren});
            }else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TOKENTYPE::eq});
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TOKENTYPE::semi});
            }else if (peek().value() == '+') {
                consume();
                tokens.push_back({TOKENTYPE::plus});
            }else if (peek().value() == '*') {
                consume();
                tokens.push_back({TOKENTYPE::star});
            }else if (peek().value() == '-') {
                consume();
                tokens.push_back({TOKENTYPE::sub});
            }else if (peek().value() == '/') {
                consume();
                tokens.push_back({TOKENTYPE::div});
            }else if (peek().value() == ')') {
                consume();
                tokens.push_back({TOKENTYPE::close_paren});
            }else if (std::isspace(peek().value())) {
                consume();
            }else {
                std::cerr << "Error: Invalid character '" << peek().value() << "'\n";
                exit(EXIT_FAILURE);
            }
        }

        // std::cout << "Tokenization complete. Token count: " << tokens.size() << "\n";
        
        return tokens;
    }
};


