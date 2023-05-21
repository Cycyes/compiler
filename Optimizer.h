#ifndef OPTIMIZER
#define OPTIMIZER

#include "IntermediateCodeGenerater.h"

/*========== Block项 ==========*/
class BlockItem {
	/*=== Functions ===*/
public:
	void init(const int& b);

	/*=== Members ===*/
public:
	int begin;
	int end;
	vector<string> waitVar;
	vector<string> uselessVar;
	vector<string> activeVar;
};
/*========== Block项 ==========*/



/*========== DAG项 ==========*/

class DAGItem {
	/*=== Static Consts ===*/
public:
	static const int TopoStructInit = -1;
	static const int MaxChildrenNum = 3;

	/*=== Functions ===*/
public:
	DAGItem();
	DAGItem(const Quaternion& q);
	DAGItem(const string& value, const bool& isLeaf);
	DAGItem(const string& op, const int& child0, const int& child1, const int& child2);
	~DAGItem();

	bool operator == (const DAGItem& d);

	/*=== Members ===*/
public:
	bool flag;
	bool isLeaf;
	bool isLeft;

	// 拓扑结构
	int parent;
	int children[MaxChildrenNum];

	string value;
	string op;

	Quaternion quaternion;

	vector<string> labelList;

};

/*========== DAG项 ==========*/



/*========== Optimizer ==========*/

class Optimizer {
	/*=== Static Consts ===*/
public:
	static const int DAG_STATE_END = -1;
	static const int DAG_STATE_PrepareNode = 1;
	static const int DAG_STATE_CombineKnown = 2;
	static const int DAG_STATE_CombineKnown1 = 21;
	static const int DAG_STATE_CombineKnown2 = 22;
	static const int DAG_STATE_CombineKnown3 = 23;
	static const int DAG_STATE_CombineKnown4 = 24;
	static const int DAG_STATE_FindCommonExpression = 3;
	static const int DAG_STATE_FindCommonExpression1 = 31;
	static const int DAG_STATE_FindCommonExpression2 = 32;
	static const int DAG_STATE_RemoveUselessAssignments = 4;

	/*=== Functions ===*/
private:
	bool judgeDAGNodeIsNum(const int& no, const vector<DAGItem>& DAG);

	int operateBopC(const string& op, const int& Bno, const int& Cno, const vector<DAGItem>& DAG);

	void findOrCreateDAGNodeByValue(int& no, bool& flag, vector<DAGItem>& DAG, const string& v);
	void findOrCreateDAGNodeByChild(int& no, const string& op, vector<DAGItem>& DAG, const int& Bno, const int& Cno = DAGItem::TopoStructInit);
	void deleteNewDAGNode(const int& no, const bool& flag, vector<DAGItem>& DAG);

	string newtemp();
	void utilizeChildren(vector<DAGItem>& DAG, int now);

	string getChildValue(const int& i, const int& p, const vector<DAGItem>& DAG);

	void preGenerateBlock();
	void generateBlock();
	vector<DAGItem> generateDAG(const int& blkno);

public:
	Optimizer(const map<int, string>& varTable, SymbolTable* gSymbolTable, const vector<Quaternion>& intermediateCode);
	~Optimizer();

	void optimize();
	void analyse();

	/*=== Members ===*/
private:
	int tmpCnt = 0;

	map<int, string> varTable; // 变量id转名称
	map<int, string> labelMap; // 将四元式中的数字转化为目标代码的符号
	SymbolTable* gSymbolTable;

	vector<Quaternion> initCode;
	vector<BlockItem> initBlock;

	vector<vector<DAGItem>> DAGs;

public:
	int optimizeCnt;
	double optimizeRate;

	vector<Quaternion> intermediateCode;
	vector<BlockItem> block;
};

/*========== Optimizer ==========*/

#endif
