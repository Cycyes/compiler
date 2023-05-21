#include "Common.h"
#include "LexcalAnalyser.h"
#include "SyntaxAnalyser.h"
#include "Optimizer.h"
#include "ObjectCodeGenerater.h"

int main() {
    // ��ʼ��ʱ��
    time_t begin, end;

    ifstream fin("./input/test.txt", ios::in);
    stringstream ss;
    ss << fin.rdbuf();
    string fstring = ss.str();

    bool isOutIntermediateCode = false;
    bool isOutObjectCode = false;

    // ��̬�﷨����
    cout << "�����﷨����..." << endl;
    begin = clock();
    IntermediateCodeGenerater intermediateCodeGenerater(fstring);
    end = clock();
    cout << endl << "�﷨�����ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    // �����м����
    cout << endl << "���������м����..." << endl;
    begin = clock();
    intermediateCodeGenerater.analyse("./output/SyntaxTree.dot");
    end = clock();
    cout << "�����м����ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    // �����м����
    cout << endl << "���ڱ����м����..." << endl;
    begin = clock();
    intermediateCodeGenerater.showIntermediateCode("./output/IntermediateCode.txt", isOutIntermediateCode);
    end = clock();
    cout << "�����м����ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;
    
    // �����﷨��
    cout << endl << "���ڻ����﷨��..." << endl;
    begin = clock();
    intermediateCodeGenerater.drawSyntaxTree();
    end = clock();
    cout << "�����﷨���ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;


    // �Ż�����
    cout << endl << "�����Ż�����..." << endl;
    begin = clock();
    Optimizer optimizer(intermediateCodeGenerater.lexcalAnalyser.varTable, intermediateCodeGenerater.semanticAnalyser.gSymbolTable, intermediateCodeGenerater.semanticAnalyser.intermediateCode);
    optimizer.analyse();
    end = clock();
    cout << "�Ż�����ɹ����Ż���Ϊ" << optimizer.optimizeRate << "����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    // Ŀ�����
    cout << endl << "��������Ŀ�����..." << endl;
    begin = clock();
    ObjectCodeGenerater objectCodeGenerater(optimizer.intermediateCode, optimizer.block, 8);
    objectCodeGenerater.generateObjectCode();
    end = clock();
    cout << "����Ŀ�����ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    // ���Ŀ�����
    cout << endl << "���ڱ���Ŀ�����..." << endl;
    begin = clock();
    objectCodeGenerater.printObjectCode("./output/ObjectCode.txt");
    end = clock();
    cout << "����Ŀ�����ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    return 0;
}