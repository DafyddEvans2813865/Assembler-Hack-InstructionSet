#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Code.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: assembler <file.asm>\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in)
    {
        std::cerr << "could not open " << argv[1] << "\n";
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;

    // cleaning loop
    while (std::getline(in, line))
    {

        auto c = line.find("//");
        if (c != std::string::npos)
        {
            line.erase(c);
        }

        // strip white spcae
        line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char ch)
                                  { return std::isspace(ch); }),
                   line.end());

        if (line.empty())
            continue;

        lines.push_back(line);
    }
    in.close();

    std::string outPath = argv[1];
    outPath = outPath.substr(0, outPath.rfind('.')) + ".hack";
    std::ofstream out(outPath);

    int rom = 0;
    for (const auto &l : lines)
    {
        if (l[0] == '(')
            Code::recordLabel(l, rom);
        else
            ++rom;
    }

    for (const auto &l : lines)
    {
        if (l[0] == '(')
            continue;
        out << (l[0] == '@' ? Code::translateA(l) : Code::translateC(l)) << "\n";
    }

    return 0;
}