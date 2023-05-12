#ifndef SEMANTICANALYSER_H
#define SEMANTICANALYSER_H

#include "SyntaxAnalyser.h"

/*========= 预先声明 =========*/

class SymbolTable; //

/*========= 预先声明 =========*/



/*========= 四元项 =========*/

class Quaternion {
	/*=== Functions ===*/
public:
	Quaternion(const string& op = "", const string& arg1 = "", const string& arg2 = "", const string& result = "");
	~Quaternion();

	/*=== Members ===*/
public:
	string op;
	string arg1;
	string arg2;
	string result;
};

/*========= 四元项 =========*/



/*========= 符号表项STableItem =========*/

class SymbolTableItem {
	/*=== Static Consts ===*/
	static const int S_INIT = -1;

	/*=== Enums ===*/
public:
	enum DataType {
		S_INT,
		S_VOID,
		S_DINIT
	};

	enum SymbolType {
		S_VAR,
		S_FUNC,
		S_ARRAY,
		S_SINIT
	};

	/*=== Functions===*/
public:
	SymbolTableItem(const int& id = S_INIT, const int& o = S_INIT, const DataType& d = S_DINIT, const SymbolType& s = S_SINIT);
	~SymbolTableItem();

	void setArrayShape(const vector<int>& aS);
	void setFuncTablePointer(SymbolTable* p);

	/*=== Members ===*/
public:
	int id;
	int offset;
	DataType dataType;
	SymbolType symbolType;
	
	// arrayShape针对ARRAY，FuncTable针对FUNC
	vector<int> arrayShape;
	SymbolTable* FuncTablePointer;
};

/*========= 符号表项STableItem =========*/



/*========= 符号表STable =========*/

class SymbolTable {
	/*=== Members ===*/
public:
	SymbolTable* prev;
	SymbolTable* next;

	SymbolTable* parent;

public:
	int width;
	vector<SymbolTableItem> table;

	/*=== Functions ===*/
public:
	SymbolTable();
	~SymbolTable();

	void clear();
	void enter(const int& id, const int& o, const SymbolTableItem::DataType& d, const SymbolTableItem::SymbolType& s);
	void setIdArrayShape(const int& id, const vector<int>& aS);
	void setIdFuncTablePointer(const int& id, SymbolTable* table);
};

/*========= 符号表STable =========*/



/*========= 语法树节点 =========*/

typedef pair<string, int> Content;

class SyntaxTreeNode {
	/*=== Functions ===*/
public:
	SyntaxTreeNode();
	~SyntaxTreeNode();

	void clear();

	/*=== Members ===*/
public:
	// 拓扑结构
	SyntaxTreeNode* parent;
	vector<SyntaxTreeNode*> children;

public:
	int level;
	Content content;
	SymbolTableItem::DataType dataType;
	SymbolTableItem::SymbolType symbolType;

	int n;
	int width;

	vector<int> arrayShpae;
	vector<string> params;
	string place;

	int quad;
	int trueList;
	int falseList;

	int x;
	int y;

};

/*========= 语法树节点 =========*/



/*========= 语义分析器SemanticAnalyser =========*/

class SemanticAnalyser {
	/*=== Static Consts ===*/
	static const int TmpCntInit = -1;
	const string ProgramToken = "<PROGRAM>::=<M><ASSERTIONS>";
	const string MToken = "<M>::='epsilon'";
	const string ASSERTIONSToken0 = "<ASSERTIONS>::=<ASSERTION>";
	const string ASSERTIONSToken1 = "<ASSERTIONS>::=<ASSERTION><ASSERTIONS>";
	const string ASSERTIONToken0 = "<ASSERTION>::='INT''ID'<ASSERTIONTYPE>'DEL'";
	const string ASSERTIONToken1 = "<ASSERTION>::=<FUNCASSERTION><SENBLOCK>";
	const string ASSERTIONTYPEToken0 = "<ASSERTIONTYPE>::='epsilon'";
	const string ASSERTIONTYPEToken1 = "<ASSERTIONTYPE>::=<ARRAYASSERTION>";
	const string FUNCASSERTIONToken0 = "<FUNCASSERTION>::='VOID''ID'<M>'LP'<FORMALPARAM>'RP'";
	const string FUNCASSERTIONToken1 = "<FUNCASSERTION>::='INT''ID'<M>'LP'<FORMALPARAM>'RP'";
	const string ARRAYASSERTIONToken0 = "<ARRAYASSERTION>::='LS''NUM''RS'";
	const string ARRAYASSERTIONToken1 = "<ARRAYASSERTION>::='LS''NUM''RS'<ARRAYASSERTION>";
	const string FORMALPARAMToken0 = "<FORMALPARAM>::=<FORMALPARAMLIST>";
	const string FORMALPARAMToken1 = "<FORMALPARAM>::='VOID'";
	const string FORMALPARAMToken2 = "<FORMALPARAM>::='epsilon'";
	const string FORMALPARAMLISTToken0 = "<FORMALPARAMLIST>::='INT''ID'";
	const string FORMALPARAMLISTToken1 = "<FORMALPARAMLIST>::='INT''ID''SEP'<FORMALPARAMLIST>";
	const string SENBLOCKToken = "<SENBLOCK>::='LB'<INNERASSERTION><SENSEQ>'RB'";
	const string INNERASSERTIONToken0 = "<INNERASSERTION>::=<INNERVARIDEF>'DEL'<INNERASSERTION>";
	const string INNERASSERTIONToken1 = "<INNERASSERTION>::='epsilon'";
	const string INNERVARIDEFToken0 = "<INNERVARIDEF>::='INT''ID'";
	const string INNERVARIDEFToken1 = "<INNERVARIDEF>::='INT''ID'<ARRAYASSERTION>";
	const string SENSEQToken0 = "<SENSEQ>::=<SENTENCE>";
	const string SENSEQToken1 = "<SENSEQ>::=<SENTENCE><SENSEQ>";
	const string SENTENCEToken0 = "<SENTENCE>::=<IFSEN>";
	const string SENTENCEToken1 = "<SENTENCE>::=<WHILESEN>";
	const string SENTENCEToken2 = "<SENTENCE>::=<RETURNSEN>'DEL'";
	const string SENTENCEToken3 = "<SENTENCE>::=<ASSIGNMENT>'DEL'";
	const string ASSIGNMENTToken0 = "<ASSIGNMENT>::='ID''ASSIGN'<EXPRESSION>";
	const string ASSIGNMENTToken1 = "<ASSIGNMENT>::=<ARRAY>'ASSIGN'<EXPRESSION>";
	const string RETURNSENToken0 = "<RETURNSEN>::='RETURN'<EXPRESSION>";
	const string RETURNSENToken1 = "<RETURNSEN>::='RETURN'";
	const string WHILESENToken = "<WHILESEN>::=<B>'WHILE''LP'<CTRL>'RP'<T><SENBLOCK>";
	const string BToken = "<B>::='epsilon'";
	const string IFSENToken0 = "<IFSEN>::='IF''LP'<CTRL>'RP'<T><SENBLOCK>";
	const string IFSENToken1 = "<IFSEN>::='IF''LP'<CTRL>'RP'<T><SENBLOCK>'ELSE'<N><SENBLOCK>";
	const string CTRLToken = "<CTRL>::=<EXPRESSION>";
	const string TToken = "<T>::='epsilon'";
	const string NToken = "<N>::='epsilon'";
	const string EXPRESSIONToken0 = "<EXPRESSION>::=<BOOLAND>";
	const string EXPRESSIONToken1 = "<EXPRESSION>::=<BOOLAND>'OR'<EXPRESSION>";
	const string BOOLANDToken0 = "<BOOLAND>::=<BOOLNOT>";
	const string BOOLANDToken1 = "<BOOLAND>::=<BOOLNOT>'AND'<BOOLAND>";
	const string BOOLNOTToken0 = "<BOOLNOT>::=<COMP>";
	const string BOOLNOTToken1 = "<BOOLNOT>::='NOT'<COMP>";
	const string COMPToken0 = "<COMP>::=<PLUSEX>";
	const string COMPToken1 = "<COMP>::=<PLUSEX>'RELOP'<COMP>";
	const string PLUSEXToken0 = "<PLUSEX>::=<TERM>";
	const string PLUSEXToken1 = "<PLUSEX>::=<TERM>'OP1'<PLUSEX>";
	const string TERMToken0 = "<TERM>::=<FACTOR>";
	const string TERMToken1 = "<TERM>::=<FACTOR>'OP2'<TERM>";
	const string FACTORToken0 = "<FACTOR>::='NUM'";
	const string FACTORToken1 = "<FACTOR>::='LP'<EXPRESSION>'RP'";
	const string FACTORToken2 = "<FACTOR>::='ID'";
	const string FACTORToken3 = "<FACTOR>::=<ARRAY>";
	const string FACTORToken4 = "<FACTOR>::='ID'<CALL>";
	const string FACTORToken5 = "<FACTOR>::='LP'<ASSIGNMENT>'RP'";
	const string CALLToken = "<CALL>::='LP'<ACTUALPARAM>'RP'";
	const string ARRAYToken0 = "<ARRAY>::='ID''LS'<EXPRESSION>'RS'";
	const string ARRAYToken1 = "<ARRAY>::=<ARRAY>'LS'<EXPRESSION>'RS'";
	const string ACTUALPARAMToken0 = "<ACTUALPARAM>::=<ACTUALPARAMLIST>";
	const string ACTUALPARAMToken1 = "<ACTUALPARAM>::='epsilon'";
	const string ACTUALPARAMLISTToken0 = "<ACTUALPARAMLIST>::=<EXPRESSION>";
	const string ACTUALPARAMLISTToken1 = "<ACTUALPARAMLIST>::=<EXPRESSION>'SEP'<ACTUALPARAMLIST>";

	/*=== Functions ===*/
private:
	string newtemp();
	string lookup(int id);
	SymbolTableItem* find(int id);
	int nextstat();
	void emit(string op, string arg1, string arg2, string restult);

	void Program();
	void M(SyntaxTreeNode* root);
	void ASSERTIONS0();
	void ASSERTIONS1();
	void ASSERTION0(SyntaxTreeNode* root);
	void ASSERTION1(SyntaxTreeNode* root);
	void ASSERTIONTYPE0(SyntaxTreeNode* root);
	void ASSERTIONTYPE1(SyntaxTreeNode* root);
	void FUNCASSERTION0(SyntaxTreeNode* root);
	void FUNCASSERTION1(SyntaxTreeNode* root);
	void ARRAYASSERTION0(SyntaxTreeNode* root);
	void ARRAYASSERTION1(SyntaxTreeNode* root);
	void FORMALPARAM0(SyntaxTreeNode* root);
	void FORMALPARAM1(SyntaxTreeNode* root);
	void FORMALPARAM2(SyntaxTreeNode* root);
	void FORMALPARAMLIST0(SyntaxTreeNode* root);
	void FORMALPARAMLIST1(SyntaxTreeNode* root);
	void SENBLOCK();
	void INNERASSERTION0();
	void INNERASSERTION1();
	void INNERVARIDEF0(SyntaxTreeNode* root);
	void INNERVARIDEF1(SyntaxTreeNode* root);
	void SENSEQ0();
	void SENSEQ1();
	void SENTENCE0();
	void SENTENCE1();
	void SENTENCE2();
	void SENTENCE3();
	void ASSIGNMENT0(SyntaxTreeNode* root, map<int, string>& nameTable);
	void ASSIGNMENT1(SyntaxTreeNode* root, map<int, string>& nameTable);
	void RETURNSEN0(SyntaxTreeNode* root);
	void RETURNSEN1();
	void WHILESEN(SyntaxTreeNode* root);
	void B(SyntaxTreeNode* root);
	void IFSEN0(SyntaxTreeNode* root);
	void IFSEN1(SyntaxTreeNode* root);
	void CTRL(SyntaxTreeNode* root);
	void T(SyntaxTreeNode* root);
	void N(SyntaxTreeNode* root);
	void EXPRESSION0(SyntaxTreeNode* root);
	void EXPRESSION1(SyntaxTreeNode* root);
	void BOOLAND0(SyntaxTreeNode* root);
	void BOOLAND1(SyntaxTreeNode* root);
	void BOOLNOT0(SyntaxTreeNode* root);
	void BOOLNOT1(SyntaxTreeNode* root);
	void COMP0(SyntaxTreeNode* root);
	void COMP1(SyntaxTreeNode* root);
	void PLUSEX0(SyntaxTreeNode* root);
	void PLUSEX1(SyntaxTreeNode* root);
	void TERM0(SyntaxTreeNode* root);
	void TERM1(SyntaxTreeNode* root);
	void FACTOR0(SyntaxTreeNode* root);
	void FACTOR1(SyntaxTreeNode* root);
	void FACTOR2(SyntaxTreeNode* root, map<int, string>& nameTable);
	void FACTOR3(SyntaxTreeNode* root, map<int, string>& nameTable);
	void FACTOR4(SyntaxTreeNode* root, map<int, string>& nameTable);
	void FACTOR5(SyntaxTreeNode* root);
	void CALL(SyntaxTreeNode* root);
	void ARRAY0(SyntaxTreeNode* root, map<int, string>& nameTable);
	void ARRAY1(SyntaxTreeNode* root);
	void ACTUALPARAM0(SyntaxTreeNode* root);
	void ACTUALPARAM1();
	void ACTUALPARAMLIST0(SyntaxTreeNode* root);
	void ACTUALPARAMLIST1(SyntaxTreeNode* root);


public:
	SemanticAnalyser();
	~SemanticAnalyser();

	void analyse(string token, SyntaxTreeNode* root, map<int, string> nameTable);

	/*=== Members ===*/
private:
	int tmpCnt;
	vector<SymbolTable*> symbolTableStack;
	vector<int> offsetStack;

public:
	SymbolTable* lastSymbolTable;
	SymbolTable* gSymbolTable;
	vector<Quaternion> intermediateCode;
};

/*========= 语义分析器SemanticAnalyser =========*/

#endif