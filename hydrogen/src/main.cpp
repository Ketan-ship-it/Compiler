#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <vector>
#include "./parser.hpp"
#include "./tokenization.hpp"
#include "./generator.hpp"


int main(int argc,char* argv[]){
    if (argc!=2)
    {
        std::cerr << "Incorrect Usage . Correct Usage is ..."<<std::endl;
        std::cerr << "hydro <Input.hy >" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1],std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }
    // std::cout << contents << std::endl;
    Tokenizer tokenizer(std::move(contents));
    std::vector<TOKEN> tokens=tokenizer.tokenize();
    // std::cout << tokens.size() << std::endl;

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_Prog();

    if(!prog.has_value()){
        std::cerr << "Invalid Program " <<std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(prog.value());
    {
        std::fstream file("out.asm" , std::ios::out);
        file << generator.generate_prog();
    }
    
    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}
