#ifndef OBJECT_CODE
#define OBJECT_CODE

#include "Optimizer.h"

typedef pair<int, bool> Tag;

class MessageTableItem {
	/*=== Static Consts ===*/
public:
	static const int arg1Pos = 0;
	static const int arg2Pos = 1;
	static const int resultPos = 2;
	static const int tagSize = 3;

	/*=== Functions ===*/
public:
	MessageTableItem();
	MessageTableItem(const int& id, const Quaternion& q);
	~MessageTableItem();

	/*=== Members ===*/
public:
	int id;
	Quaternion q;
	Tag tags[tagSize];
};

class ObjectCodeGenerater {
	/*=== Static Consts ===*/
public:
	static const int bufferSize = 1024;

	static const int Type_Jl = 1;
	static const int Type_Jle = 2;
	static const int Type_Jg = 3;
	static const int Type_Jge = 4;
	static const int Type_Jeq = 5;
	static const int Type_Jne = 6;

	static const int Type_Plus = 1;
	static const int Type_Minus = 2;
	static const int Type_And = 3;
	static const int Type_Or = 4;
	static const int Type_Xor = 5;
	static const int Type_Mul = 6;
	static const int Type_Divide = 7;

	static const int ObjectCodeHeadDataSpacePos = 1;
	static const int ObjectCodeHeadStackSpacePos = 2;
	static const int ObjectCodeHeadTempSpacePos = 3;
	static const int ObjectCodeHeadSize = 20;
	const string ObjectCodeHead[ObjectCodeHeadSize] = { 
		".data",
		"data:.space ",
		"stack:.space ",
		"temp:.space ",
		".text",
		"j B0",
		"B1:",
		"nop",
		"j B1",
		"nop",
		"B2:",
		"jal Fmain",
		"nop",
		"break",
		"B0:",
		"addi $gp,$zero,0",
		"addi $fp,$zero,0",
		"addi $sp,$zero,4",
		"j B2",
		"nop"
	};

	const string OP_NOP = "nop";
	const string OP_J = "j";
	const string OP_JAL = "jal";

	const string OP_BREAK = "break";
	const string OP_RET = "ret";
	const string OP_JNZ = "jnz";
	const string OP_JL = "j<";
	const string OP_JLE = "j<=";
	const string OP_JG = "j>";
	const string OP_JGE = "j>=";
	const string OP_JEQ = "j==";
	const string OP_JNE = "j!=";
	const string OP_ASSIGN = ":=";
	const string OP_INDEXASSIGN = "[]=";
	const string OP_ASSIGNINDEX = "=[]";
	const string OP_PLUS = "+";
	const string OP_MINUS = "-";
	const string OP_AND = "&";
	const string OP_OR = "|";
	const string OP_XOR = "^";
	const string OP_MUL = "*";
	const string OP_DIVIDE = "/";


	/*=== Functions ===*/
private:
	void generateJcmp(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item, const int& type);
	void generateCalc(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item, const int& type);

	void generateNop();
	void generateJ(bool& jEnd, const Quaternion& q);
	void generateJal(bool& jEnd, const Quaternion& q);

	void generateBreak(bool& jEnd);
	void generateRet(bool& jEnd);
	void generateJnz(bool& jEnd, const Quaternion& q, const string& reg, const MessageTableItem& item);
	void generateJl(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateJle(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateJg(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateJge(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateJeq(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateJne(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateAssign(const Quaternion& q, const string& reg1, const MessageTableItem& item);
	void generateIndexAssign(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateAssignIndex(const Quaternion& q, const string& reg2, const MessageTableItem& item);
	void generatePlus(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateMinus(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateAnd(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateOr(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateXor(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateMul(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);
	void generateDivide(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item);

	void emit(string code);
	void endBlock();
	void updateVALUE(const Tag& tag, const string& R, const string& V, const bool& flag);
	string getReg(const string& result);

	vector<MessageTableItem> generateMessageTable(const int& blkno);

	void preGenerate();

	string allocateRegGVT(const string& arg, const char& c, const int& i);
	string allocateReg(const string& arg, const string& op, const int& i, const string& regAllocated = "");

public:
	ObjectCodeGenerater(const vector<Quaternion> intermediateCode, const vector<BlockItem>& block, const int& stackSize);
	~ObjectCodeGenerater();

	void generateObjectCode();

	void printObjectCode(const string& filename = "./output/ObjectCode.txt", const bool& isOut = false);

	/*=== Members ===*/
private:
	vector<Quaternion> intermediateCode;
	vector<BlockItem> block;

	map<string, vector<string>> AVALUE;
	map<string, vector<pair<string, int>>> RVALUE;

public:
	int stackBufferSize;
	int dataBufferSize;
	int tempBufferSize;

	vector<string> objectCode;
};

#endif
