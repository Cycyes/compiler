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
    

    IntermediateCodeGenerater syntax(fstring);
    // syntax.initializeLR1();
    // syntax.getInput(fstring);
    syntax.analyse("SyntaxTree.dot");
    syntax.drawSyntaxTree();
    // syntax.showIntermediateCode();
    // 优化代码
    Optimizer optimizer(syntax.lexcalAnalyser.varTable, syntax.semanticAnalyser.gSymbolTable, syntax.semanticAnalyser.intermediateCode);
    optimizer.analyse();

    // 目标代码
    objectCodeGenerator MIPSgenerator(optimizer.intermediateCode, optimizer.block, 8);
    MIPSgenerator.geneObjectCode();
    //
    fstream fout("ObjectCode.txt", ios::out);
    for (auto i = 0; i < MIPSgenerator.object_code.size(); i++)
        fout << MIPSgenerator.object_code[i] << endl;
    fout.close();
    
    return 0;
}