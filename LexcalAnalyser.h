#ifndef LEXCALANALYSER_H
#define LEXCALANALYSER_H

#include "Common.h"

class Token {
	/*=== Members ===*/
public:
	string str;
	int code;

	/*=== Functions ===*/
public:
	Token(const string s = "", const int c = -1);
	~Token();

	bool operator == (const Token& t);
	bool operator != (const Token& t);
};

class LexcalAnalyser {
	/*=== Enum ===*/
public:
	enum State {
		LA_INIT,
		LA_COMMENT_LINE,
		LA_COMMENT_SEG,
		LA_OPERATOR,
		LA_NUMBER,
		LA_STR,
		LA_ERROR
	};

	/*=== Consts ===*/
public:
	const set<string> skipCharSet = { " ", "\r", "\t" };

	const set<string> unaryOperatorSet = {
		"+", "-", "*", "/",
		"&", "|", "!", "^",
		">", "<", "=",
		";", ",", "(", ")", "[", "]", "{", "}"
	};

	const set<string> binaryOperatorSet = {
		"<=", ">=", "!=", "==",
		"&&", "||"
	};

	const set<string> reservedWordSet = {
		"int", "void",
		"while", "if", "else", "return"
	};

	const map<string, Token> strTokenMap = {
		{"int", Token("INT",-1)},
		{"void", Token("VOID", -1)},
		{"id", Token("ID", -1)},
		{"(", Token("LP", -1)},
		{")", Token("RP", -1)},
		{"[", Token("LS", -1)},
		{"]", Token("RS", -1)},
		{"{", Token("LB", -1)},
		{"}", Token("RB", -1)},
		{"!", Token("NOT", -1)},
		{"while", Token("WHILE", -1)},
		{"if", Token("IF", -1)},
		{"else", Token("ELSE", -1)},
		{"return", Token("RETURN", -1)},
		{"=", Token("ASSIGN", -1)},
		{"+", Token("OP1", 0)},
		{"-", Token("OP1", 1)},
		{"&", Token("OP1", 2)},
		{"|", Token("OP1", 3)},
		{"^", Token("OP1", 4)},
		{"*", Token("OP2", 0)},
		{"/", Token("OP2", 1)},
		{"<", Token("RELOP", 0)},
		{"<=", Token("RELOP", 1)},
		{">", Token("RELOP", 2)},
		{">=", Token("RELOP", 3)},
		{"==", Token("RELOP", 4)},
		{"!=", Token("RELOP", 5)},
		{"||", Token("OR", -1)},
		{"&&", Token("AND", -1)},
		{";", Token("DEL", -1)},
		{",", Token("SEP", -1)},
		{"\n", Token("NL", -1)}
	};

	/*=== Functions ===*/
public:
	LexcalAnalyser();
	~LexcalAnalyser();

	void setSourceCode(string s);
	Token getNextToken();

	/*=== Members ===*/
private:
	string sourceCode; // 源代码
	unsigned int sourceCodePos; // 词法分析器当前分析位置
	State state; // 词法分析器状态
	int varCnt;  // 变量个数
	int lineCnt; // 当前行数

public:
	map<int, string> varTable;
};

#endif