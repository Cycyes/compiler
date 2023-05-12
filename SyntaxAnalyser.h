#ifndef SYNTAXANALYSER_H
#define SYNTAXANALYSER_H

#include "Common.h"
#include "LexcalAnalyser.h"

typedef string vt; // �ս��
typedef string vn; // ���ս��
typedef vector<string> ProductionRight; // ����ʽ�Ҳ�
typedef vector<vector<string>> ProductionRights; // ����ʽ�Ҳ���
typedef pair<vn, ProductionRight> Production; // ����ʽ
typedef pair<vn, ProductionRights> Productions; // �ķ��ļ��е��в���ʽ

typedef string ForwardSearchStr; // LR1չ����

/*========= LR1��Ŀ =========*/
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
/*========= LR1��Ŀ =========*/



/*========= LR1��Ŀ���� =========*/
class LR1ItemSet {
	/*=== Members ===*/
public:
	set<LR1Item> ItemSet;
	map<string, int> next;

	/*=== Functions ===*/
public:
	bool operator== (const LR1ItemSet& I);
};
/*========= LR1��Ŀ���� =========*/



class DFA {
	/*=== Members ===*/
};


/*========= Action���� =========*/
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
/*========= Action���� =========*/



/*========= �﷨������SyntaxAnalyser =========*/
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
	vn S; // ��ʼ��ʶ��
	map<string, vector<vt>> FIRST;
	map<int, LR1ItemSet> stateLR1ItemSetMap;

public:
	set<vn> VN; // ���ս����
	set<vt> VT; // �ս����
	map<string, ProductionRights> vnProductionRightsMap; // �ķ��е��в���ʽ��Ӧ��
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
/*========= �﷨������SyntaxAnalyser =========*/

#endif 