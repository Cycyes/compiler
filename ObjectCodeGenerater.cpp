#include "ObjectCodeGenerater.h"

/*============================== MessageTableItem ==============================*/

MessageTableItem::MessageTableItem() {

}

MessageTableItem::MessageTableItem(const int& id, const Quaternion& q) {
	this->id = id;
	this->q = q;
}

MessageTableItem::~MessageTableItem() {

}

/*============================== MessageTableItem ==============================*/



/*============================== ObjectCodeGenerater ==============================*/

ObjectCodeGenerater::ObjectCodeGenerater(const vector<Quaternion> intermediateCode, const vector<BlockItem>& block, const int& stackSize) {
	this->intermediateCode = intermediateCode;
	this->block = block;
	this->stackBufferSize = stackSize * bufferSize;
	this->dataBufferSize = stackSize * bufferSize;
	this->tempBufferSize = stackSize * bufferSize;

	RVALUE = {
		{"$t0", vector<pair<string,int>> {}} ,
		{"$t1", vector<pair<string,int>> {}},
		{"$t2", vector<pair<string,int>> {}},
		{"$t3", vector<pair<string,int>> {}},
		{"$t4", vector<pair<string,int>> {}},
		{"$t5", vector<pair<string,int>> {}},
		{"$t6", vector<pair<string,int>> {}},
		{"$t7", vector<pair<string,int>> {}}
	};
}

ObjectCodeGenerater::~ObjectCodeGenerater() {

}

void ObjectCodeGenerater::emit(string code) {
	this->objectCode.push_back(code);
}

void ObjectCodeGenerater::endBlock() {
	// 遍历AVALUE
	for (auto i = this->AVALUE.begin(); i != AVALUE.end(); i++) {
		string V = i->first;
		// 尚未存入内存的变量
		if (find(this->AVALUE[V].begin(), this->AVALUE[V].end(), "M") == this->AVALUE[V].end()) {
			string R;
			for (int j = 0; j < this->AVALUE[V].size(); j++) {
				if (this->AVALUE[V][j] != "M") {
					R = AVALUE[V][j];
					break;
				}
			}
			// sw
			if (V[0] == 'G') {
				this->emit("sw " + R + ",data+" + to_string(stoi(V.substr(1))));
			}
			else if (V[0] == 'V') {
				this->emit("sw " + R + ",stack+" + to_string(4 + stoi(V.substr(1))) + "($fp)");
			}
			else if (V[0] == 'T') {
				this->emit("sw " + R + ",temp+" + to_string(4 * stoi(V.substr(1))));
			}
			else {
				cerr << "ERROR: 目标代码生成器AVALUE出现意外变量名!" << endl;
				exit(ERROR_OBJECTCODEGENERATER);
			}
		}
	}
	AVALUE.clear(); // 清AVALUE

	// 遍历RVALUE
	for (auto i = this->RVALUE.begin(); i != this->RVALUE.end(); i++) {
		i->second.clear();
	}
}

void ObjectCodeGenerater::updateVALUE(const Tag& tag, const string& R, const string& V, const bool& flag) {
	if (flag || !tag.second) {
		// 遍历AVALUE
		for (int i = 0; i < this->AVALUE[V].size(); i++) {
			if (this->RVALUE.find(this->AVALUE[V][i]) != this->RVALUE.end()) {
				string t = this->AVALUE[V][i];
				for (int j = 0; j < this->RVALUE[t].size(); j++) {
					if (this->RVALUE[t][j].first == V) {
						this->RVALUE[t].erase(find(this->RVALUE[t].begin(), this->RVALUE[t].end(), this->RVALUE[t][j]));
						break;
					}
				}
			}
		}
		if (tag.second) {
			this->AVALUE[V] = vector<string>{ R };
			this->RVALUE[R].push_back(pair<string, int>(V, tag.first));
		}
		else {
			this->AVALUE.erase(V);
		}
	}
	else {
		bool is_find = false;
		for (int i = 0; i < this->RVALUE[R].size(); i++) { // 遍历RVALUE
			if (this->RVALUE[R][i].first == V) { // 判断是否与reg相等
				is_find = true;
				this->RVALUE[R][i].second = tag.first;
				break;
			}
		}
		if (!is_find) { 
			this->RVALUE[R].push_back(pair<string, int>(V, tag.first));
		}

		if (this->AVALUE.find(V) == this->AVALUE.end()) {
			this->AVALUE[V] = vector<string>{ R }; // arg不存在于AVALUE，新建arg
		}
		else {
			if (find(this->AVALUE[V].begin(), this->AVALUE[V].end(), R) == this->AVALUE[V].end()) {
				this->AVALUE[V].push_back(R);
			}
		}
	}
}

string ObjectCodeGenerater::getReg(const string& result) {
	string R;
	bool has_R = false;
	//result本身有寄存器
	if (AVALUE.find(result) != AVALUE.end() && AVALUE[result].size() > 0) {
		for (auto i = 0; i < AVALUE[result].size(); i++) {
			if (AVALUE[result][i] != "M") {
				R = AVALUE[result][i];
				has_R = true;
				break;
			}
		}
	}
	if (!has_R) {
		for (map<string, vector<pair<string, int>>>::iterator iter = RVALUE.begin(); iter != RVALUE.end(); iter++) {
			if (iter->second.size() == 0) {
				R = iter->first;
				return R;
			}
		}
		//choose R which will be used in longest time
		int farthest_R = -1;
		for (map<string, vector<pair<string, int>>>::iterator iter = RVALUE.begin(); iter != RVALUE.end(); iter++) {
			int closest_V = INT_MAX;
			for (auto i = 0; i < iter->second.size(); i++) {
				if (iter->second[i].second < closest_V)
					closest_V = iter->second[i].second;
			}
			if (closest_V > farthest_R) {
				farthest_R = closest_V;
				R = iter->first;
			}
		}
	}
	for (auto i = 0; i < RVALUE[R].size(); i++) {
		string V = RVALUE[R][i].first;
		if (AVALUE[V].size() == 1 && AVALUE[V][0] == R) {
			//save variable V
			if (V[0] == 'G')
				this->emit("sw " + R + ",data+" + to_string(stoi(V.substr(1))));
			else if (V[0] == 'V')
				this->emit("sw " + R + ",stack+" + to_string(4 + stoi(V.substr(1))) + "($fp)");
			else if (V[0] == 'T')
				this->emit("sw " + R + ",temp+" + to_string(4 * stoi(V.substr(1))));
			else {
				cerr << "ERROR: AVALUE中出现意外的变量名:" << V << endl;
				exit(ERROR_OBJECTCODEGENERATER);
			}
		}
		//delete R from AVALUE
		vector<string>::iterator Ritor = find(AVALUE[V].begin(), AVALUE[V].end(), R);
		AVALUE[V].erase(Ritor);
		//add memroy address to AVALUE
		if (find(AVALUE[V].begin(), AVALUE[V].end(), "M") == AVALUE[V].end())
			AVALUE[V].push_back("M");
	}
	//delete all V from RVALUE
	RVALUE[R].clear();
	return R;
}

void updateMessageItem(const int& i, const string& arg, map<string, Tag>& messageLink, const BlockItem& b, MessageTableItem& item, const int& p) {
	if (arg[0] == 'G' || arg[0] == 'V' || arg[0] == 'T') {
		if (messageLink.find(arg) == messageLink.end()) {
			if (arg[0] == 'G' || find(b.waitVar.begin(), b.waitVar.end(), arg) != b.waitVar.end()) {
				messageLink[arg] = Tag(INT_MAX, true);
			}
			else {
				messageLink[arg] = Tag(0, false);
			}
		}
		item.tags[p] = messageLink[arg];
		if (p == MessageTableItem::resultPos) {
			messageLink[arg] = Tag(0, false);
		}
		else {
			messageLink[arg] = Tag(i, true);
		}
	}
}

vector<MessageTableItem> ObjectCodeGenerater::generateMessageTable(const int& blkno) {
	vector<MessageTableItem> messageTable;
	map<string, Tag> messageLink;

	// 遍历blkno对应代码块
	for (int i = block[blkno].end; i >= block[blkno].begin; i--) {
		Quaternion q = intermediateCode[i];
		MessageTableItem item(i, q);
		updateMessageItem(i, q.arg1, messageLink, block[blkno], item, MessageTableItem::arg1Pos);
		updateMessageItem(i, q.arg2, messageLink, block[blkno], item, MessageTableItem::arg2Pos);
		updateMessageItem(i, q.result, messageLink, block[blkno], item, MessageTableItem::resultPos);
		messageTable.push_back(item);
	}
	reverse(messageTable.begin(), messageTable.end());
	return messageTable;
}

void ObjectCodeGenerater::preGenerate() {
	for (int i = 0; i < ObjectCodeHeadSize; i++) {
		if (i == ObjectCodeHeadDataSpacePos) {
			string code = this->ObjectCodeHead[i] + to_string(this->dataBufferSize);
			this->emit(code);
		}
		else if (i == ObjectCodeHeadStackSpacePos) {
			string code = this->ObjectCodeHead[i] + to_string(this->stackBufferSize);
			this->emit(code);
		}
		else if (i == ObjectCodeHeadTempSpacePos) {
			string code = this->ObjectCodeHead[i] + to_string(this->tempBufferSize);
			this->emit(code);
		}
		else {
			this->emit(this->ObjectCodeHead[i]);
		}
	}
}

string ObjectCodeGenerater::allocateRegGVT(const string& arg, const char& c, const int& i) {
	string reg = i == 1 ? "$t8" : "$t9";
	if (this->AVALUE.find(arg) == this->AVALUE.end()) { // AVALUE中不存在arg的key
		this->AVALUE[arg] = vector<string>{ "M" };
	}
	else if (this->AVALUE[arg].size() == 0) {
		cerr << "ERROR: 目标代码生成器找不到中间代码的arg地址" << endl;
		exit(ERROR_OBJECTCODEGENERATER);
	}

	if (this->AVALUE[arg].size() == 1 && this->AVALUE[arg][0] == "M") {
		if (c == 'G') {
			this->emit("lw " + reg + ",data + " + to_string(stoi(arg.substr(1))));
		}
		else if (c == 'V') {
			this->emit("lw " + reg + ",stack+" + to_string(4 + stoi(arg.substr(1))) + "($fp)");
		}
		else if (c == 'T') {
			this->emit("lw " + reg + ",temp+" + to_string(4 * stoi(arg.substr(1))));
		}
		else {
			cerr << "ERROR: 目标代码生成器找不到中间代码的arg地址" << endl;
			exit(ERROR_OBJECTCODEGENERATER);
		}
		return reg;
	}
	else {
		for (int i = 0; i < this->AVALUE[arg].size(); i++) {
			if (this->AVALUE[arg][i] != "M") {
				reg = this->AVALUE[arg][i];
			}
		}
		return reg;
	}
}

string ObjectCodeGenerater::allocateReg(const string& arg, const string& op, const int& i, const string& regAllocated) {
	string reg = i == 1 ? "$t8" : "$t9";
	if ((i == 1 && (arg == "" || op == "=[]")) || (i == 2 && arg == "")) {
		return "";
	}
	else if (arg[0] == '$') {
		return arg;
	}
	else if (arg == "[$sp]") {
		return "stack($sp)";
	}
	else if (is_num(arg)) {
		if ((i == 1 && op == "+") || (i == 2 && op == "+" && !is_num(regAllocated))) {
			return arg;
		}
		else {
			this->emit("addi "+ reg + ",$zero," + arg);
			return reg;
		}
	}
	else if (arg[0] == 'G') {
		return this->allocateRegGVT(arg, 'G', i);
	}
	else if (arg[0] == 'V') {
		return this->allocateRegGVT(arg, 'V', i);
	}
	else if (arg[0] == 'T') {
		return this->allocateRegGVT(arg, 'T', i);
	}
}

void ObjectCodeGenerater::generateNop() {
	this->emit("nop");
}

void ObjectCodeGenerater::generateJ(bool& jEnd, const Quaternion& q) {
	jEnd = true;
	this->endBlock();
	this->emit("j " + q.result);
}

void ObjectCodeGenerater::generateJal(bool& jEnd, const Quaternion& q) {
	jEnd = true;
	this->emit("jal " + q.result);
}

void ObjectCodeGenerater::generateBreak(bool& jEnd) {
	jEnd = true;
	this->endBlock();
	this->emit("break");
}

void ObjectCodeGenerater::generateRet(bool& jEnd) {
	jEnd = true;
	this->endBlock();
	this->emit("jr $ra");
}

void ObjectCodeGenerater::generateJnz(bool& jEnd, const Quaternion& q, const string& reg1, const MessageTableItem& item) {
	jEnd = true;
	if (this->RVALUE.find(reg1) != this->RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg1Pos], reg1, q.arg1, false);
	}
	this->endBlock();
	this->emit("bne " + reg1 + ",$zero," + q.result);
}

void ObjectCodeGenerater::generateJcmp(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item, const int& type) {
	jEnd = true;

	switch (type) {
		case Type_Jl:
			this->emit("addi $t8," + reg1 + ",1");
			this->emit("sub $t9," + reg2 + ",$t8");
			break;
		case Type_Jle:
			this->emit("sub $t9," + reg2 + "," + reg1);
			break;
		case Type_Jg:
			this->emit("addi $t9," + reg2 + ",1");
			this->emit("sub  $t8," + reg1 + ",$t9");
			break;
		case Type_Jge:
			this->emit("sub $t8," + reg1 + "," + reg2);
			break;
		default:
			break;
	}
	
	if (this->RVALUE.find(reg1) != this->RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg1Pos], reg1, q.arg1, false);
	}
	if (this->RVALUE.find(reg2) != this->RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg2Pos], reg2, q.arg2, false);
	}

	this->endBlock();

	switch (type) {
		case Type_Jl:
		case Type_Jle:
			this->emit("bgez $t9," + q.result);
			break;
		case Type_Jg:
		case Type_Jge:
			this->emit("bgez $t8," + q.result);
			break;
		case Type_Jeq:
			this->emit("beq " + reg1 + "," + reg2 + "," + q.result);
			break;
		case Type_Jne:
			this->emit("bne " + reg1 + "," + reg2 + "," + q.result);
			break;
		default:
			break;
	}
}

void ObjectCodeGenerater::generateJl(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateJcmp(jEnd, q, reg1, reg2, item, Type_Jl);
}

void ObjectCodeGenerater::generateJle(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateJcmp(jEnd, q, reg1, reg2, item, Type_Jle);
}

void ObjectCodeGenerater::generateJg(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateJcmp(jEnd, q, reg1, reg2, item, Type_Jg);
}

void ObjectCodeGenerater::generateJge(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateJcmp(jEnd, q, reg1, reg2, item, Type_Jge);
}

void ObjectCodeGenerater::generateJeq(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateJcmp(jEnd, q, reg1, reg2, item, Type_Jeq);
}

void ObjectCodeGenerater::generateJne(bool& jEnd, const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateJcmp(jEnd, q, reg1, reg2, item, Type_Jne);
}

void ObjectCodeGenerater::generateAssign(const Quaternion& q, const string& reg1, const MessageTableItem& item) {
	string R;
	if (q.result[0] == '$') {
		R = q.result;
		if (q.arg1 == "[$sp]") {
			this->emit("lw " + R + "," + reg1);
		}
		else {
			this->emit("add " + R + ",$zero," + reg1);
		}
	}
	else if (q.result == "[$sp]") {
		R = "stack($sp)";
		if (q.arg1 == "[$sp]") {
			cerr << "ERROR: 从[$sp]到[$sp]的赋值" << endl;;
			exit(ERROR_OBJECTCODEGENERATER);
		}
		else {
			this->emit("sw " + reg1 + "," + R);
		}
	}
	else {
		R = getReg(q.result);
		if (q.arg1 == "[$sp]") {
			this->emit("lw " + R + "," + reg1);
		}
		else {
			this->emit("add " + R + ",$zero," + reg1);
		}
	}
	if (RVALUE.find(R) != RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::resultPos], R, q.result, true);
	}
	if (RVALUE.find(reg1) != RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg1Pos], reg1, q.arg1, false);
	}
}

void ObjectCodeGenerater::generateIndexAssign(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	string base = q.result;
	if (q.result[0] == 'G') {
		this->emit("sll $t9," + reg2 + ",2");
		this->emit("addi $t9,$t9," + base.substr(1));
		this->emit("sw " + reg1 + ",data" + "($t9)");
	}
	else if (q.result[0] == 'V') {
		this->emit("sll $t9," + reg2 + ",2");
		this->emit("addi $t9,$t9," + base.substr(1));
		this->emit("addi $t9,$t9,4");
		this->emit("add $t9,$t9,$fp");
		this->emit("sw " + reg1 + ",stack" + "($t9)");
	}
	else if (q.result[0] == 'T') {
		this->emit("addi $t9," + reg2 + "," + base.substr(1));
		this->emit("sll $t9,$t9,2");
		this->emit("sw " + reg1 + ",temp" + "($t9)");
	}
	else {
		cerr << "ERROR: []=的左部result标识符不合法\n";
		exit(ERROR_OBJECTCODEGENERATER);
	}

	if (this->RVALUE.find(reg1) != this->RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg1Pos], reg1, q.arg1, false);
	}
	if (this->RVALUE.find(reg2) != this->RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg2Pos], reg2, q.arg2, false);
	}
}

void ObjectCodeGenerater::generateAssignIndex(const Quaternion& q, const string& reg2, const MessageTableItem& item) {
	string R;
	if (q.result[0] == '$') {
		R = q.result;
	}
	else if (q.result == "[$sp]") {
		R = "stack($sp)";
	}
	else {
		R = getReg(q.result);
	}

	if (q.arg1[0] == 'G') {
		this->emit("sll $t9," + reg2 + ",2");
		this->emit("addi $t9,$t9," + q.arg1.substr(1));
		this->emit("lw " + R + ",data($t9)");
	}
	else if (q.arg1[0] == 'V') {
		this->emit("sll $t9," + reg2 + ",2");
		this->emit("addi $t9,$t9," + q.arg1.substr(1));
		this->emit("addi $t9,$t9,4");
		this->emit("add $t9,$t9,$fp");
		this->emit("lw " + R + ",stack($t9)");
	}
	else if (q.arg1[0] == 'T') {
		this->emit("addi $t9," + reg2 + "," + q.arg1.substr(1));
		this->emit("sll $t9,$t9,2");
		this->emit("lw " + R + ",temp($t9)");
	}
	else {
		cerr << "ERROR: =[]的右部arg1标识符不合法\n";
		exit(-1);
	}

	if (RVALUE.find(R) != RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::resultPos], R, q.result, true);
	}
	if (this->RVALUE.find(reg2) != this->RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg2Pos], reg2, q.arg2, false);
	}
}

void ObjectCodeGenerater::generateCalc(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item, const int& type) {
	string R;
	if (q.result[0] == '$') {
		R = q.result;
	}
	else if (q.result == "[$sp]") {
		R = "stack($sp)";
	}
	else {
		R = getReg(q.result);
	}

	switch (type) {
		case Type_Plus:
			if (is_num(reg1)) {
				this->emit("addi " + R + "," + reg2 + "," + reg1);
			}
			else if (is_num(reg2)) {
				this->emit("addi " + R + "," + reg1 + "," + reg2);
			}
			else {
				this->emit("add " + R + "," + reg1 + "," + reg2);
			}
			break;
		case Type_Minus:
			this->emit("sub " + R + "," + reg1 + "," + reg2);
			break;
		case Type_And:
			this->emit("and " + R + "," + reg1 + "," + reg2);
			break;
		case Type_Or:
			this->emit("or " + R + "," + reg1 + "," + reg2);
			break;
		case Type_Xor:
			this->emit("xor " + R + "," + reg1 + "," + reg2);
			break;
		case Type_Mul:
			this->emit("mul " + R + "," + reg1 + "," + reg2);
			break;
		case Type_Divide:
			this->emit("div " + R + "," + reg1 + "," + reg2);
			this->emit("mflo " + R);
			break;
	}
	

	if (RVALUE.find(R) != RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::resultPos], R, q.result, true);
	}
	if (this->RVALUE.find(reg1) != this->RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg1Pos], reg1, q.arg1, false);
	}
	if (this->RVALUE.find(reg2) != this->RVALUE.end()) {
		this->updateVALUE(item.tags[MessageTableItem::arg2Pos], reg2, q.arg2, false);
	}
}

void ObjectCodeGenerater::generatePlus(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateCalc(q, reg1, reg2, item, Type_Plus);
}

void ObjectCodeGenerater::generateMinus(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateCalc(q, reg1, reg2, item, Type_Minus);
}

void ObjectCodeGenerater::generateAnd(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateCalc(q, reg1, reg2, item, Type_And);
}

void ObjectCodeGenerater::generateOr(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateCalc(q, reg1, reg2, item, Type_Or);
}

void ObjectCodeGenerater::generateXor(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateCalc(q, reg1, reg2, item, Type_Xor);
}

void ObjectCodeGenerater::generateMul(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateCalc(q, reg1, reg2, item, Type_Mul);
}

void ObjectCodeGenerater::generateDivide(const Quaternion& q, const string& reg1, const string& reg2, const MessageTableItem& item) {
	this->generateCalc(q, reg1, reg2, item, Type_Divide);
}

void ObjectCodeGenerater::generateObjectCode() {
	this->preGenerate(); // 生成代码头

	// 遍历所有代码块
	for (int blkno = 0; blkno < block.size(); blkno++) {
		vector<MessageTableItem> messageTable = generateMessageTable(blkno); // 产生信息表
		bool jEnd = false;
		// 遍历信息表
		for (int i = 0; i < messageTable.size(); i++) {
			
			Quaternion q = messageTable[i].q;

			// 为中间代码的操作数分配寄存器
			string reg1 = this->allocateReg(q.arg1, q.op, 1);
			string reg2 = this->allocateReg(q.arg2, q.op, 2, reg1);
			
			// 根据op生成目标代码
			if (q.op[0] == 'F' || q.op[0] == 'L') {
				this->emit(q.op + ":");
			}
			else if (q.op == OP_NOP) {
				this->generateNop();
			}
			else if (q.op == OP_J) {
				this->generateJ(jEnd, q);
			}
			else if (q.op == OP_JAL) {
				this->generateJal(jEnd, q);
			}
			else if (q.op == OP_BREAK) {
				this->generateBreak(jEnd);
			}
			else if (q.op == OP_RET) {
				this->generateRet(jEnd);
			}
			else if (q.op == OP_JNZ) {
				this->generateJnz(jEnd, q, reg1, messageTable[i]);
			}
			else if (q.op == OP_JL) {
				this->generateJl(jEnd, q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_JLE) {
				this->generateJle(jEnd, q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_JG) {
				this->generateJg(jEnd, q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_JGE) {
				this->generateJge(jEnd, q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_JEQ) {
				this->generateJeq(jEnd, q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_JNE) {
				this->generateJne(jEnd, q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_ASSIGN) {
				this->generateAssign( q, reg1, messageTable[i]);
			}
			else if (q.op == OP_INDEXASSIGN) {
				this->generateIndexAssign( q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_ASSIGNINDEX) {
				this->generateAssignIndex( q,  reg2, messageTable[i]);
			}
			else if (q.op == OP_PLUS) {
				this->generatePlus(q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_MINUS) {
				this->generateMinus(q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_AND) {
				this->generateAnd(q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_OR) {
				this->generateOr(q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_XOR) {
				this->generateXor(q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_MUL) {
				this->generateMul(q, reg1, reg2, messageTable[i]);
			}
			else if (q.op == OP_DIVIDE) {
				this->generateDivide(q, reg1, reg2, messageTable[i]);
			}
			else {
				cerr << "ERROR: 目标代码生成器遇到非法的中间代码" << endl;
				exit(ERROR_OBJECTCODEGENERATER);
			}
		}
		if (!jEnd) {
			this->endBlock();
		}
	}
}

void ObjectCodeGenerater::printObjectCode(const string& filename, const bool& isOut) {
	fstream fout(filename, ios::out);
	if (!fout.is_open()) {
		cerr << "无法打开目标代码文件" << filename << endl;
		exit(ERROR_OPEN_FILE);
	}
	for (int i = 0; i < this->objectCode.size(); i++) {
		if (isOut) {
			cout << this->objectCode[i] << endl;
		}
		fout << this->objectCode[i] << endl;
	}
	fout.close();
}

/*============================== ObjectCodeGenerater ==============================*/
