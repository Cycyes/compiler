#ifndef SYNTAXANALYSER_H
#define SYNTAXANALYSER_H

#include "Common.h"
#include "LexcalAnalyser.h"

typedef string vt; // 终结符
typedef string vn; // 非终结符
typedef vector<string> ProductionRight; // 产生式右部
typedef vector<vector<string>> ProductionRights; // 产生式右部集
typedef pair<vn, ProductionRight> Production; // 产生式
typedef pair<vn, ProductionRights> Productions; // 文法文件中的行产生式

typedef string ForwardSearchStr; // LR1展望串

/*========= LR1项目 =========*/
class LR1Item {
	/*=== Members ===*/
public:
	Production production;
	int dotPos;
	set<ForwardSearchStr> forwardSearchStrSet;

	/*=== Functions ===*/
public:
	LR1Item();
	LR1Item(const Production& p, const int& d = 0);
	LR1Item(const Production& p, const int& d, const set<ForwardSearchStr>& f);
	~LR1Item();

	bool operator== (const LR1Item& item);
	bool operator< (const LR1Item& item) const;
};
/*========= LR1项目 =========*/



/*========= LR1项目集族 =========*/
class LR1ItemSet {
	/*=== Members ===*/
public:
	set<LR1Item> ItemSet;
	map<string, int> next;

	/*=== Functions ===*/
public:
	bool operator== (const LR1ItemSet& I);
};
/*========= LR1项目集族 =========*/



class DFA {
	/*=== Members ===*/
};


/*========= Action表项 =========*/
class ActionItem {
	/*=== Enums ===*/
public:
	enum ActionState {
		A_SHIFT,
		A_REDUCTION,
		A_ACC,
		A_ERROR
	};

	/*=== Members ===*/
public:
	ActionState status;
	int nextState;
	Production production;

	/*=== Functions ===*/
public:
	ActionItem();
	~ActionItem();
};
/*========= Action表项 =========*/



/*========= 语法分析器SyntaxAnalyser =========*/
class SyntaxAnalyser {
	/*=== Consts ===*/
	const string G_FileName = "grammar.txt";
	const string G_SINIT = "@";
	static const char G_VNSTART = '<';
	static const char G_VNEND = '>';
	static const char G_VTSTART = '\'';
	static const char G_VTEND = '\'';
	static const char G_ASSIGNSTART = ':';
	static const char G_ASSIGNEND = '=';
	static const char G_MORE = '|';

	/*=== Members ===*/
private:
	vn S; // 开始标识符
	map<string, vector<vt>> FIRST;
	map<int, LR1ItemSet> stateLR1ItemSetMap;

public:
	set<vn> VN; // 非终结符集
	set<vt> VT; // 终结符集
	map<string, ProductionRights> vnProductionRightsMap; // 文法中的行产生式对应表
	// map<pair<state, string>, state> DFA;
	map<int, vector<ActionItem>> ACTION;
	map<int, vector<int>> GOTO;

	/*=== Functions ===*/
private:

	void CLOSURE(LR1ItemSet& I);
	LR1ItemSet GO(const LR1ItemSet& I, const string& v);
	vector<string> generateNextV(const LR1ItemSet& I);

	void readGrammar();
	void generateFIRST();
	void analyseLR1();
	void generateLR1Table();

public:
	SyntaxAnalyser();
	~SyntaxAnalyser();

	void analyse();
	void show();
};
/*========= 语法分析器SyntaxAnalyser =========*/

#endif 