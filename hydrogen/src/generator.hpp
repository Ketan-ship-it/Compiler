#pragma once

#define _GLIBCXX_NODISCARD [[__nodiscard__]]

#include <unordered_map>
#include <sstream>
#include <iostream>
#include "./parser.hpp"
#include <cassert>

class Generator
{
private:
    struct Var{
        size_t stack_loc;
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size =0;
    std::unordered_map<std::string , Var> m_vars{};

    void push(std::string reg){
        m_output << "    push " << reg <<"\n";
        m_stack_size++;
    }
    void pop(std::string reg){
        m_output << "    pop " << reg <<"\n";
        m_stack_size--;
    }
public:
    inline Generator(NodeProg prog)
        :m_prog(std::move(prog))
    {
    }
    void gen_Term(const NodeTerm* Term){
        struct TermVisitor{
            Generator* gen;
            void operator()(const NodeTermInt_Lit* integer)const{
                std::cout << "Generating assembly for int_lit"<<std::endl;
                gen->m_output << "    mov rax, " << integer->int_lit.value.value() <<"\n";
                gen->push("rax");
            }
            void operator()(const NodeTermIden* ident)const{
                if(!gen->m_vars.contains(ident->iden.value.value())){
                    std::cerr << "Identifier \'" << ident->iden.value.value() << "\' not declared and initialised" <<std::endl;
                    exit(EXIT_FAILURE);
                }
                const auto& var = gen->m_vars.at(ident->iden.value.value());
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc -1)*8 << "]\n";
                gen->push(offset.str());
            }
            void operator()(const NodeNestedExpr* nested){
                gen->gen_Expr(nested->exp);
            }
        };
        TermVisitor visitor{.gen = this};
        std::visit(visitor , Term->term);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr){
        struct BinaryVisitor{
            Generator* gen;
            void operator()(const NodeAdd* bin_add)const{
                gen->gen_Expr(bin_add->lhs);
                gen->gen_Expr(bin_add->rhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax ,rbx\n" ;
                gen->push("rax");
            }
            void operator()(const NodeSub* bin_sub)const{
                gen->gen_Expr(bin_sub->rhs);
                gen->gen_Expr(bin_sub->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    sub rax ,rbx\n" ;
                gen->push("rax");
            }
            void operator()(const NodeDiv* bin_div)const{
                gen->gen_Expr(bin_div->rhs);
                gen->gen_Expr(bin_div->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    div rbx\n" ;
                gen->push("rax");
            }
            void operator()(const NodeMulti* bin_multi)const{
                gen->gen_Expr(bin_multi->lhs);
                gen->gen_Expr(bin_multi->rhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    mul rbx\n" ;
                gen->push("rax");
            }
        };
        BinaryVisitor vistor{.gen = this};
        std::visit(vistor , bin_expr->ope);
    }

    void gen_Expr(const NodeExpr* expr) {
        struct ExprVisitor{
            Generator* gen;
            void operator()(const NodeTerm* Term){
                gen->gen_Term(Term);
            }
            void operator()(const NodeBinExpr* bin_xpr){
                gen->gen_bin_expr(bin_xpr);
                
            }
        };
        ExprVisitor visitor{.gen = this};
        std::visit(visitor,expr->var);
    }

    void gen_stmt(const NodeStmt* stmt){
        struct StmtVisitor{
            Generator* gen;
            void operator()(const NodeStmtExit* exit_)const{
                gen->gen_Expr(exit_->expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }
            void operator()(const NodeStmtlet* let_){
                if(gen->m_vars.contains(let_->ident.value.value())){
                    std::cerr << "Identifier \'" << let_->ident.value.value() << "\' already used" <<std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({let_->ident.value.value() , Var{.stack_loc = gen->m_stack_size}});
                gen->gen_Expr(let_->expr);

            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor , stmt->stmnt);
    }

    _GLIBCXX_NODISCARD std::string generate_prog() {
        
        m_output << "global _start\n_start:\n";

        std::cout << "Generating program , no of statements: "<< m_prog.stmts.size() << std::endl;
        for(const NodeStmt* stmt : m_prog.stmts){
            std::cout << "Inside genreating statements" << std::endl;
            gen_stmt(stmt);
        }


        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall";

        return m_output.str();
    }
};
