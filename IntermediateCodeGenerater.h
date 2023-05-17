#ifndef INTERMEDIATECODEGENERATER
#define INTERMEDIATECODEGENERATER

#include "SemanticAnalyser.h"

class SyntaxTreeInfo {
	/*=== Functions ===*/
public:
	SyntaxTreeInfo();
	~SyntaxTreeInfo();

	void updateInfo(SyntaxTreeNode* n, const int& l);

	/*=== Members ===*/
public:
	int maxTreeLevel;
	int leafNum;

	SyntaxTreeNode* root;
};

class IntermediateCodeGenerater {
	/*=== Functions ===*/
private:
	string generateProductionStr(const Production& p);

	void generateTreeLevel(SyntaxTreeNode* n, const int& l);

public:
	IntermediateCodeGenerater(const string& s = "");
	~IntermediateCodeGenerater();

	void analyse(const string& filename = "SyntaxTree.dot");
	void drawSyntaxTree(const string& s = "SyntaxTree.dot", const string& d = "SyntaxTree.png");
	void showIntermediateCode(const string& filename = "IntermediateCode.txt");

	/*=== Members ===*/
private:
	int lineCnt;

	SyntaxTreeInfo syntaxTreeInfo; // 语法树信息

public:
	LexcalAnalyser lexcalAnalyser; // 词法分析器
	SyntaxAnalyser syntaxAnalyser; // 语法分析器
	SemanticAnalyser semanticAnalyser; // 语义分析器

};

#endif