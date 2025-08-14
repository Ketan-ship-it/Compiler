#pragma once

#define _GLIBCXX_NODISCARD [[__nodiscard__]]

#include <variant>
#include "./tokenization.hpp"
#include "./arena.hpp"

struct NodeTermIden
{
    TOKEN iden;
};

struct NodeTermInt_Lit{
    TOKEN int_lit;
};

struct NodeExpr;

struct NodeNestedExpr{
    NodeExpr* exp;
};

struct NodeTerm{
    std::variant<NodeTermInt_Lit* , NodeTermIden* , NodeNestedExpr*> term;
};

struct NodeAdd{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeMulti{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeSub{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeDiv{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr{
    std::variant<NodeAdd* , NodeMulti* , NodeSub* , NodeDiv*> ope;
};

struct NodeExpr{
    std::variant<NodeTerm*, NodeBinExpr* > var;
};

struct NodeStmtExit{
    NodeExpr* expr;
};

struct NodeStmtlet{
    TOKEN ident;
    NodeExpr* expr;
};

struct NodeStmt{
    std::variant<NodeStmtExit* , NodeStmtlet*> stmnt;
};

struct NodeProg{
    std::vector<NodeStmt*> stmts;
};

class Parser
{
private:
    _GLIBCXX_NODISCARD inline std::optional<TOKEN> peek(int mov_ahead = 1) const{
        if(m_index + mov_ahead > m_tokens.size()){
            return {};
        }else{
            return m_tokens.at(m_index+mov_ahead-1);
        }
    }

    inline TOKEN consume(){
        return m_tokens.at(m_index++);
    }

   const std::vector<TOKEN> m_tokens;
   int m_index;
   ArenaAllocator m_allocator;
public:
    inline explicit Parser(std::vector<TOKEN> tokens)
        :m_tokens(std::move(tokens)),m_index(0),
        m_allocator(1024 * 1024 * 4)
    {
    }

    std::optional<NodeTerm*> parse_term(){
        if(peek().has_value() && peek().value().type == TOKENTYPE::int_lit){
            auto term_int_lit = m_allocator.alloc<NodeTermInt_Lit>(); 
            term_int_lit->int_lit = consume();
            auto Term = m_allocator.alloc<NodeTerm>();
            Term->term = term_int_lit;
            return Term;
        }else if(peek().has_value() && peek().value().type == TOKENTYPE::iden){
            auto term_iden = m_allocator.alloc<NodeTermIden>(); 
            term_iden->iden = consume();
            auto Term = m_allocator.alloc<NodeTerm>();
            Term->term = term_iden;
            return Term;
        }else if(peek().has_value() && peek().value().type == TOKENTYPE::open_paren){
            consume();
            auto expr = parse_Expr();
            if(!expr.has_value()){
                std::cerr << "Empty Parenthesis" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TOKENTYPE::close_paren){
                consume();
            }else{
                std::cerr << "Missing Parenthesis ')'" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            auto term_nested_expr = m_allocator.alloc<NodeTerm>();
            auto term_expr = m_allocator.alloc<NodeNestedExpr>();
            term_expr->exp = expr.value();
            term_nested_expr->term=term_expr;
            return term_nested_expr;
        }else{
            return {};
        }
    }

    std::optional<NodeExpr*> parse_Expr(int min_prec = 0){
        // std::cout << " recursive call" << std::endl;
        std::optional<NodeTerm*> lhs = parse_term();
        if(!lhs.has_value()){
            return {};
        }
        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = lhs.value();
        while(true){
            // std::cout << " inside loop" << std::endl;
            std::optional<TOKEN> curr_tok = peek();
            std::optional<int> prec;
            if (curr_tok.has_value()){
                // std::cout << " checking token" << std::endl;
                prec = is_Bin_Op(curr_tok->type);
                if(!prec.has_value() || prec < min_prec){
                    break;
                }
                // std::cout << " token found" << std::endl;
            }else{
                break;
            }

            TOKEN op = consume();
            int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_Expr(next_min_prec);
            if(!expr_rhs.has_value()){
                std::cerr << "Unable to parse expression" << std::endl ;
                exit(EXIT_FAILURE);
            }
            auto bin_expr = m_allocator.alloc<NodeBinExpr>();
            auto expr_lhs_2 = m_allocator.alloc<NodeExpr>();
            if(op.type == TOKENTYPE::plus){
                auto bin_add = m_allocator.alloc<NodeAdd>();
                expr_lhs_2->var = expr_lhs->var;
                bin_add->lhs = expr_lhs_2;
                bin_add->rhs = expr_rhs.value();
                bin_expr->ope = bin_add;
            }else if(op.type == TOKENTYPE::star){
                auto bin_multi = m_allocator.alloc<NodeMulti>();
                expr_lhs_2->var = expr_lhs->var;
                bin_multi->lhs = expr_lhs_2;
                bin_multi->rhs = expr_rhs.value();
                bin_expr->ope = bin_multi;
            }else if(op.type == TOKENTYPE::div){
                auto bin_div = m_allocator.alloc<NodeDiv>();
                expr_lhs_2->var = expr_lhs->var;
                bin_div->lhs = expr_lhs_2;
                bin_div->rhs = expr_rhs.value();
                bin_expr->ope = bin_div;
            }else if(op.type == TOKENTYPE::sub){
                auto bin_sub = m_allocator.alloc<NodeSub>();
                expr_lhs_2->var = expr_lhs->var;
                bin_sub->lhs = expr_lhs_2;
                bin_sub->rhs = expr_rhs.value();
                bin_expr->ope = bin_sub;
            }
            expr_lhs->var = bin_expr;
        }
        return expr_lhs;
    }

    std::optional<NodeStmt*> parse_Stmnt(){
        if(peek().value().type == TOKENTYPE::exit && peek().has_value() && peek(2).value().type==TOKENTYPE::open_paren){
            consume();
            consume();
            auto exit_stmt = m_allocator.alloc<NodeStmtExit>();
            if(auto node_expr = parse_Expr()){
                exit_stmt->expr = node_expr.value();
            }else{
                std::cerr << "Invalid expression" <<std::endl;
                exit(EXIT_FAILURE);
            }
            if(peek().has_value() && peek().value().type == TOKENTYPE::close_paren){
                consume();
            }else{
                std::cerr << "Invalid expression \n Expected close-paren" <<std::endl;
                exit(EXIT_FAILURE);
            }
            if(peek().has_value() && peek().value().type == TOKENTYPE::semi){
                consume();
            }else{
                std::cerr << "Invalid expression 1 \n Expected semi-colon" <<std::endl;
                exit(EXIT_FAILURE);
            }
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->stmnt = exit_stmt;
            return stmt;
        }else if(peek().has_value() && peek().value().type == TOKENTYPE::let 
                && peek(2).has_value() && peek(2).value().type == TOKENTYPE::iden 
                && peek(3).has_value() && peek(3).value().type == TOKENTYPE::eq){

                consume();
                auto stmt_let = m_allocator.alloc<NodeStmtlet>();
                stmt_let->ident = consume();
                consume();
                if(auto exp = parse_Expr()){
                    stmt_let->expr = exp.value();
                }else{
                    std::cerr << "Invalid expression " <<std::endl;
                    exit(EXIT_FAILURE);
                }
                if(peek().has_value() && peek().value().type == TOKENTYPE::semi){
                    consume();
                }else{
                    std::cerr << "Invalid expression 2\n Expected semi-colon" <<std::endl;
                    exit(EXIT_FAILURE);
                }
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->stmnt=stmt_let;
            return stmt;
        }else{
            std::cerr << "Invalid Statement" << std::endl;
            exit(EXIT_FAILURE);
        }   
    }

    std::optional<NodeProg> parse_Prog(){
        NodeProg prog;
        while (peek().has_value()){
            if(auto stmnt = parse_Stmnt()){
                prog.stmts.push_back(stmnt.value());
            }else{
                std::cerr << "Invalid StateMent" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }
};
 
