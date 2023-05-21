#include "Optimizer.h"

/*============================== BlockItem ==============================*/

void BlockItem::init(const int& b) {
	this->begin = b;
	this->waitVar.clear();
	this->activeVar.clear();
}

/*============================== BlockItem ==============================*/



/*============================== DAG项 ==============================*/

DAGItem::DAGItem() {
	this->flag = false;
	this->isLeft = false;

	this->parent = TopoStructInit;
	memset(this->children, TopoStructInit, sizeof(this->children));
}

DAGItem::DAGItem(const Quaternion& q) {
	this->flag = false;

	this->parent = TopoStructInit;
	memset(this->children, TopoStructInit, sizeof(this->children));

	this->isLeft = true;
	this->isLeaf = true;
	this->quaternion = q;
}

DAGItem::DAGItem(const string& value, const bool& isLeaf) {
	this->flag = false;
	this->isLeft = false;

	this->parent = TopoStructInit;
	memset(this->children, TopoStructInit, sizeof(this->children));

	this->value = value;
	this->isLeaf = isLeaf;
}

DAGItem::DAGItem(const string& op, const int& child0, const int& child1, const int& child2) {
	this->flag = false;
	this->isLeft = false;

	this->parent = TopoStructInit;
	memset(this->children, TopoStructInit, sizeof(this->children));

	this->isLeaf = false;
	this->op = op;
	this->children[0] = child0;
	this->children[1] = child1;
	this->children[2] = child2;
}

DAGItem::~DAGItem() {

}

bool DAGItem::operator == (const DAGItem& d) {

	for (int i = 0; i < MaxChildrenNum; i++) {
		if (this->children[i] != d.children[i]) {
			return false;
		}
	}
	if (this->labelList.size() != d.labelList.size()) {
		return false;
	}
	for (unsigned int i = 0; i < this->labelList.size(); i++) {
		if (this->labelList[i] != d.labelList[i]) {
			return false;
		}
	}

	return this->isLeaf == d.isLeaf && this->value == d.value && this->op == d.op && this->parent == d.parent;
}

/*============================== DAG项 ==============================*/



/*============================== Optimizer ==============================*/

Optimizer::Optimizer(const map<int, string>& varTable, SymbolTable* gSymbolTable, const vector<Quaternion>& intermediateCode) {
	this->tmpCnt = 0;

	this->varTable = varTable;
	this->gSymbolTable = gSymbolTable;
	this->intermediateCode = intermediateCode;

	this->optimizeCnt = 0;
	this->optimizeRate = 0;
}

Optimizer::~Optimizer() {

}

bool Optimizer::judgeDAGNodeIsNum(const int& no, const vector<DAGItem>& DAG) {
	return DAG[no].isLeaf && is_num(DAG[no].value);
}

int Optimizer::operateBopC(const string& op, const int& Bno, const int& Cno, const vector<DAGItem>& DAG) {
	int B = stoi(DAG[Bno].value), C = stoi(DAG[Cno].value); // 引用B和C节点的value
	if (op == "+") {
		return B + C;
	}
	else if (op == "-") {
		return B - C;
	}
	else if (op == "&") {
		return B & C;
	}
	else if (op == "|") {
		return B | C;
	}
	else if (op == "^") {
		return B ^ C;
	}
	else if (op == "*") {
		return B * C;
	}
	else if (op == "/") {
		return B / C;
	}
	else {
		cerr << "ERROR: 优化器处理op错误!" << endl;
		exit(ERROR_OPTIMIZER);
	}
}

string Optimizer::newtemp() {
	return "S" + to_string(this->tmpCnt++);
}

void Optimizer::utilizeChildren(vector<DAGItem>& DAG, int now) {
	DAG[now].flag = true;
	if (!DAG[now].isLeaf) {
		if (DAG[now].children[1] != DAGItem::TopoStructInit) {
			utilizeChildren(DAG, DAG[now].children[1]);
		}
		if (DAG[now].children[0] != DAGItem::TopoStructInit) {
			utilizeChildren(DAG, DAG[now].children[0]);
		}
		if (DAG[now].children[2] != DAGItem::TopoStructInit) {
			utilizeChildren(DAG, DAG[now].children[2]);
		}
	}
}

string Optimizer::getChildValue(const int& i, const int& p, const vector<DAGItem>& DAG) {
	if (DAG[DAG[i].children[p]].isLeaf) {
		return DAG[DAG[i].children[p]].value[0] == '-' ? DAG[DAG[i].children[p]].value.substr(1) : DAG[DAG[i].children[p]].value;
	}
	else {
		return DAG[DAG[i].children[p]].labelList[0];
	}
}

/*
* Function Name:        preGenerateBlock
* Function Description: 1. 静态语义检查main函数
*                       2. 为j和jal的目标地址进行标记，方便进行块的划分
*/
void Optimizer::preGenerateBlock() {
	// 检查main函数
	int mainId = NOT_FOUND;
	int mainOffset;
	for (auto i = this->varTable.begin(); i != this->varTable.end(); i++) {
		if (i->second == "main" && mainId == NOT_FOUND) {
			mainId = i->first;
		}
		else if (i->second == "main" && mainId != NOT_FOUND) {
			cerr << "ERROR: 静态语义分析错误：定义了多个main函数" << endl;
			exit(ERROR_OPTIMIZER_REDEFINEMAIN);
		}
	}
	if (mainId == NOT_FOUND) {
		cerr << "ERROR: 静态语义分析错误：未定义main函数" << endl;
		exit(ERROR_OPTIMIZER_NOTDEFINEMAIN);
	}

	// 在gSymbolTable中找到offset
	for (unsigned int i = 0; i < this->gSymbolTable->table.size(); i++) {
		if (this->gSymbolTable->table[i].id == mainId) {
			mainOffset = gSymbolTable->table[i].offset;
			break;
		}
	}

	labelMap[mainOffset] = "Fmain";

	int varLabelCnt = 0;
	int funcLabelCnt = 0;
	// 遍历中间代码
	for (unsigned int i = 0; i < intermediateCode.size(); i++) {
		Quaternion* q = &intermediateCode[i]; // 定义指针，简化引用长度

		if (q->op == "jal") { // jal四元式，函数调用
			// 判断四元式result是否第一次出现
			if (labelMap.find(stoi(q->result)) == labelMap.end()) {
				labelMap[stoi(q->result)] = "F" + to_string(funcLabelCnt++);
			}
			q->result = labelMap[stoi(q->result)];
		}
		else if (q->op[0] == 'j') { // j四元式
			if (labelMap.find(stoi(q->result)) == labelMap.end()) {
				labelMap[stoi(q->result)] = "L" + to_string(varLabelCnt++);
			}
			q->result = labelMap[stoi(q->result)];
		}
	}

	vector<Quaternion> code;
	for (unsigned int i = 0; i < intermediateCode.size(); i++) {
		Quaternion q = intermediateCode[i];
		if (labelMap.find(i) != labelMap.end()) {
			Quaternion label = Quaternion(labelMap[i], "", "", "");
			code.push_back(label);
		}
		code.push_back(q);
	}

	intermediateCode = code;
}

void Optimizer::generateBlock() {
	BlockItem item;

	// 遍历中间代码
	for (int i = 0; i < (int)intermediateCode.size(); i++) {

		Quaternion q = intermediateCode[i]; // 引用，减轻后面代码长度

		bool jumpFlag = (i - 1 >= 0 && (intermediateCode[i - 1].op[0] == 'j' || intermediateCode[i - 1].op == "ret")); // 转移标志
		if (i == 0 || q.op[0] == 'L' || q.op[0] == 'F' || jumpFlag) { // 产生新的代码块
			item.init(i);
		}
		
		// 更新块信息
		if ((q.result[0] == 'V' || q.result[0] == 'T') && find(item.waitVar.begin(), item.waitVar.end(), q.result) == item.waitVar.end()) { // V->变量
			item.waitVar.push_back(q.result);
		}
		if ((q.arg1[0] == 'V' || q.arg1[0] == 'T') && find(item.waitVar.begin(), item.waitVar.end(), q.arg1) == item.waitVar.end()) {
			item.waitVar.push_back(q.arg1);
		}
		if ((q.arg2[0] == 'V' || q.arg2[0] == 'T') && find(item.waitVar.begin(), item.waitVar.end(), q.arg2) == item.waitVar.end()) {
			item.waitVar.push_back(q.arg2);
		}
		if ((q.arg1[0] == 'V' || q.arg1[0] == 'T') && find(item.activeVar.begin(), item.activeVar.end(), q.arg1) == item.activeVar.end()) {
			item.activeVar.push_back(q.arg1);
		}
		if ((q.arg2[0] == 'V' || q.arg2[0] == 'T') && find(item.activeVar.begin(), item.activeVar.end(), q.arg2) == item.activeVar.end()) {
			item.activeVar.push_back(q.arg2);
		}

		// 块结束
		bool enterFlag = ((i + 1 < (int)intermediateCode.size() && (intermediateCode[i + 1].op[0] == 'L' || intermediateCode[i + 1].op[0] == 'F')) || q.op[0] == 'j' || q.op == "ret" || q.op == "break");
		if (enterFlag) {
			item.end = i;
			block.push_back(item);
		}
	}

	map<string, int> labelLoop;
	for (unsigned int i = 0; i < intermediateCode.size(); i++) {
		if (intermediateCode[i].op[0] == 'L') {
			labelLoop[intermediateCode[i].op] = i;
		}
	}

	for (unsigned int i = 0; i < block.size(); i++) {
		BlockItem* bip = &block[i]; // 引用，减轻后面代码长度

		if (intermediateCode[bip->end].op == "ret") { // 该基本块是函数返回块
			// 清除赋值语句中的变量
			bip->uselessVar = bip->waitVar;
			bip->waitVar.clear();
		}
		else {
			// 找到本块最后一条语句
			int p = block[i + 1].begin;
			int prep = p - 1;

			while (prep < (int)intermediateCode.size()) {
				if (labelLoop.find(intermediateCode[prep].result) != labelLoop.end() && labelLoop[intermediateCode[prep].result] < p) {
					p = labelLoop[intermediateCode[prep].result];
					prep = labelLoop[intermediateCode[prep].result];
				}
				prep++;
			}

			vector<string> realWaitVar;

			while (p < (int)intermediateCode.size() && intermediateCode[p].op[0] != 'F') {
				if (find(bip->waitVar.begin(), bip->waitVar.end(), intermediateCode[p].arg1) != bip->waitVar.end() && find(realWaitVar.begin(), realWaitVar.end(), intermediateCode[p].arg1) == realWaitVar.end()) {
					realWaitVar.push_back(intermediateCode[p].arg1);
				}
				if (find(bip->waitVar.begin(), bip->waitVar.end(), intermediateCode[p].arg2) != bip->waitVar.end() && find(realWaitVar.begin(), realWaitVar.end(), intermediateCode[p].arg2) == realWaitVar.end()) {
					realWaitVar.push_back(intermediateCode[p].arg2);
				}
				p++;
			}

			for (unsigned int j = 0; j < bip->waitVar.size(); j++) {
				if (find(realWaitVar.begin(), realWaitVar.end(), bip->waitVar[j]) == realWaitVar.end()) {
					bip->uselessVar.push_back(bip->waitVar[j]);
				}
			}
			bip->waitVar = realWaitVar;
		}
	}
}

void Optimizer::findOrCreateDAGNodeByValue(int& no, bool& flag, vector<DAGItem>& DAG, const string& v) {
	// 查找是否有该结点
	no = NOT_FOUND;
	for (unsigned int i = 0; i < DAG.size(); i++) {
		if ((DAG[i].isLeaf && DAG[i].value == v) || find(DAG[i].labelList.begin(), DAG[i].labelList.end(), v) != DAG[i].labelList.end()) {
			no = i;
			flag = false;
			break;
		}
	}

	// 创建新结点
	if (no == NOT_FOUND) {
		DAGItem node(v, true);
		no = DAG.size();
		flag = true;
		DAG.push_back(node);
	}
}

void Optimizer::findOrCreateDAGNodeByChild(int& no, const string& op, vector<DAGItem>& DAG, const int& Bno, const int& Cno) {
	// 查找是否有该结点
	no = NOT_FOUND;
	for (unsigned int i = 0; i < DAG.size(); i++) {
		if (!DAG[i].isLeaf && DAG[i].children[0] == Bno && DAG[i].op == op && (Cno == DAGItem::TopoStructInit || (DAG[i].children[1] != DAGItem::TopoStructInit && DAG[i].children[1] == Cno))) {
			no = i;
			break;
		}
	}

	// 创建新结点
	if (no == NOT_FOUND) {
		DAGItem node(op, Bno, Cno, DAGItem::TopoStructInit);
		no = DAG.size();
		DAG.push_back(node);
		DAG[Bno].parent = no;
		if (Cno != DAGItem::TopoStructInit) {
			DAG[Cno].parent = no;
		}
	}
}

void Optimizer::deleteNewDAGNode(const int& no, const bool& flag, vector<DAGItem>& DAG) {
	if (flag) {
		DAG.erase(find(DAG.begin(), DAG.end(), DAG[no]));
	}
}

vector<DAGItem> Optimizer::generateDAG(const int& blkno) {
	vector<DAGItem> DAG;

	// 遍历代码块中的中间代码
	BlockItem* bip = &block[blkno]; // 引用，减轻后面代码长度
	for (int i = bip->begin; i <= bip->end; i++) {
		// 引用，减轻后面代码长度
		Quaternion q = intermediateCode[i];
		string op = intermediateCode[i].op;
		string B = intermediateCode[i].arg1;
		string C = intermediateCode[i].arg2;
		string A = intermediateCode[i].result;

		int quaternionType; // 中间代码式子类型
		if (op == "nop" || op[0] == 'F' || op[0] == 'L' || op == "j<" || op == "j<=" || op == "j>" || op == "j>=" || op == "j==" || op == "j!=" || op == "jnz" || op == "j" || op == "jal" || op == "break" || op == "ret") {
			quaternionType = -1;
		}
		else if (A[0] == '$' || A == "[$sp]") {
			quaternionType = -1;
		}
		else if (op == ":=") {
			quaternionType = 0;
		}
		else if (op == "=[]") {
			quaternionType = 2;
		}
		else if (op == "[]=") {
			quaternionType = 3;
		}
		else {
			quaternionType = 2;
		}

		// 判断该中间代码是否需要DAG转化
		if (quaternionType == -1) {
			DAGItem item(q);
			DAG.push_back(item);
			
			// 更新叶子节点的值
			if (A[0] == '$' || A == "[$sp]") {
				for (unsigned int j = 0; j < DAG.size(); j++) {
					if (DAG[j].isLeaf && DAG[j].value == A) {
						DAG[j].value = "-" + A;
						break;
					}
				}
			}
			continue;
		}
		
		/* DAG生成状态机*/
		int state = DAG_STATE_PrepareNode; // 定义状态机状态
		int n, Ano, Bno, Cno;
		bool newN, newA, newB, newC;

		while (state > 0) {
			switch (state) {
				case DAG_STATE_PrepareNode: {
					
					this->findOrCreateDAGNodeByValue(Bno, newB, DAG, B); // 查找（不存在则创建）B节点

					// 判断四元式类型
					switch (quaternionType) {
						case 0: { // 0型式
							n = Bno;
							state = DAG_STATE_RemoveUselessAssignments;
							break;
						}
						case 1: { // 1型式
							state = DAG_STATE_CombineKnown1;
							break;
						}
						case 2: { // 2型式
							this->findOrCreateDAGNodeByValue(Cno, newC, DAG, C); // 查找（不存在则创建）C节点
							state = DAG_STATE_CombineKnown2;
							break;
						}
						case 3: {
							this->findOrCreateDAGNodeByValue(Cno, newC, DAG, C); // 查找（不存在则创建）C节点
							this->findOrCreateDAGNodeByValue(Ano, newA, DAG, A); // 查找（不存在则创建）A节点
							// 产生新的DAG节点
							DAGItem node(op, Bno, Cno, Ano);
							n = DAG.size();
							DAG.push_back(node);
							// 更新拓扑结构
							DAG[Bno].parent = n;
							DAG[Cno].parent = n;
							DAG[Ano].parent = n;
							//
							for (unsigned int j = 0; j < DAG.size(); j++) {
								if (DAG[j].isLeaf && DAG[j].value == A) {
									DAG[j].value = "-" + A;
									break;
								}
							}
							state = DAG_STATE_END;
							break;
						}
						default: {
							state = DAG_STATE_END;
							break;
						}
					}
					break;
				}
				case DAG_STATE_CombineKnown1: {
					state = DAG_STATE_CombineKnown3 ? this->judgeDAGNodeIsNum(Bno, DAG) : DAG_STATE_FindCommonExpression1; // 判断B节点是否是常数节点
					break;
				}
				case DAG_STATE_CombineKnown2: {
					state = (this->judgeDAGNodeIsNum(Bno, DAG) && this->judgeDAGNodeIsNum(Cno, DAG)) ? DAG_STATE_CombineKnown4 : DAG_STATE_FindCommonExpression2; // 判断B和C节点是否是常数节点
					break;
				}
				case DAG_STATE_CombineKnown3: { 
					// 执行 op B
					// 不存在单目运算，直接退出
					state = DAG_STATE_END;
					break;
				}
				case DAG_STATE_CombineKnown4: {
					// 执行 A op B
					int P = this->operateBopC(op, Bno, Cno, DAG);
					// 删除构造的B和C节点
					DAGItem recB = DAG[Bno], recC = DAG[Cno];
					this->deleteNewDAGNode(Bno, newB, DAG);
					this->deleteNewDAGNode(Cno, newC, DAG);
					// 查找（不存在则创建）R节点
					this->findOrCreateDAGNodeByValue(n, newN, DAG, to_string(P));
					state = DAG_STATE_RemoveUselessAssignments;
					break;
				}
				case DAG_STATE_FindCommonExpression1: {
					this->findOrCreateDAGNodeByChild(n, op, DAG, Bno); // 查找（不存在则创建）节点（只存在B唯一子节点）
					state = DAG_STATE_RemoveUselessAssignments;
					break;
				}
				case DAG_STATE_FindCommonExpression2: {
					this->findOrCreateDAGNodeByChild(n, op, DAG, Bno, Cno); // 查找（不存在则创建）节点（存在BC子节点）
					state = DAG_STATE_RemoveUselessAssignments;
					break;
				}
				case DAG_STATE_RemoveUselessAssignments: {
					for (unsigned int j = 0; j < DAG.size(); j++) {
						if (DAG[j].isLeaf && DAG[j].value == A) { // A是叶子节点
							DAG[j].value = "-" + A;
							break;
						}
						else if (find(DAG[j].labelList.begin(), DAG[j].labelList.end(), A) != DAG[j].labelList.end()) { // A在附加标记符中删除
							DAG[j].labelList.erase(find(DAG[j].labelList.begin(), DAG[j].labelList.end(), A));
							break;
						}
					}
					DAG[n].labelList.push_back(A);
					state = DAG_STATE_END;
					break;
				}
				default: {
					break;
				}
			}
		}
	}
	return DAG;
}

void assignByMap(map<string, string>& tm, string& s) {
	if (tm.find(s) != tm.end()) {
		s = tm[s];
	}
}

void Optimizer::optimize() {
	vector<Quaternion> optimizedCode;
	// 遍历所有block
	for (unsigned int blkno = 0; blkno < block.size(); blkno++) {
		vector<DAGItem> DAG = generateDAG(blkno);
		this->DAGs.push_back(DAG);
		BlockItem newBlock;
		newBlock.begin = optimizedCode.size();
		BlockItem oldBlock = block[blkno];
		vector<string> waitVar = oldBlock.waitVar;
		waitVar.push_back("$gp");
		waitVar.push_back("$sp");
		waitVar.push_back("$fp");
		waitVar.push_back("$v0");
		waitVar.push_back("$t0");
		waitVar.push_back("$t1");
		waitVar.push_back("$t2");
		waitVar.push_back("$t3");
		waitVar.push_back("$t4");
		waitVar.push_back("$t5");
		waitVar.push_back("$t6");
		waitVar.push_back("$t7");
		waitVar.push_back("[$sp]");
		// 遍历DAG节点，更新waitVar
		for (unsigned int i = 0; i < DAG.size(); i++) {
			if (DAG[i].isLeft) {
				if (DAG[i].quaternion.arg1 != "" && find(waitVar.begin(), waitVar.end(), DAG[i].quaternion.arg1) == waitVar.end()) {
					waitVar.push_back(DAG[i].quaternion.arg1);
				}
				if (DAG[i].quaternion.arg2 != "" && find(waitVar.begin(), waitVar.end(), DAG[i].quaternion.arg2) == waitVar.end()) {
					waitVar.push_back(DAG[i].quaternion.arg2);
				}
			}
		}
		// 遍历DAG节点
		for (unsigned int i = 0; i < DAG.size(); i++) {
			if (!DAG[i].isLeft) {
				if (DAG[i].children[2] == DAGItem::TopoStructInit) { // 非3型式
					vector<string> newLabelList;
					for (unsigned int j = 0; j < DAG[i].labelList.size(); j++) {
						if (DAG[i].labelList[j][0] == 'G' || find(waitVar.begin(), waitVar.end(), DAG[i].labelList[j]) != waitVar.end()) {
							newLabelList.push_back(DAG[i].labelList[j]);
							DAG[i].flag = true;
						}
					}
					DAG[i].labelList = newLabelList;
					if (DAG[i].flag) {
						this->utilizeChildren(DAG, i);
					}
					if (!DAG[i].isLeaf && DAG[i].labelList.size() == 0) {
						DAG[i].labelList.push_back(this->newtemp());
					}
				}
				else { // 3型式
					DAG[i].flag = true;
					this->utilizeChildren(DAG, i);
				}
			}
		}
		// 遍历DAG节点
		for (unsigned int i = 0; i < DAG.size(); i++) {
			if (DAG[i].isLeft) {
				optimizedCode.push_back(DAG[i].quaternion);
			}
			else {
				if (DAG[i].isLeaf) {
					for (unsigned int j = 0; j < DAG[i].labelList.size(); j++) {
						string v = DAG[i].value[0] == '-' ? DAG[i].value.substr(1) : DAG[i].value;
						Quaternion q(":=", v, "", DAG[i].labelList[j]);
						optimizedCode.push_back(q);
					}
				}
				else {
					string lv = this->getChildValue(i, 0, DAG);
					string rv = this->getChildValue(i, 1, DAG);
					if (DAG[i].children[2] != DAGItem::TopoStructInit) {
						string triv = this->getChildValue(i, 2, DAG);
						Quaternion q(DAG[i].op, lv, rv, triv);
						optimizedCode.push_back(q);
					}
					else {
						Quaternion q(DAG[i].op, lv, rv, DAG[i].labelList[0]);
						optimizedCode.push_back(q);
						for (unsigned int j = 1; j < DAG[i].labelList.size(); j++) {
							Quaternion q(":=", DAG[i].labelList[0], "", DAG[i].labelList[j]);
							optimizedCode.push_back(q);
						}
					}
				}
			}
		}
		// 遍历新产生的中间代码
		for (unsigned int i = newBlock.begin; i < optimizedCode.size(); i++) {
			Quaternion q = optimizedCode[i];
			if (q.op == "+" && q.arg1 == "$sp" && is_num(q.arg2) && q.result == "$sp") {
				int sum = atoi(q.arg2.c_str());
				while (i + 1 < (int)optimizedCode.size() && optimizedCode[i + 1].op == "+" && optimizedCode[i + 1].arg1 == "$sp" && is_num(optimizedCode[i + 1].arg2) && optimizedCode[i + 1].result == "$sp") {
					sum += atoi(optimizedCode[i + 1].arg2.c_str());
					optimizedCode.erase(optimizedCode.begin() + i + 1);
				}
				optimizedCode[i].arg2 = to_string(sum);
			}
		}
		newBlock.end = optimizedCode.size() - 1;
	}

	map<string, string> tmpVMap;
	int newTmpCnt = 0;
	for (unsigned int i = 0; i < optimizedCode.size(); i++) {
		Quaternion q = optimizedCode[i];
		if ((q.arg1[0] == 'T' || q.arg1[0] == 'S') && tmpVMap.find(q.arg1) == tmpVMap.end()) {
			tmpVMap[q.arg1] = "T" + to_string(newTmpCnt++);
		}
		if ((q.arg2[0] == 'T' || q.arg2[0] == 'S') && tmpVMap.find(q.arg2) == tmpVMap.end()) {
			tmpVMap[q.arg2] = "T" + to_string(newTmpCnt++);
		}
		if ((q.result[0] == 'T' || q.result[0] == 'S') && tmpVMap.find(q.result) == tmpVMap.end()) {
			tmpVMap[q.result] = "T" + to_string(newTmpCnt++);
		}
	}
	for (unsigned int i = 0; i < optimizedCode.size(); i++) {
		assignByMap(tmpVMap, optimizedCode[i].arg1);
		assignByMap(tmpVMap, optimizedCode[i].arg2);
		assignByMap(tmpVMap, optimizedCode[i].result);
	}

	//
	initBlock = block;
	initCode = intermediateCode;
	intermediateCode = optimizedCode;
	block.clear();
	generateBlock();
}

void Optimizer::analyse() {
	this->preGenerateBlock();
	this->generateBlock();

	int originalSize = intermediateCode.size();

	while (initCode.size() != intermediateCode.size()) {
		this->optimize();
		this->optimizeCnt++;
	}

	int optimizedSize = intermediateCode.size();
	this->optimizeRate = double(optimizedSize) / originalSize;
}

/*============================== Optimizer ==============================*/