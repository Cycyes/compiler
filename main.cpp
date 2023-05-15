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
    // �м����
    
    if (1) {
        IntermediateCodeGenerater syntax(fstring);
        // syntax.initializeLR1();
        // syntax.getInput(fstring);
        syntax.analyse();
        // �Ż�����
        optimizerAnalysis optimizer(syntax.lexcalAnalyser.varTable, syntax.semanticAnalyser.gSymbolTable, syntax.semanticAnalyser.intermediateCode);
        double opt_rate = optimizer.analysis();

        // Ŀ�����
        objectCodeGenerator MIPSgenerator(optimizer.intermediate_code, optimizer.block_group, 8);
        MIPSgenerator.geneObjectCode();
        //
        fstream fout("ObjectCode.txt", ios::out);
        for (auto i = 0; i < MIPSgenerator.object_code.size(); i++)
            fout << MIPSgenerator.object_code[i] << endl;
        fout.close();
    }
    
   
    
    return 0;
}