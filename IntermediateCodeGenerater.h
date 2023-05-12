#ifndef INTERMEDIATECODEGENERATER
#define INTERMEDIATECODEGENERATER

#include "SemanticAnalyser.h"

class syntaxAnalysis
{
private:
	int retcode;
	void _genTreeLevel(treeNode* nownode, int nowlevel);
	void _showTree2(treeNode* nownode);
public:
	SyntaxAnalyser G;
	LexcalAnalyser L;
	semanticAnalysis S;
	vector<vector<vector<string>>> history;
	treeNode* reductionTreeRoot = NULL;
	int maxTreeLevel = 0;
	int leafNum = 0;
	~syntaxAnalysis();
	void initializeLR1();
	void getInput(string input);
	void analysis();
	void showHistory();
	void showTree();
	void showTree2();
};

#endif