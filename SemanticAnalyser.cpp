#include "SemanticAnalyser.h"

/*============================== ���ű���SymbolTableItem ==============================*/

Quaternion::Quaternion(const string& op, const string& arg1, const string& arg2, const string& result) {
	this->op = op;
	this->arg1 = arg1;
	this->arg2 = arg2;
	this->result = result;
}

Quaternion::~Quaternion() {

}

ostream& operator<< (ostream& output, const Quaternion& q) {
	output << q.op << " " << q.arg1 << " " << q.arg2 << " " << q.result;
	return output;
}

/*============================== ���ű���SymbolTableItem ==============================*/



/*============================== ���ű���SymbolTableItem ==============================*/

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

/*============================== ���ű���SymbolTableItem ==============================*/



/*============================== ���ű���SymbolTable ==============================*/

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

	// ���id�Ƿ��ض���
	for (int i = 0; i < table.size(); i++) {
		if (table[i].id == id) {
			cerr << "ERROR: ���������: ���ű�id�ض���: " << to_string(id) << "�ض���" << endl;
			exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_REDEFINE);
		}
	}

	// �����
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
	// ������ǰ�ķ��ű�SymbolTable
	for (int i = 0; i < table.size(); i++) {
		// ����id
		if (table[i].id == id && table[i].symbolType == SymbolTableItem::S_FUNC) {
			table[i].FuncTablePointer = p; // ��ֵ������ָ��
			break;
		}
	}

}

/*============================== ���ű���SymbolTable ==============================*/



/*============================== �������ڵ� ==============================*/

SyntaxTreeNode::SyntaxTreeNode() {
	
	parent = NULL;

	level = 0;
	dataType = SymbolTableItem::S_INT;
	symbolType = SymbolTableItem::S_VAR;

	n = 0;
	width = 0;

	clear();
}

SyntaxTreeNode::~SyntaxTreeNode() {

}

void SyntaxTreeNode::clear() {
	level = -1;
	parent = NULL;
	children.clear();
	token = Token("", -1);
}

/*============================== �������ڵ� ==============================*/



/*============================== ��������� ==============================*/

SemanticAnalyser::SemanticAnalyser() {
	tmpCnt = TmpCntInit;
	lastSymbolTable = NULL;
}

SemanticAnalyser::~SemanticAnalyser() {

}

/*
* Function Name:        newtemp
* Function Description: ����һ���µı�����
*/
string SemanticAnalyser::newtemp() {
	string tmp = "V" + to_string(offsetStack.top());
	symbolTableStack.top()->enter(tmpCnt--, offsetStack.top(), SymbolTableItem::S_INT, SymbolTableItem::S_VAR);
	offsetStack.top() += 4;
	emit("+", "$sp", to_string(4), "$sp");
	return tmp;
}

/*
* Function Name:        lookup
* Function Description: ����Ƿ����id��Ӧ�ı���
*/
string SemanticAnalyser::lookup(int id) {
	SymbolTable* stsp = symbolTableStack.top(); // ȡ��ǰ���ű�ջ��
	while (stsp) {
		// ������ǰ���ű�
		for (int i = 0; i < stsp->table.size(); i++) {
			if (stsp->table[i].id == id) {
				int offset = stsp->table[i].offset; // ȡƫ����
				// �ж�����ΪVAR����ARRAY
				if (stsp->table[i].symbolType == SymbolTableItem::S_VAR || stsp->table[i].symbolType == SymbolTableItem::S_ARRAY) {
					// �ж��Ƿ���ȫ�ֱ���
					if (stsp->parent) {
						return "V" + to_string(offset);
					}
					else {
						return "G" + to_string(offset);
					}
				}
			}
		}
		// ���·��ű�
		stsp = stsp->parent;
	}
	// δ�ҵ�
	return "";
}

/*
* Function Name:        find
* Function Description: ����id��Ӧ��TableItem
*/
SymbolTableItem* SemanticAnalyser::find(int id) {
	SymbolTable* tp = symbolTableStack.top();
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

/*
* Function Description: ������һ������λ��
*/
int SemanticAnalyser::nextstat() {
	return intermediateCode.size();
}

/*
* Function Description: ����Ԫʽ�����м����
*/
void SemanticAnalyser::emit(string op, string arg1, string arg2, string result) {
	Quaternion q(op, arg1, arg2, result);
	intermediateCode.push_back(q);
}


/*
* Function Description: ��ջ��ָ��-4���ٽ���ջ�е�ֵ�洢�ڼĴ���
*/
void SemanticAnalyser::loadRegFromSp(const string& r) {
	this->emit("-", "$sp", to_string(VarWidth), "$sp");
	this->emit(":=", "[$sp]", "", r);
}

/*
* Function Description: ���Ĵ����洢�ڶ�ջ�У�����ջ��ָ��+4
*/
void SemanticAnalyser::saveRegInSp(const string& r) {
	this->emit(":=", r, "", "[$sp]");
	this->emit("+", "$sp", to_string(VarWidth), "$sp");
}

/*
* Function Name: 
* Function Description: ���� <PROGRAM>::=<M><ASSERTIONS>
*/
void SemanticAnalyser::Program() {
	gSymbolTable = symbolTableStack.top();
	symbolTableStack.pop();
	gSymbolTable->width = offsetStack.top();
	offsetStack.pop();
}

/*
* Function Name: M
* Function Description: ���� <M>::='epsilon'
*/
void SemanticAnalyser::M(SyntaxTreeNode* root) {
	root->quad = nextstat(); // ����quad��ֵ�����ڵ�
	
	// ��̬����ռ�-���ű�
	SymbolTable* stp = new(nothrow) SymbolTable;
	if (!stp) {
		cerr << "ERROR: ������ű�ʧ��" << endl;
		exit(ERROR_NEW);
	}

	// ά��˫������
	if (lastSymbolTable != NULL) {
		lastSymbolTable->next = stp;
		stp->prev = lastSymbolTable;
	}

	// ά��parent
	if (!symbolTableStack.empty()) {
		stp->parent = symbolTableStack.top();
	}

	// ����lastTable, SymbolTableStack, offsetStack
	lastSymbolTable = stp;
	symbolTableStack.push(stp);
	offsetStack.push(0);
}

void SemanticAnalyser::ASSERTIONS0() {

}

void SemanticAnalyser::ASSERTIONS1() {

}

/*
* ASSERTION����
* <ASSERTION>::='INT''ID'<ASSERTIONTYPE>'DEL'
*/
void SemanticAnalyser::ASSERTION0(SyntaxTreeNode* root) {
	// ����root�ڵ�����
	root->dataType = SymbolTableItem::S_INT; // ��������ΪINT
	root->symbolType = root->children[ASSERTION0_ASSERTIONTYPE_CHILDPOS]->symbolType; // �̳з�������
	root->n = root->children[ASSERTION0_ASSERTIONTYPE_CHILDPOS]->n; // �̳з���Ԫ�ظ���
	root->width = VarWidth * root->n; // ����ռ�

	// �ڵ�ǰ���ű��в���
	this->symbolTableStack.top()->enter(root->children[ASSERTION0_ID_CHILDPOS]->token.code, offsetStack.top(), root->dataType, root->symbolType);
	
	// ��Ϊ��������
	if (root->symbolType == SymbolTableItem::S_ARRAY) {
		root->arrayShpae = root->children[ASSERTION0_ASSERTIONTYPE_CHILDPOS]->arrayShpae; // �̳�arrayShape
		symbolTableStack.top()->setIdArrayShape(root->children[ASSERTION0_ID_CHILDPOS]->token.code, root->arrayShpae); // ���·��ű��и�id��arrayShape
	}

	// ����offsetStack
	offsetStack.top() += root->width;
}

/*
* ASSERTION����
* ���룺<ASSERTION>::==<FUNCASSERTION><SENBLOCK>
*/
void SemanticAnalyser::ASSERTION1(SyntaxTreeNode* root) {
	// ����root�ڵ����ݣ��̳�ASSERTION1_FUNCASSERTION_CHILDPOS�ڵ�
	root->dataType = root->children[ASSERTION1_FUNCASSERTION_CHILDPOS]->dataType;
	root->symbolType = root->children[ASSERTION1_FUNCASSERTION_CHILDPOS]->symbolType;
	root->n = root->children[ASSERTION1_FUNCASSERTION_CHILDPOS]->n;
	root->width = root->children[ASSERTION1_FUNCASSERTION_CHILDPOS]->width;

	// ȡ���ű�ջջ��
	SymbolTable* stp = symbolTableStack.top();
	symbolTableStack.pop();

	stp->width = stp->table.empty() ? 0 : offsetStack.top() - stp->table[0].offset;
	offsetStack.pop();
}

void SemanticAnalyser::ASSERTIONTYPE0(SyntaxTreeNode* root) {
	root->symbolType = SymbolTableItem::S_VAR;
	root->n = 1;
}

/*
* ASSERTIONTYPE1
* ���룺<ASSERTIONTYPE>::=<ARRAYASSERTION>
*/
void SemanticAnalyser::ASSERTIONTYPE1(SyntaxTreeNode* root) {
	root->symbolType = SymbolTableItem::S_ARRAY; // ����ǰ�ڵ���������ΪARRAY
	root->n = root->children[0]->n; // �̳�n������Ԫ�أ�
	root->arrayShpae = root->children[0]->arrayShpae; // �̳�arrayShape
	reverse(root->arrayShpae.begin(), root->arrayShpae.end()); // ����
}

/*
* Function Name:        FUNCASSERTION_UpdateStack
* Function Description: ����FUNCASSERTION0��FUNCASSERTION1�������̽��з��ű�ջ��ƫ����ջ�ĸ���
*/
void SemanticAnalyser::FUNCASSERTION_UpdateStack(const SyntaxTreeNode* root) {
	// ȡ���ű�ջ��ƫ����ջ��ջ��
	// ��ջ���ݽṹ��ֻ�ܶ�ջ�����в�������Ҫ��ȡ��ջ��Ԫ��
	SymbolTable* stp = this->symbolTableStack.top();
	this->symbolTableStack.pop();
	int offset = this->offsetStack.top();
	this->offsetStack.pop();

	// �ڵ�ǰջ�����ű��ƫ�������м���ĺ���
	this->symbolTableStack.top()->enter(root->children[FUNCASSERTION_FUNCID_CHILDPOS]->token.code, root->children[FUNCASSERTION_OFFSET_CHILDPOS]->quad, root->dataType, root->symbolType);
	this->symbolTableStack.top()->setIdFuncTablePointer(root->children[FUNCASSERTION_FUNCID_CHILDPOS]->token.code, stp); // ���ú������ű�ָ��
	this->symbolTableStack.top()->setIdArrayShape(root->children[FUNCASSERTION_FUNCID_CHILDPOS]->token.code, root->children[FUNCASSERTION_FORMALPARAM_CHILDPOS]->arrayShpae); // ���ò����б����
	this->offsetStack.top() += 0;
	
	// ��ȡ���ķ��ű��ƫ�����Ż�ջ��
	symbolTableStack.push(stp);
	offsetStack.push(offset);
}

/*
* Function Name:        FUNCASSERTION0
* Function Description: ���� <FUNCASSERTION>::='VOID''ID'<M>'LP'<FORMALPARAM>'RP'
*                       ����ֵΪvoid�ĺ�������
*/
void SemanticAnalyser::FUNCASSERTION0(SyntaxTreeNode* root) {
	// ����root��Ϣ
	root->dataType = SymbolTableItem::S_VOID;
	root->symbolType = SymbolTableItem::S_FUNC;
	root->n = 1;
	root->width = -1 * root->n;

	// ���·��ű�ջ��ƫ����ջ
	this->FUNCASSERTION_UpdateStack(root);
}

/*
* Function Name:        FUNCASSERTION1
* Function Description: ���� <FUNCASSERTION>::='INT''ID'<M>'LP'<FORMALPARAM>'RP'
*                       ����ֵΪint�ĺ�������
*/
void SemanticAnalyser::FUNCASSERTION1(SyntaxTreeNode* root) {
	// ����root��Ϣ
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_FUNC;
	root->n = 1;
	root->width = -1 * root->n;

	// ���·��ű�ջ��ƫ����ջ
	this->FUNCASSERTION_UpdateStack(root);
}

/*
* Function Name:        ARRAYASSERTION0
* Function Description: ���� <ARRAYASSERTION>::='LS''NUM''RS'
*/
void SemanticAnalyser::ARRAYASSERTION0(SyntaxTreeNode* root) {
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->n = root->children[1]->token.code;
	root->arrayShpae.push_back(root->children[1]->token.code);
}

/*
* Function Name:        ARRAYASSERTION1
* Function Description: ���� <ARRAYASSERTION>::='LS''NUM''RS'<ARRAYASSERTION>
*/
void SemanticAnalyser::ARRAYASSERTION1(SyntaxTreeNode* root) {
	root->symbolType = SymbolTableItem::S_ARRAY; // root����Ϊ��������
	root->n = root->children[ARRAYASSERTION1_NUM_CHILDPOS]->token.code * root->children[ARRAYASSERTION1_ARRAYASSERTION_CHILDPOS]->n; // ����ǰά����ǰ���Լ��ARRAYASSERTION��ˣ��õ�n
	root->arrayShpae = root->children[ARRAYASSERTION1_ARRAYASSERTION_CHILDPOS]->arrayShpae; // ��ǰ��arrayShape��ֵΪǰ���Լ��ARRAYASSERTION��arrayShape
	root->arrayShpae.push_back(root->children[ARRAYASSERTION1_NUM_CHILDPOS]->token.code); // ��arrayShape�м��뵱ǰ��ά�ȣ�ע���ʱά��Ϊ����
}

/*
* Function Name:        FORMALPARAM0
* Function Description: ���� <FORMALPARAM>::=<FORMALPARAMLIST>
*/
void SemanticAnalyser::FORMALPARAM0(SyntaxTreeNode* root) {
	root->arrayShpae = root->children[FORMALPARAM_ARRAYSHAPE_CHILDPOS]->arrayShpae; // �������Ĳ����б���root
}

/*
* Function Name:        FORMALPARAM0
* Function Description: ���� <FORMALPARAM>::='VOID'
*                       �������Ĳ����б���root
*/
void SemanticAnalyser::FORMALPARAM1(SyntaxTreeNode* root) {
	root->arrayShpae.push_back(0); // �޲����б�
}

/*
* Function Name:        FORMALPARAM0
* Function Description: ���� <FORMALPARAM>::='epsilon'
*                       �������Ĳ����б���root
*/
void SemanticAnalyser::FORMALPARAM2(SyntaxTreeNode* root) {
	root->arrayShpae.push_back(0); // �޲����б�
}

/*
* Function Name:        FORMALPARAMLIST0
* Function Description: ���� <FORMALPARAMLIST>::='INT''ID'
*                       ���������б��еı���������䣬�����ı���Ϊint����
*/
void SemanticAnalyser::FORMALPARAMLIST0(SyntaxTreeNode* root) {
	// ����root�ڵ�����
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_VAR;
	root->n = VarNum;
	root->width = VarWidth;
	root->arrayShpae.push_back(VarShape);

	// ���·��ű��ƫ������
	symbolTableStack.top()->enter(root->children[FORMALPARAMLIST_VARID_CHILDPOS]->token.code, offsetStack.top(), root->dataType, root->symbolType);
	offsetStack.top() += root->width;
}

/*
* Function Name:        FORMALPARAMLIST1
* Function Description: ���� <FORMALPARAMLIST>::='INT''ID''SEP'<FORMALPARAMLIST>
*                       ���������б��еı���������䣬�����ı���Ϊint����
*/
void SemanticAnalyser::FORMALPARAMLIST1(SyntaxTreeNode* root) {
	// ����root�ڵ�����
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_VAR;
	root->n = VarNum;
	root->width = VarWidth;
	root->arrayShpae = root->children[FORMALPARAMLIST_ARRAYSHAPE_CHILDPOS]->arrayShpae; // ��ǰ�������ı����ӵ����ڵ���
	root->arrayShpae[0]++;

	// ���·��ű��ƫ������
	symbolTableStack.top()->enter(root->children[FORMALPARAMLIST_VARID_CHILDPOS]->token.code, offsetStack.top(), root->dataType, root->symbolType);
	offsetStack.top() += root->width;
}

//
/*
* Function Name:        SENBLOCK
* Function Description: ���� <SENBLOCK>::='LB'<INNERASSERTION><SENSEQ>'RB'
*/
void SemanticAnalyser::SENBLOCK() {

}

// <INNERASSERTION>::=<INNERVARIDEF>'DEL'<INNERASSERTION>
void SemanticAnalyser::INNERASSERTION0() {

}

// <INNERASSERTION>::='epsilon'
void SemanticAnalyser::INNERASSERTION1() {

}

/*
* Function Name:        INNERVARIDEF0
* Function Description: ���� <INNERVARIDEF>::='INT''ID'
*                       �����ڲ��еı���������䣬�����ı���Ϊint����
*/
void SemanticAnalyser::INNERVARIDEF0(SyntaxTreeNode* root) {
	// ����root��Ϣ
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_VAR;
	root->n = VarNum;
	root->width = VarWidth;

	// ���·��ű�ջ��ƫ����ջ
	symbolTableStack.top()->enter(root->children[INNERVARIDEF_VARID_CHILDPOS]->token.code, offsetStack.top(), root->dataType, root->symbolType);
	offsetStack.top() += root->width;

	// �����м����
	emit("+", "$sp", to_string(root->width), "$sp");
}

/*
* Function Name:        INNERVARIDEF1
* Function Description: ���� <INNERVARIDEF>::='INT''ID'<ARRAYASSERTION>
*                       �����ڲ��еı���������䣬�����ı���Ϊint��������
*/
void SemanticAnalyser::INNERVARIDEF1(SyntaxTreeNode* root) {
	// ����root��Ϣ
	root->dataType = SymbolTableItem::S_INT;
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->n = root->children[INNERVARIDEF_ARRAY_CHILDPOS]->n;
	root->width = VarWidth * root->n;
	root->arrayShpae = root->children[INNERVARIDEF_ARRAY_CHILDPOS]->arrayShpae;
	reverse(root->arrayShpae.begin(), root->arrayShpae.end());

	// ���·��ű�ջ��ƫ����ջ
	symbolTableStack.top()->enter(root->children[INNERVARIDEF_VARID_CHILDPOS]->token.code, offsetStack.top(), root->dataType, root->symbolType);
	symbolTableStack.top()->setIdArrayShape(root->children[INNERVARIDEF_VARID_CHILDPOS]->token.code, root->arrayShpae);
	offsetStack.top() += root->width;

	// �����м����
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

void SemanticAnalyser::ASSIGNMENT0(SyntaxTreeNode* root, map<int, string>& varTable) {
	string p = lookup(root->children[0]->token.code);
	if (p == "") {
		cerr << "ERROR: �������������:" << varTable[root->children[0]->token.code] << "δ����" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTDEFINE);
	}
	else {
		emit(":=", root->children[2]->place, "", p);
		root->place = newtemp();
		emit(":=", root->children[2]->place, "", root->place);
	}
}

void SemanticAnalyser::ASSIGNMENT1(SyntaxTreeNode* root, map<int, string>& varTable) {
	if (root->children[0]->arrayShpae.size() != 1) {
		cerr << "ERROR: �������������:��������������������" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTCOMPLETEINDEX);
	}
	string p = lookup(root->children[0]->token.code);
	if (p == "") {
		cerr << "ERROR: �������������:" << varTable[root->children[0]->token.code] << "δ����" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTDEFINE);
	}
	else
	{
		emit("[]=", root->children[2]->place, root->children[0]->place, p);
		root->place = newtemp();
		emit(":=", root->children[2]->place, "", root->place);
	}
}

/*
* Function Name:        RETURNSEN0
* Function Description: ���� <RETURNSEN>::='RETURN'<EXPRESSION>
*/
void SemanticAnalyser::RETURNSEN0(SyntaxTreeNode* root) {
	emit(":=", root->children[RETURNSEN0_EXPRESSION_CHILDPOS]->place, "", "$v0"); // ������ֵ������$v0�Ĵ�����
	emit("ret", "", "", ""); // ����
}

/*
* Function Name:        RETURNSEN1
* Function Description: ���� <RETURNSEN>::='RETURN'
*/
void SemanticAnalyser::RETURNSEN1() {
	emit(":=", to_string(0), "", "$v0"); // ������ֵ������$v0�Ĵ�����
	emit("ret", "", "", ""); // ����
}

void SemanticAnalyser::WHILESEN(SyntaxTreeNode* root) {
	SymbolTable* t = symbolTableStack.top();
	symbolTableStack.pop();
	t->width = t->table.empty() ? 0 : offsetStack.top() - t->table[0].offset;
	offsetStack.pop();

	emit("-", "$sp", to_string(t->width), "$sp");

	emit("j", "", "", to_string(root->children[0]->quad));
	intermediateCode[root->children[3]->trueList].result = to_string(root->children[5]->quad);
	intermediateCode[root->children[3]->falseList].result = to_string(nextstat());
}

/*
* Function Name: B
* Function Description:
*/
void SemanticAnalyser::B(SyntaxTreeNode* root) {
	root->quad = nextstat();
}

/*
* Function Name: IFSEN0
* Function Description: ����<IFSEN>::='IF''LP'<CTRL>'RP'<T><SENBLOCK>
*/
void SemanticAnalyser::IFSEN0(SyntaxTreeNode* root) {
	// ȡ����ջ�����һ�ű�
	SymbolTable* t = this->symbolTableStack.top();
	symbolTableStack.pop();

	t->width = t->table.empty() ? 0 : offsetStack.top() - t->table[0].offset;
	offsetStack.pop();

	emit("-", "$sp", to_string(t->width), "$sp");

	intermediateCode[root->children[2]->trueList].result = to_string(root->children[4]->quad);
	intermediateCode[root->children[2]->falseList].result = to_string(nextstat());
}

void SemanticAnalyser::IFSEN1(SyntaxTreeNode* root) {
	SymbolTable* t = symbolTableStack.top();
	symbolTableStack.pop();
	t->width = t->table.empty() ? 0 : offsetStack.top() - t->table[0].offset;
	offsetStack.pop();

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

/*
* Function Name: T
* Function Description: ����<T>::='epsilon'
*/
void SemanticAnalyser::T(SyntaxTreeNode* root) {
	root->quad = nextstat();
	SymbolTable* t = new(nothrow)SymbolTable;
	if (!t) { /*cerr << "ERROR: create symbol table failed\n"; exit(-1);*/ throw string("ERROR: �ڲ�����:bad_alloc�������ű�ʧ��\n"); }
	if (lastSymbolTable)
	{
		lastSymbolTable->next = t;
		t->prev = lastSymbolTable;
	}
	if (!symbolTableStack.empty())
	{
		t->parent = symbolTableStack.top();
	}
	lastSymbolTable = t;
	symbolTableStack.push(t);
	if (offsetStack.empty())
	{
		offsetStack.push(0);
	}
	else
	{
		int back_offset = offsetStack.top();
		offsetStack.push(back_offset);
	}
}

void SemanticAnalyser::N(SyntaxTreeNode* root) {
	SymbolTable* t = symbolTableStack.top();
	symbolTableStack.pop();
	t->width = t->table.empty() ? 0 : offsetStack.top() - t->table[0].offset;
	offsetStack.pop();

	emit("-", "$sp", to_string(t->width), "$sp");

	t = new(nothrow)SymbolTable;
	if (!t) { /*cerr << "ERROR: create symbol table failed\n"; exit(-1);*/ throw string("ERROR: �ڲ�����:bad_alloc�������ű�ʧ��\n"); }
	if (lastSymbolTable)
	{
		lastSymbolTable->next = t;
		t->prev = lastSymbolTable;
	}
	if (!symbolTableStack.empty())
	{
		t->parent = symbolTableStack.top();
	}
	lastSymbolTable = t;
	symbolTableStack.push(t);
	if (offsetStack.empty())
	{
		offsetStack.push(0);
	}
	else
	{
		int back_offset = offsetStack.top();
		offsetStack.push(back_offset);
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
	switch (root->children[1]->token.code)
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
	switch (root->children[1]->token.code)
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
	switch (root->children[1]->token.code)
	{
		case 0:
			emit("*", root->children[0]->place, root->children[2]->place, root->place); break;
		case 1:
			emit("/", root->children[0]->place, root->children[2]->place, root->place); break;
	}
}

/*
* Function Name:        FACTOR0
* Function Description: ���� <FACTOR>::='NUM'
*                       �����ڲ��е�int��ֵ���
*/
void SemanticAnalyser::FACTOR0(SyntaxTreeNode* root) {
	// ����root��Ϣ������������
	root->place = newtemp();

	// ������ֵ����м����
	emit(":=", to_string(root->children[FACTOR0_NUM_CHILDPOS]->token.code), "", root->place);
}

/*
* Function Name:        FACTOR1
* Function Description: ���� <FACTOR>::='LP'<EXPRESSION>'RP'
*                       �����ڲ��еı��ʽ��ֵ���
*/
void SemanticAnalyser::FACTOR1(SyntaxTreeNode* root) {
	// ����root��Ϣ��������Ϊ��ֵ���ʽ�ı�����
	root->place = root->children[FACTOR1_EXPRESSION_CHILDPOS]->place;
}

/*
* Function Name:        FACTOR2
* Function Description: ���� <FACTOR>::='ID'
*                       �����ڲ��еı�����ֵ���
*/
void SemanticAnalyser::FACTOR2(SyntaxTreeNode* root, map<int, string>& varTable) {
	string t = lookup(root->children[FACTOR2_VAR_CHILDPOS]->token.code); // ���Ҹ�ֵ����
	if (t == "") {
		// ���ű����ڸñ���
		cerr << "ERROR: �������������:" << varTable[root->children[0]->token.code] << "δ����" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTDEFINE);
	}
	else {
		root->place = t; // ��������ֵ
	}
}

/*
* Function Name:        FACTOR3
* Function Description: ���� <FACTOR>::=<ARRAY>
*                       �����ڲ��е����鸳ֵ���
*/
void SemanticAnalyser::FACTOR3(SyntaxTreeNode* root, map<int, string>& varTable) {
	if (root->children[FACTOR3_ARRAY_CHILDPOS]->arrayShpae.size() != 1) {
		cerr << "ERROR: �������������������������" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTCOMPLETEINDEX);
	}

	string t = lookup(root->children[FACTOR3_ARRAY_CHILDPOS]->token.code); // ���Ҹ�ֵ����
	if (t == "") {
		// ���ű����ڸñ���
		cerr << "ERROR: �������������:" << varTable[root->children[FACTOR3_ARRAY_CHILDPOS]->token.code] << "δ����" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTDEFINE);
	}
	else {
		root->place = newtemp(); // �����±�����
		emit("=[]", t, root->children[FACTOR3_ARRAY_CHILDPOS]->place, root->place);
	}
}

/*
* Function Name:        FACTOR4
* Function Description: ���� <FACTOR>::='ID'<CALL>
*                       �����ڲ��еĺ������ø�ֵ���
*/
void SemanticAnalyser::FACTOR4(SyntaxTreeNode* root, map<int, string>& varTable) {
	SymbolTableItem* f = this->find(root->children[FACTOR4_FUNCID_CHILDPOS]->token.code); // ���Һ�����'ID'

	if (f == NULL) { // δ�ҵ�
		cerr << "ERROR: �������������: " << varTable[root->children[FACTOR4_FUNCID_CHILDPOS]->token.code] << "δ����" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTDEFINE);
	}
	if (f->arrayShape[0] != root->children[FACTOR4_ACTUALPARALIST_CHILDPOS]->params.size()) { // ������������
		cerr << "ERROR: �������������: ���ù��� " << varTable[f->id] << " ��Ҫʵ��: " << f->arrayShape[0] << " ��, ʵ�ʸ���: " << root->children[FACTOR4_ACTUALPARALIST_CHILDPOS]->params.size() << " ��" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_FUNCPARA);
	}

	/*--- �����ֳ� ---*/
	// ����CALL_SAVE_REGS�Ĵ���
	for (int i = 0; i < CallSaveRegsNum; i++) {
		this->saveRegInSp(CALL_SAVE_REGS[i]); 
	}
	// $sp������$fp�У�����$fp�Ĵ���
	this->emit(":=", "$sp", "", "$s0");
	this->saveRegInSp("$fp");
	this->emit(":=", "$s0", "", "$fp");
	// ����ʵ��
	for (int i = 0; i < root->children[FACTOR4_ACTUALPARALIST_CHILDPOS]->params.size(); i++) {
		this->saveRegInSp(root->children[FACTOR4_ACTUALPARALIST_CHILDPOS]->params[i]);
	}

	// ������ת
	emit("jal", "", "", to_string(f->offset));

	//�������أ���ԭ$sp�Ĵ�����$fp�Ĵ���
	this->emit(":=", "$fp", "", "$sp");
	this->emit(":=", "[$sp]", "", "$fp");

	// ��ԭCALL_SAVE_REGS�Ĵ���
	for (int i = CallSaveRegsNum - 1; i >= 0; i--) {
		this->loadRegFromSp(CALL_SAVE_REGS[i]);
	}

	// Ϊ�ýڵ����ɱ����������淵��ֵ
	root->place = newtemp();
	this->emit(":=", "$v0", "", root->place);
}

/*
* Function Name:        FACTOR5
* Function Description: ���� <FACTOR>::='ID'<CALL>
*                       �����ڲ��еĺ������ø�ֵ���
*/
void SemanticAnalyser::FACTOR5(SyntaxTreeNode* root) {
	root->place = root->children[1]->place;
}

/*
* Function Name:        FACTOR5
* Function Description: ���� <CALL>::='LP'<ACTUALPARAM>'RP'
*                       ���ݲ�����
*/
void SemanticAnalyser::CALL(SyntaxTreeNode* root) {
	root->params = root->children[1]->params;
}

void SemanticAnalyser::ARRAY0(SyntaxTreeNode* root, map<int, string>& varTable) {
	SymbolTableItem* e = find(root->children[0]->token.code);
	if (e == NULL) {
		cerr << "ERROR: �������������: " << varTable[root->children[0]->token.code] << "δ����" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTDEFINE);
	}
	root->token = root->children[0]->token;
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->arrayShpae = e->arrayShape;

	if (root->arrayShpae.size() == 0) {
		cerr << "ERROR: �������������:����" << varTable[root->children[0]->token.code] << "�±����" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_ARRAYSHAPE);
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

void SemanticAnalyser::ARRAY1(SyntaxTreeNode* root, map<int, string>& varTable) {
	root->token = root->children[0]->token;
	root->symbolType = SymbolTableItem::S_ARRAY;
	root->arrayShpae = root->children[0]->arrayShpae;
	root->arrayShpae.erase(root->arrayShpae.begin());

	if (root->arrayShpae.size() == 0) {
		cerr << "ERROR: �������������:����" << varTable[root->children[0]->token.code] << "�±����" << endl;
		exit(ERROR_SEMANTIC_ANALYSE_ARRAYSHAPE);
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

/*
* Function Name:        ACTUALPARAM0
* Function Description: ���� <ACTUALPARAM>::=<ACTUALPARAMLIST>
*                       ���ݲ�����
*/
void SemanticAnalyser::ACTUALPARAM0(SyntaxTreeNode* root) {
	root->params = root->children[0]->params;
}

/*
* Function Name:        ACTUALPARAM1
* Function Description: ���� <ACTUALPARAM>::='epsilon'
*/
void SemanticAnalyser::ACTUALPARAM1() {

}


void SemanticAnalyser::ACTUALPARAMLIST0(SyntaxTreeNode* root) {
	root->params.push_back(root->children[0]->place);
}

void SemanticAnalyser::ACTUALPARAMLIST1(SyntaxTreeNode* root) {
	root->params = root->children[2]->params;
	root->params.push_back(root->children[0]->place);
}

void SemanticAnalyser::analyse(string token, SyntaxTreeNode* root, map<int, string> varTable) {
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
		this->ASSIGNMENT0(root, varTable);
	}
	else if (token == ASSIGNMENTToken1) {
		this->ASSIGNMENT1(root, varTable);
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
		this->FACTOR2(root, varTable);
	}
	else if (token == FACTORToken3) {
		this->FACTOR3(root, varTable);
	}
	else if (token == FACTORToken4) {
		this->FACTOR4(root, varTable);
	}
	else if (token == FACTORToken5) {
		this->FACTOR5(root);
	}
	else if (token == CALLToken) {
		this->CALL(root);
	}
	else if (token == ARRAYToken0) {
		this->ARRAY0(root, varTable);
	}
	else if (token == ARRAYToken1) {
		this->ARRAY1(root, varTable);
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

/*============================== ��������� ==============================*/