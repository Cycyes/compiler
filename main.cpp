#include "Common.h"
#include "LexcalAnalyser.h"
#include "SyntaxAnalyser.h"
#include "Optimizer.h"
#include "ObjectCodeGenerater.h"

int main() {
    // 初始化时间
    time_t begin, end;

    ifstream fin("./input/test.txt", ios::in);
    stringstream ss;
    ss << fin.rdbuf();
    string fstring = ss.str();

    bool isOutIntermediateCode = false;
    bool isOutObjectCode = false;

    // 静态语法分析
    cout << "正在语法分析..." << endl;
    begin = clock();
    IntermediateCodeGenerater intermediateCodeGenerater(fstring);
    end = clock();
    cout << endl << "语法分析成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    // 生成中间代码
    cout << endl << "正在生成中间代码..." << endl;
    begin = clock();
    intermediateCodeGenerater.analyse("./output/SyntaxTree.dot");
    end = clock();
    cout << "生成中间代码成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    // 保存中间代码
    cout << endl << "正在保存中间代码..." << endl;
    begin = clock();
    intermediateCodeGenerater.showIntermediateCode("./output/IntermediateCode.txt", isOutIntermediateCode);
    end = clock();
    cout << "保存中间代码成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;
    
    // 画出语法树
    cout << endl << "正在绘制语法树..." << endl;
    begin = clock();
    intermediateCodeGenerater.drawSyntaxTree();
    end = clock();
    cout << "绘制语法树成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;


    // 优化代码
    cout << endl << "正在优化代码..." << endl;
    begin = clock();
    Optimizer optimizer(intermediateCodeGenerater.lexcalAnalyser.varTable, intermediateCodeGenerater.semanticAnalyser.gSymbolTable, intermediateCodeGenerater.semanticAnalyser.intermediateCode);
    optimizer.analyse();
    end = clock();
    cout << "优化代码成功，优化率为" << optimizer.optimizeRate << "，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    // 目标代码
    cout << endl << "正在生成目标代码..." << endl;
    begin = clock();
    ObjectCodeGenerater objectCodeGenerater(optimizer.intermediateCode, optimizer.block, 8);
    objectCodeGenerater.generateObjectCode();
    end = clock();
    cout << "生成目标代码成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    // 输出目标代码
    cout << endl << "正在保存目标代码..." << endl;
    begin = clock();
    objectCodeGenerater.printObjectCode("./output/ObjectCode.txt");
    end = clock();
    cout << "保存目标代码成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    return 0;
}