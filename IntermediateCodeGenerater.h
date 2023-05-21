#ifndef INTERMEDIATECODEGENERATER
#define INTERMEDIATECODEGENERATER

#include "SemanticAnalyser.h"

class IntermediateCodeGenerater {
	/*=== Functions ===*/
private:
	string generateProductionStr(const Production& p);

public:
	IntermediateCodeGenerater(const string& s = "");
	~IntermediateCodeGenerater();

	void analyse(const string& filename = "./output/SyntaxTree.dot");
	void drawSyntaxTree(const string& s = "./output/SyntaxTree.dot", const string& d = "./output/SyntaxTree.png");
	void showIntermediateCode(const string& filename = "./output/IntermediateCode.txt", const bool& isOut = false);

	/*=== Members ===*/
private:
	int lineCnt;

public:
	LexcalAnalyser lexcalAnalyser; // �ʷ�������
	SyntaxAnalyser syntaxAnalyser; // �﷨������
	SemanticAnalyser semanticAnalyser; // ���������
};

#endif