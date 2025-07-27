#include "lexer.hpp"
#include "parser.hpp"
#include "generator.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>

void compileNRun(std::string &code)
{
    const std::string cppFile = "_temp.cxx";
    const std::string exeFile =
    #ifdef _WIN32
        "_temp.exe";
    #else
        "_temp";
    #endif

    std::ofstream f(cppFile);
    if (!f.is_open())
    {
        return;
    }

    f << code;
    f.close();

    std::string compileCmd = "g++ " + cppFile + " -o " + exeFile;
    int compileResult = system(compileCmd.c_str());

    if (compileResult != 0)
    {
       std::remove(cppFile.c_str());
        return;
    }

    int runResult =
    #ifdef _WIN32
        system(exeFile.c_str());
    #else
        system(("./" + exeFile).c_str());
    #endif

    std::remove(cppFile.c_str());
    std::remove(exeFile.c_str());

    if (runResult != 0)
    {
        std::cout << "Program exited with error.\n";
    }
}


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <source_file>\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << argv[1] << "\n";
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parse(tokens);
    auto ast = parse.parse();
    Generator generator(ast);
    std::string cppCode = generator.generate();
    compileNRun(cppCode);
    file.close();
    return 0;
}