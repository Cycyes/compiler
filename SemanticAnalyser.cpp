#include "SemanticAnalyser.h"

/*============================== 符号表项SymbolTableItem ==============================*/

Quaternion::Quaternion(const string& op, const string& arg1, const string& arg2, const string& result) {
	this->op = op;
	this->arg1 = arg1;
	this->arg2 = arg2;
	this->result = result;
}

Quaternion::~Quaternion() {

}

/*============================== 符号表项SymbolTableItem ==============================*/



/*============================== 符号表项SymbolTableItem ==============================*/

SymbolTableItem::SymbolTableItem(const int& id, const int& o, const DataType& d, const SymbolType& s) {
	this->id = id;
	this->offset = o;
	this->dataType = d;
	this->symbolType = s;
	
	FuncTablePointer = NULL;
}

SymbolTableItem::~SymbolTableItem() {

}

void SymbolTableItem::setArrayShape(const vector<int>& aS) {
	this->arrayShape = aS;
}

void SymbolTableItem::setFuncTablePointer(SymbolTable* p) {
	this->FuncTablePointer = p;
}

/*============================== 符号表项SymbolTableItem ==============================*/



/*============================== 符号表项SymbolTable ==============================*/

SymbolTable::SymbolTable() {
	prev = NULL;
	next = NULL;
	parent = NULL;
	width = 0;
	table.clear();
}

SymbolTable::~SymbolTable() {

}

void SymbolTable::clear() {
	prev = NULL;
	next = NULL;
	width = 0;
	table.clear();
}

void SymbolTable::enter(const int& id, const int& o, const SymbolTableItem::DataType& d, const SymbolTableItem::SymbolType& s) {
	SymbolTableItem item(id, o, d, s);

	// 检查id是否重定义
	for (int i = 0; i < table.size(); i++) {
		if (table[i].id == id) {
			cerr << "ERROR: 语义分析器: 符号表id重定义: " << to_string(id) << "重定义" << endl;
			exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE);
		}
	}

	// 加入表
	table.push_back(item);
}

void SymbolTable::setIdArrayShape(const int& id, const vector<int>& aS) {
	for (int i = 0; i < table.size(); i++) {
		if (table[i].id == id && (table[i].symbolType == SymbolTableItem::S_ARRAY || table[i].symbolType == SymbolTableItem::S_FUNC)) {
			table[i].setArrayShape(aS);
			break;
		}
	}
}

void SymbolTable::setIdFuncTablePointer(const int& id, SymbolTable* p) {
	for (int i = 0; i < table.size(); i++) {
		if (table[i].id == id && table[i].symbolType == SymbolTableItem::S_FUNC) {
			table[i].FuncTablePointer = p;
			break;
		}
	}

}

/*============================== 符号表项SymbolTable ==============================*/



/*============================== 语义树节点 ==============================*/

SyntaxTreeNode::SyntaxTreeNode() {
	
	parent = NULL;

	level = 0;
	dataType = SymbolTableItem::S_INT;
	symbolType = SymbolTableItem::S_VAR;

	n = 0;
	width = 0;

	x = -1;
	y = -1;

	clear();
}

SyntaxTreeNode::~SyntaxTreeNode() {

}

void SyntaxTreeNode::clear() {
	level = -1;
	parent = NULL;
	children.clear();
	content = pair<string, int>("", -1);
}

/*============================== 语义树节点 ==============================*/



/*============================== 语义分析器 ==============================*/

SemanticAnalyser::SemanticAnalyser() {
	tmpCnt = TmpCntInit;
	lastSymbolTable = NULL;
}

SemanticAnalyser::~SemanticAnalyser() {

}

//
string SemanticAnalyser::newtemp() {
	string tmp = "V" + to_string(offsetStack.back());
	symbolTableStack.back()->enter(tmpCnt--, offsetStack.back(), SymbolTableItem::S_INT, SymbolTableItem::S_VAR);
	offsetStack.back() += 4;
	emit("+", "$sp", to_string(4), "$sp");
	return tmp;
}

/*
* Function Name:        lookup
* Function Description: 检查是否存在id对应的表项
*/
string SemanticAnalyser::lookup(int id) {
	SymbolTable* tp = symbolTableStack.back();
	int offset;
	while (tp) {
		for (int i = 0; i < tp->table.size(); i++) {
			if (tp->table[i].id == id) {
				offset = tp->table[i].offset;
				if (tp->table[i].symbolType == SymbolTableItem::S_VAR || tp->table[i].symbolType == SymbolTableItem::S_ARRAY) {
					if (tp->parent) {
						return "V" + to_string(offset);
					}
					else {
						return "G" + to_string(offset);
					}
				}
			}
		}
		tp = tp->parent;
	}
	return "";
}

/*
* Function Name:        find
* Function Description: 返回id对应的TableItem
*/
SymbolTableItem* SemanticAnalyser::find(int id) {
	SymbolTable* tp = symbolTableStack.back();
	int offset = -1;
	while (tp) {
		for (int i = 0; i < tp->table.size(); i++) {
			if (tp->table[i].id == id) {
				return &(tp->table[i]);
			}
		}
		tp = tp->parent;
	}
	return NULL;
}

int SemanticAnalyser::nextstat() {
	return intermediateCode.size();
}

void SemanticAnalyser::emit(string op, string arg1, string arg2, string result) {
	Quaternion q(op, arg1, arg2, result);
	intermediateCode.push_back(q);
}

void SemanticAnalyser::Program() {
	gSymbolTable = symbolTableStack.back();
	symbolTableStack.pop_back();
	gSymbolTable->width = offsetStack.back();
	offsetStack.pop_back();
}

void SemanticAnalyser::M(SyntaxTreeNode* root) {
	root->quad = nextstat(); // 计算quad赋值给根节点
	
	// 动态申请空间-符号表
	SymbolTable* stp = new(nothrow) SymbolTable;
	if (!stp) {
		cerr << "ERROR: 申请符号表失败" << endl;
		exit(ERROR_NEW);
	}

	// 维护双向链表
	if (lastSymbolTable != NULL) {
		lastSymbolTable->next = stp;
		stp->prev = lastSymbolTable;
	}

	// 维护parent
	if (!symbolTableStack.empty()) {
		stp->parent = symbolTableStack.back();
	}

	// 更新lastTable, SymbolTableStack, offsetStack
	lastSymbolTable = stp;
	symbolTableStack.push_back(stp);
	offsetStack.push_back(0);
}

void SemanticAnalyser::ASSERTIONS0() {

}

void SemanticAnalyser::ASSERTIONS1() {

}

/*
* 
* ASSERTION变量
*/
void SemanticAnalyser::ASSERTION0(SyntaxTreeNode* root) {
	// 更新root节点内容
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = root->children[2]->symbolType;
	root->n = root->children[2]->n;
	root->width = 4 * root->n;

	// 在当前符号表中插入
	symbolTableStack.back()->enter(root->children[1]->content.second, offsetStack.back(), root->dataType, root->symbolType);
	
	if (root->symbolType == SymbolTableItem::S_ARRAY) {
		root->arrayShpae = root->children[2]->arrayShpae;
		symbolTableStack.back()->setIdArrayShape(root->children[1]->content.second, root->arrayShpae);
	}

	// 跟新offsetStack
	offsetStack.back() += root->width;
}

/*
*
* ASSERTION函数
*/
void SemanticAnalyser::ASSERTION1(SyntaxTreeNode* root) {
	// 更新root节点内容
	root->dataType = root->children[0]->dataType;
	root->symbolType = root->children[0]->symbolType;
	root->n = root->children[0]->n;
	root->width = root->children[0]->width;

	//
	SymbolTable* stp = symbolTableStack.back();
	symbolTableStack.pop_back();
	stp->width = stp->table.empty() ? 0 : offsetStack.back() - stp->table[0].offset;
	offsetStack.pop_back();
}

void SemanticAnalyser::ASSERTIONTYPE0(SyntaxTreeNode* root) {
	root->symbolType = SymbolTableItem::S_VAR;
	root->n = 1;
}

void SemanticAnalyser::ASSERTIONTYPE1(SyntaxTreeNode* root) {
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->n = root->children[0]->n;
	root->arrayShpae = root->children[0]->arrayShpae;
	reverse(root->arrayShpae.begin(), root->arrayShpae.end());
}

void SemanticAnalyser::FUNCASSERTION0(SyntaxTreeNode* root) {
	root->dataType = SymbolTableItem::S_VOID;
	root->symbolType = SymbolTableItem::S_FUNC;
	root->n = 1;
	root->width = -1 * root->n;

	SymbolTable* stp = symbolTableStack.back();
	symbolTableStack.pop_back();
	int offset = offsetStack.back();
	offsetStack.pop_back();

	symbolTableStack.back()->enter(root->children[1]->content.second, root->children[2]->quad, root->dataType, root->symbolType);
	offsetStack.back() += 0;
	symbolTableStack.back()->setIdFuncTablePointer(root->children[1]->content.second, stp);
	symbolTableStack.back()->setIdArrayShape(root->children[1]->content.second, root->children[4]->arrayShpae);

	symbolTableStack.push_back(stp);
	offsetStack.push_back(offset);
}

void SemanticAnalyser::FUNCASSERTION1(SyntaxTreeNode* root) {
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_FUNC;
	root->n = 1;
	root->width = -1 * root->n;

	SymbolTable* stp = symbolTableStack.back();
	symbolTableStack.pop_back();
	int offset = offsetStack.back();
	offsetStack.pop_back();

	symbolTableStack.back()->enter(root->children[1]->content.second, root->children[2]->quad, root->dataType, root->symbolType);
	offsetStack.back() += 0;
	symbolTableStack.back()->setIdFuncTablePointer(root->children[1]->content.second, stp);
	symbolTableStack.back()->setIdArrayShape(root->children[1]->content.second, root->children[4]->arrayShpae);

	symbolTableStack.push_back(stp);
	offsetStack.push_back(offset);
}

void SemanticAnalyser::ARRAYASSERTION0(SyntaxTreeNode* root) {
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->n = root->children[1]->content.second;
	root->arrayShpae.push_back(root->children[1]->content.second);
}

void SemanticAnalyser::ARRAYASSERTION1(SyntaxTreeNode* root) {
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->n = root->children[1]->content.second * root->children[3]->n;
	root->arrayShpae = root->children[3]->arrayShpae;
	root->arrayShpae.push_back(root->children[1]->content.second);
}

void SemanticAnalyser::FORMALPARAM0(SyntaxTreeNode* root) {
	root->arrayShpae = root->children[0]->arrayShpae;
}

void SemanticAnalyser::FORMALPARAM1(SyntaxTreeNode* root) {
	root->arrayShpae.push_back(0);
}

void SemanticAnalyser::FORMALPARAM2(SyntaxTreeNode* root) {
	root->arrayShpae.push_back(0);
}

void SemanticAnalyser::FORMALPARAMLIST0(SyntaxTreeNode* root) {
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_VAR;
	root->n = 1;
	root->width = 4 * root->n;
	root->arrayShpae.push_back(1);
	symbolTableStack.back()->enter(root->children[1]->content.second, offsetStack.back(), root->dataType, root->symbolType);
	offsetStack.back() += root->width;
}

void SemanticAnalyser::FORMALPARAMLIST1(SyntaxTreeNode* root) {
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_VAR;
	root->n = 1;
	root->width = 4 * root->n;
	root->arrayShpae = root->children[3]->arrayShpae;
	root->arrayShpae[0] += 1;
	symbolTableStack.back()->enter(root->children[1]->content.second, offsetStack.back(), root->dataType, root->symbolType);
	offsetStack.back() += root->width;
}

void SemanticAnalyser::SENBLOCK() {

}

void SemanticAnalyser::INNERASSERTION0() {

}

void SemanticAnalyser::INNERASSERTION1() {

}

void SemanticAnalyser::INNERVARIDEF0(SyntaxTreeNode* root) {
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_VAR;
	root->n = 1;
	root->width = 4 * root->n;
	symbolTableStack.back()->enter(root->children[1]->content.second, offsetStack.back(), root->dataType, root->symbolType);
	offsetStack.back() += root->width;

	emit("+", "$sp", to_string(root->width), "$sp");
}

void SemanticAnalyser::INNERVARIDEF1(SyntaxTreeNode* root) {
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->n = root->children[2]->n;
	root->width = 4 * root->n;
	root->arrayShpae = root->children[2]->arrayShpae;
	reverse(root->arrayShpae.begin(), root->arrayShpae.end());
	symbolTableStack.back()->enter(root->children[1]->content.second, offsetStack.back(), root->dataType, root->symbolType);
	symbolTableStack.back()->setIdArrayShape(root->children[1]->content.second, root->arrayShpae);
	// symbolTableStack.back()->enterarrayShpae(root->children[1]->content.second, root->arrayShpae);
	offsetStack.back() += root->width;
	emit("+", "$sp", to_string(root->width), "$sp");
}

void SemanticAnalyser::SENSEQ0() {

}

void SemanticAnalyser::SENSEQ1() {

}

void SemanticAnalyser::SENTENCE0() {

}

void SemanticAnalyser::SENTENCE1() {

}

void SemanticAnalyser::SENTENCE2() {

}

void SemanticAnalyser::SENTENCE3() {

}

void SemanticAnalyser::ASSIGNMENT0(SyntaxTreeNode* root, map<int, string>& nameTable) {
	string p = lookup(root->children[0]->content.second);
	if (p == "")
	{
		//cerr << "ERROR: " << root->children[0]->content.second << "is undefineded\n";
		//exit(-1);
		throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
	}
	else
	{
		emit(":=", root->children[2]->place, "", p);
		root->place = newtemp();
		emit(":=", root->children[2]->place, "", root->place);
	}
}

void SemanticAnalyser::ASSIGNMENT1(SyntaxTreeNode* root, map<int, string>& nameTable) {
	if (root->children[0]->arrayShpae.size() != 1)
	{
		//cerr << "ERROR: 数组索引不完整\n";
		//exit(-1);
		throw string("ERROR: 语义分析器错误:遇到不完整的数组索引\n");
	}
	string p = lookup(root->children[0]->content.second);
	if (p == "")
	{
		//cerr << "ERROR: " << root->children[0]->content.second << "is undefineded\n";
		//exit(-1);
		throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
	}
	else
	{
		emit("[]=", root->children[2]->place, root->children[0]->place, p);
		root->place = newtemp();
		emit(":=", root->children[2]->place, "", root->place);
	}
}

void SemanticAnalyser::RETURNSEN0(SyntaxTreeNode* root) {
	emit(":=", root->children[1]->place, "", "$v0");
	emit("ret", "", "", "");
}

void SemanticAnalyser::RETURNSEN1() {
	emit(":=", to_string(0), "", "$v0");
	emit("ret", "", "", "");
}

void SemanticAnalyser::WHILESEN(SyntaxTreeNode* root) {
	SymbolTable* t = symbolTableStack.back();
	symbolTableStack.pop_back();
	t->width = t->table.empty() ? 0 : offsetStack.back() - t->table[0].offset;
	offsetStack.pop_back();

	emit("-", "$sp", to_string(t->width), "$sp");

	emit("j", "", "", to_string(root->children[0]->quad));
	intermediateCode[root->children[3]->trueList].result = to_string(root->children[5]->quad);
	intermediateCode[root->children[3]->falseList].result = to_string(nextstat());
}

void SemanticAnalyser::B(SyntaxTreeNode* root) {
	root->quad = nextstat();
}

void SemanticAnalyser::IFSEN0(SyntaxTreeNode* root) {
	SymbolTable* t = symbolTableStack.back();
	symbolTableStack.pop_back();
	t->width = t->table.empty() ? 0 : offsetStack.back() - t->table[0].offset;
	offsetStack.pop_back();

	emit("-", "$sp", to_string(t->width), "$sp");

	intermediateCode[root->children[2]->trueList].result = to_string(root->children[4]->quad);
	intermediateCode[root->children[2]->falseList].result = to_string(nextstat());
}

void SemanticAnalyser::IFSEN1(SyntaxTreeNode* root) {
	SymbolTable* t = symbolTableStack.back();
	symbolTableStack.pop_back();
	t->width = t->table.empty() ? 0 : offsetStack.back() - t->table[0].offset;
	offsetStack.pop_back();

	emit("-", "$sp", to_string(t->width), "$sp");

	intermediateCode[root->children[2]->trueList].result = to_string(root->children[4]->quad);
	intermediateCode[root->children[2]->falseList].result = to_string(root->children[7]->quad);
	intermediateCode[root->children[7]->trueList].result = to_string(nextstat());
}

void SemanticAnalyser::CTRL(SyntaxTreeNode* root) {
	root->trueList = nextstat();
	emit("jnz", root->children[0]->place, "", to_string(0));
	root->falseList = nextstat();
	emit("j", "", "", to_string(0));
}

void SemanticAnalyser::T(SyntaxTreeNode* root) {
	root->quad = nextstat();
	SymbolTable* t = new(nothrow)SymbolTable;
	if (!t) { /*cerr << "ERROR: create symbol table failed\n"; exit(-1);*/ throw string("ERROR: 内部错误:bad_alloc创建符号表失败\n"); }
	if (lastSymbolTable)
	{
		lastSymbolTable->next = t;
		t->prev = lastSymbolTable;
	}
	if (!symbolTableStack.empty())
	{
		t->parent = symbolTableStack.back();
	}
	lastSymbolTable = t;
	symbolTableStack.push_back(t);
	if (offsetStack.empty())
	{
		offsetStack.push_back(0);
	}
	else
	{
		int back_offset = offsetStack.back();
		offsetStack.push_back(back_offset);
	}
}

void SemanticAnalyser::N(SyntaxTreeNode* root) {
	SymbolTable* t = symbolTableStack.back();
	symbolTableStack.pop_back();
	t->width = t->table.empty() ? 0 : offsetStack.back() - t->table[0].offset;
	offsetStack.pop_back();

	emit("-", "$sp", to_string(t->width), "$sp");

	t = new(nothrow)SymbolTable;
	if (!t) { /*cerr << "ERROR: create symbol table failed\n"; exit(-1);*/ throw string("ERROR: 内部错误:bad_alloc创建符号表失败\n"); }
	if (lastSymbolTable)
	{
		lastSymbolTable->next = t;
		t->prev = lastSymbolTable;
	}
	if (!symbolTableStack.empty())
	{
		t->parent = symbolTableStack.back();
	}
	lastSymbolTable = t;
	symbolTableStack.push_back(t);
	if (offsetStack.empty())
	{
		offsetStack.push_back(0);
	}
	else
	{
		int back_offset = offsetStack.back();
		offsetStack.push_back(back_offset);
	}

	root->trueList = nextstat();
	emit("j", "", "", to_string(0));
	root->quad = nextstat();
}

void SemanticAnalyser::EXPRESSION0(SyntaxTreeNode* root) {
	root->place = root->children[0]->place;
}

void SemanticAnalyser::EXPRESSION1(SyntaxTreeNode* root) {
	root->place = newtemp();
	emit("jnz", root->children[0]->place, "", to_string(nextstat() + 4));
	emit("jnz", root->children[2]->place, "", to_string(nextstat() + 3));
	emit(":=", to_string(0), "", root->place);
	emit("j", "", "", to_string(nextstat() + 2));
	emit(":=", to_string(1), "", root->place);
}

void SemanticAnalyser::BOOLAND0(SyntaxTreeNode* root) {
	root->place = root->children[0]->place;
}

void SemanticAnalyser::BOOLAND1(SyntaxTreeNode* root) {
	root->place = newtemp();
	emit("jnz", root->children[0]->place, "", to_string(nextstat() + 2));
	emit("j", "", "", to_string(nextstat() + 2));
	emit("jnz", root->children[2]->place, "", to_string(nextstat() + 3));
	emit(":=", to_string(0), "", root->place);
	emit("j", "", "", to_string(nextstat() + 2));
	emit(":=", to_string(1), "", root->place);
}

void SemanticAnalyser::BOOLNOT0(SyntaxTreeNode* root) {
	root->place = root->children[0]->place;
}

void SemanticAnalyser::BOOLNOT1(SyntaxTreeNode* root) {
	root->place = newtemp();
	emit("jnz", root->children[1]->place, "", to_string(nextstat() + 3));
	emit(":=", to_string(1), "", root->place);
	emit("j", "", "", to_string(nextstat() + 2));
	emit(":=", to_string(0), "", root->place);
}

void SemanticAnalyser::COMP0(SyntaxTreeNode* root) {
	root->place = root->children[0]->place;
}

void SemanticAnalyser::COMP1(SyntaxTreeNode* root) {
	root->place = newtemp();
	switch (root->children[1]->content.second)
	{
		case 0:
			emit("j<", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 1:
			emit("j<=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 2:
			emit("j>", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 3:
			emit("j>=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 4:
			emit("j==", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
		case 5:
			emit("j!=", root->children[0]->place, root->children[2]->place, to_string(nextstat() + 3)); break;
	}
	emit(":=", to_string(0), "", root->place);
	emit("j", "", "", to_string(nextstat() + 2));
	emit(":=", to_string(1), "", root->place);
}

void SemanticAnalyser::PLUSEX0(SyntaxTreeNode* root) {
	root->place = root->children[0]->place;
}

void SemanticAnalyser::PLUSEX1(SyntaxTreeNode* root) {
	root->place = newtemp();
	switch (root->children[1]->content.second)
	{
		case 0:
			emit("+", root->children[0]->place, root->children[2]->place, root->place); break;
		case 1:
			emit("-", root->children[0]->place, root->children[2]->place, root->place); break;
		case 2:
			emit("&", root->children[0]->place, root->children[2]->place, root->place); break;
		case 3:
			emit("|", root->children[0]->place, root->children[2]->place, root->place); break;
		case 4:
			emit("^", root->children[0]->place, root->children[2]->place, root->place); break;
	}
}

void SemanticAnalyser::TERM0(SyntaxTreeNode* root) {
	root->place = root->children[0]->place;
}

void SemanticAnalyser::TERM1(SyntaxTreeNode* root) {
	root->place = newtemp();
	switch (root->children[1]->content.second)
	{
		case 0:
			emit("*", root->children[0]->place, root->children[2]->place, root->place); break;
		case 1:
			emit("/", root->children[0]->place, root->children[2]->place, root->place); break;
	}
}

void SemanticAnalyser::FACTOR0(SyntaxTreeNode* root) {
	root->place = newtemp();
	emit(":=", to_string(root->children[0]->content.second), "", root->place);
}

void SemanticAnalyser::FACTOR1(SyntaxTreeNode* root) {
	root->place = root->children[1]->place;
}

void SemanticAnalyser::FACTOR2(SyntaxTreeNode* root, map<int, string>& nameTable) {
	string p = lookup(root->children[0]->content.second);
	if (p == "")
	{
		//cerr << "ERROR: " << root->children[0]->content.second << " is undefineded\n";
		//exit(-1);
		throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
	}
	else
	{
		root->place = p;
	}
}

void SemanticAnalyser::FACTOR3(SyntaxTreeNode* root, map<int, string>& nameTable) {
	if (root->children[0]->arrayShpae.size() != 1)
	{
		/*cerr << "ERROR: 数组索引不完整\n";
		exit(-1);*/
		throw string("ERROR: 语义分析器错误:遇到不完整的数组索引\n");
	}
	string p = lookup(root->children[0]->content.second);
	if (p == "")
	{
		//cerr << "ERROR: " << root->children[0]->content.second << "is undefineded\n";
		//exit(-1);
		throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
	}
	else
	{
		root->place = newtemp();
		emit("=[]", p, root->children[0]->place, root->place);
	}
}

void SemanticAnalyser::FACTOR4(SyntaxTreeNode* root, map<int, string>& nameTable) {
	SymbolTableItem* f = find(root->children[0]->content.second);
	if (f == NULL)
	{
		throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string("未定义\n");
	}
	if (f->arrayShape[0] != root->children[1]->params.size())
	{
		//cerr << "ERROR: 调用过程 " << f->id << " 需要实参: " << f->arrayShpae[0] << " 个, 实际给出: " << root->children[1]->params.size() << " 个\n";
		throw string("ERROR: 语义分析器错误:调用过程 ") + nameTable[f->id] + string(" 需要实参: ") + to_string(f->arrayShape[0]) + string(" 个, 实际给出: ") + to_string(root->children[1]->params.size()) + string("个\n");
	}

	emit(":=", "$ra", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$t0", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$t1", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$t2", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$t3", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$t4", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$t5", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$t6", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$t7", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");

	//string t = newtemp();
	emit(":=", "$sp", "", "$s0");
	emit(":=", "$fp", "", "[$sp]");
	emit("+", "$sp", to_string(4), "$sp");
	emit(":=", "$s0", "", "$fp");

	for (auto i = 0; i < root->children[1]->params.size(); i++)
	{
		emit(":=", root->children[1]->params[i], "", "[$sp]");
		emit("+", "$sp", to_string(4), "$sp");
	}
	emit("jal", "", "", to_string(f->offset));
	emit(":=", "$fp", "", "$sp");
	emit(":=", "[$sp]", "", "$fp");

	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$t7");
	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$t6");
	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$t5");
	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$t4");
	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$t3");
	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$t2");
	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$t1");
	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$t0");
	emit("-", "$sp", to_string(4), "$sp");
	emit(":=", "[$sp]", "", "$ra");

	root->place = newtemp();
	emit(":=", "$v0", "", root->place);
}

void SemanticAnalyser::FACTOR5(SyntaxTreeNode* root) {
	root->place = root->children[1]->place;
}

void SemanticAnalyser::CALL(SyntaxTreeNode* root) {
	root->params = root->children[1]->params;
}

void SemanticAnalyser::ARRAY0(SyntaxTreeNode* root, map<int, string>& nameTable) {
	SymbolTableItem* e = find(root->children[0]->content.second);
	if (e == NULL)
	{
		//cerr << "ERROR: " << root->children[0]->content.second << " is undefineded\n";
		//exit(-1);
		throw string("ERROR: 语义分析器错误:") + nameTable[root->children[0]->content.second] + string(" 未定义\n");
	}
	root->content = root->children[0]->content;
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->arrayShpae = e->arrayShape;

	if (root->arrayShpae.size() == 0)
	{
		//cerr << "ERROR: array pos error\n";
		//exit(-1);
		throw string("ERROR: 语义分析器错误:数组下标错误");
	}
	else if (root->arrayShpae.size() == 1)
	{
		root->place = root->children[2]->place;
	}
	else
	{
		int dim_len = root->arrayShpae[1];
		for (auto i = 2; i < root->arrayShpae.size(); i++)
		{
			dim_len *= root->arrayShpae[i];
		}
		string p = newtemp();
		emit(":=", to_string(dim_len), "", p);
		root->place = newtemp();
		emit("*", p, root->children[2]->place, root->place);
	}
}

void SemanticAnalyser::ARRAY1(SyntaxTreeNode* root) {
	root->content = root->children[0]->content;
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->arrayShpae = root->children[0]->arrayShpae;
	root->arrayShpae.erase(root->arrayShpae.begin());

	if (root->arrayShpae.size() == 0)
	{
		//cerr << "ERROR: array pos error\n";
		//exit(-1);
		throw string("ERROR: 语义分析器错误:数组下标错误");
	}
	else if (root->arrayShpae.size() == 1)
	{
		root->place = newtemp();
		emit("+", root->children[0]->place, root->children[2]->place, root->place);
	}
	else
	{
		int dim_len = root->arrayShpae[1];
		for (auto i = 2; i < root->arrayShpae.size(); i++)
		{
			dim_len *= root->arrayShpae[i];
		}
		string p1 = newtemp();
		emit(":=", to_string(dim_len), "", p1);
		string p2 = newtemp();
		emit("*", p1, root->children[2]->place, p2);
		root->place = newtemp();
		emit("+", root->children[0]->place, p2, root->place);
	}
}

void SemanticAnalyser::ACTUALPARAM0(SyntaxTreeNode* root) {
	root->params = root->children[0]->params;
}

void SemanticAnalyser::ACTUALPARAM1() {

}

void SemanticAnalyser::ACTUALPARAMLIST0(SyntaxTreeNode* root) {
	root->params.push_back(root->children[0]->place);
}

void SemanticAnalyser::ACTUALPARAMLIST1(SyntaxTreeNode* root) {
	root->params = root->children[2]->params;
	root->params.push_back(root->children[0]->place);
}











void SemanticAnalyser::analyse(string token, SyntaxTreeNode* root, map<int, string> nameTable) {
	if (token == ProgramToken) {
		this->Program();
	}
	else if (token == MToken) {
		this->M(root);
	}
	else if (token == ASSERTIONSToken0) {
		this->ASSERTIONS0();
	}
	else if (token == ASSERTIONSToken1) {
		this->ASSERTIONS1();
	}
	else if (token == ASSERTIONToken0) {
		this->ASSERTION0(root);
	}
	else if (token == ASSERTIONToken1) {
		this->ASSERTION1(root);
	}
	else if (token == ASSERTIONTYPEToken0) {
		this->ASSERTIONTYPE0(root);
	}
	else if (token == ASSERTIONTYPEToken1) {
		this->ASSERTIONTYPE1(root);
	}
	else if (token == FUNCASSERTIONToken0) {
		this->FUNCASSERTION0(root);
	}
	else if (token == FUNCASSERTIONToken1) {
		this->FUNCASSERTION1(root);
	}
	else if (token == ARRAYASSERTIONToken0) {
		this->ARRAYASSERTION0(root);
	}
	else if (token == ARRAYASSERTIONToken1) {
		this->ARRAYASSERTION1(root);
	}
	else if (token == FORMALPARAMToken0) {
		this->FORMALPARAM0(root);
	}
	else if (token == FORMALPARAMToken1) {
		this->FORMALPARAM1(root);
	}
	else if (token == FORMALPARAMToken2) {
		this->FORMALPARAM2(root);
	}
	else if (token == FORMALPARAMLISTToken0) {
		this->FORMALPARAMLIST0(root);
	}
	else if (token == FORMALPARAMLISTToken1) {
		this->FORMALPARAMLIST1(root);
	}
	else if (token == SENBLOCKToken) {
		this->SENBLOCK();
	}
	else if (token == INNERASSERTIONToken0) {
		this->INNERASSERTION0();
	}
	else if (token == INNERASSERTIONToken1) {
		this->INNERASSERTION1();
	}
	else if (token == INNERVARIDEFToken0) {
		this->INNERVARIDEF0(root);
	}
	else if (token == INNERVARIDEFToken1) {
		this->INNERVARIDEF1(root);
	}
	else if (token == SENSEQToken0) {
		this->SENSEQ0();
	}
	else if (token == SENSEQToken1) {
		this->SENSEQ1();
	}
	else if (token == SENTENCEToken0) {
		this->SENTENCE0();
	}
	else if (token == SENTENCEToken1) {
		this->SENTENCE1();
	}
	else if (token == SENTENCEToken2) {
		this->SENTENCE2();
	}
	else if (token == SENTENCEToken3) {
		this->SENTENCE3();
	}
	else if (token == ASSIGNMENTToken0) {
		this->ASSIGNMENT0(root, nameTable);
	}
	else if (token == ASSIGNMENTToken1) {
		this->ASSIGNMENT1(root, nameTable);
	}
	else if (token == RETURNSENToken0) {
		this->RETURNSEN0(root);
	}
	else if (token == RETURNSENToken1) {
		this->RETURNSEN1();
	}
	else if (token == WHILESENToken) {
		this->WHILESEN(root);
	}
	else if (token == BToken) {
		this->B(root);
	}
	else if (token == IFSENToken0) {
		this->IFSEN0(root);
	}
	else if (token == IFSENToken1) {
		this->IFSEN1(root);
	}
	else if (token == CTRLToken) {
		this->CTRL(root);
	}
	else if (token == TToken) {
		this->T(root);
	}
	else if (token == NToken) {
		this->N(root);
	}
	else if (token == EXPRESSIONToken0) {
		this->EXPRESSION0(root);
	}
	else if (token == EXPRESSIONToken1) {
		this->EXPRESSION1(root);
	}
	else if (token == BOOLANDToken0) {
		this->BOOLAND0(root);
	}
	else if (token == BOOLANDToken1) {
		this->BOOLAND1(root);
	}
	else if (token == BOOLNOTToken0) {
		this->BOOLNOT0(root);
	}
	else if (token == BOOLNOTToken1) {
		this->BOOLNOT1(root);
	}
	else if (token == COMPToken0) {
		this->COMP0(root);
	}
	else if (token == COMPToken1) {
		this->COMP1(root);
	}
	else if (token == PLUSEXToken0) {
		this->PLUSEX0(root);
	}
	else if (token == PLUSEXToken1) {
		this->PLUSEX1(root);
	}
	else if (token == TERMToken0) {
		this->TERM0(root);
	}
	else if (token == TERMToken1) {
		this->TERM1(root);
	}
	else if (token == FACTORToken0) {
		this->FACTOR0(root);
	}
	else if (token == FACTORToken1) {
		this->FACTOR1(root);
	}
	else if (token == FACTORToken2) {
		this->FACTOR2(root, nameTable);
	}
	else if (token == FACTORToken3) {
		this->FACTOR3(root, nameTable);
	}
	else if (token == FACTORToken4) {
		this->FACTOR4(root, nameTable);
	}
	else if (token == FACTORToken5) {
		this->FACTOR5(root);
	}
	else if (token == CALLToken) {
		this->CALL(root);
	}
	else if (token == ARRAYToken0) {
		this->ARRAY0(root, nameTable);
	}
	else if (token == ARRAYToken1) {
		this->ARRAY1(root);
	}
	else if (token == ACTUALPARAMToken0) {
		this->ACTUALPARAM0(root);
	}
	else if (token == ACTUALPARAMToken1) {
		this->ACTUALPARAM1();
	}
	else if (token == ACTUALPARAMLISTToken0) {
		this->ACTUALPARAMLIST0(root);
	}
	else if (token == ACTUALPARAMLISTToken1) {
		this->ACTUALPARAMLIST1(root);
	}
}

/*============================== 语义分析器 ==============================*/