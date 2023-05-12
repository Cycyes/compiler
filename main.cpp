#include "Common.h"
#include "LexcalAnalyser.h"
#include "SyntaxAnalyser.h"
#include "Optimizer.h"
#include "ObjectCodeGenerater.h"

int main() {
    ifstream fin("input.txt", ios::in);
    stringstream ss;
    ss << fin.rdbuf();
    string fstring = ss.str();

    //cout << fstring;
    // 中间代码
    syntaxAnalysis syntax;
    syntax.initializeLR1();
    syntax.getInput(fstring);
    syntax.analysis();
    // 优化代码
    optimizerAnalysis optimizer(syntax.L.varTable, syntax.S.gSymbolTable, syntax.S.intermediateCode);
    //optimizerAnalysis optimizer(syntax.L.nameTable, syntax.S.global_table, syntax.S.intermediate_code);
    double opt_rate = optimizer.analysis();
    // 目标代码
    objectCodeGenerator MIPSgenerator(optimizer.intermediate_code, optimizer.block_group, 8);
    MIPSgenerator.geneObjectCode();
    //
    fstream fout("ObjectCode.txt", ios::out);
    for (auto i = 0; i < MIPSgenerator.object_code.size(); i++)
        fout << MIPSgenerator.object_code[i] << endl;
    fout.close();
    return 0;
}