#include "SyntaxAnalyser.h"

/*============================== LR1Item ==============================*/

LR1Item::LR1Item() {

}

LR1Item::LR1Item(const Production& p, const int& d) {
	this->production = p;
	this->dotPos = d;
}

LR1Item::LR1Item(const Production& p, const int& d, const set<ForwardSearchStr>& f) {
	this->production = p;
	this->dotPos = d;
	this->forwardSearchStrSet = f;
}

LR1Item::~LR1Item() {

}

bool LR1Item::operator== (const LR1Item& item) {
	if (forwardSearchStrSet.size() != item.forwardSearchStrSet.size()) {
		return false;
	}
	else {
		for (auto i = forwardSearchStrSet.begin(); i != forwardSearchStrSet.end(); i++) {
			if (item.forwardSearchStrSet.count(*i) == 0) {
				return false;
			}
		}
	}
	if (production.second.size() != item.production.second.size()) {
		return false;
	}
	else {
		for (auto i : production.second) {
			if (find(item.production.second.begin(), item.production.second.end(), i) == item.production.second.end()) {
				return false;
			}
		}
	}
	return production.first == item.production.first && dotPos == item.dotPos;
}

bool LR1Item::operator< (const LR1Item& item) const {
	if (production.first != item.production.first) {
		return production.first < item.production.first;
	}
	if (dotPos != item.dotPos) {
		return dotPos < item.dotPos;
	}

	if (forwardSearchStrSet.size() != item.forwardSearchStrSet.size()) {
		return forwardSearchStrSet.size() < item.forwardSearchStrSet.size();
	}
	if (production.second.size() != item.production.second.size()) {
		return production.second.size() < item.production.second.size();
	}

	for (unsigned int i = 0; i < production.second.size(); i++) {
		if (production.second[i] != item.production.second[i]) {
			return production.second[i] < item.production.second[i];
		}
	}

	vector<string> a(forwardSearchStrSet.begin(), forwardSearchStrSet.end());
	vector<string> b(item.forwardSearchStrSet.begin(), item.forwardSearchStrSet.end());
	for (unsigned int i = 0; i < a.size(); i++) {
		if (a[i] != b[i]) {
			return a[i] < b[i];
		}
	}

	return false;
}

/*============================== LR1Item ==============================*/



/*============================== LR1ItemSet ==============================*/

bool LR1ItemSet::operator== (const LR1ItemSet& I) {
	if (this->ItemSet.size() != I.ItemSet.size()) {
		return false;
	}
	for (auto i = this->ItemSet.begin(); i != this->ItemSet.end(); i++) {
		if (!I.ItemSet.count(*i)) {
			return false;
		}
	}
	return true;
}


/*============================== LR1ItemSet ==============================*/



/*============================== LR1Item ==============================*/

ActionItem::ActionItem() {
	nextState = -1;
	status = A_ERROR;
}

ActionItem::~ActionItem() {

}

/*============================== LR1Item ==============================*/



/*============================== SyntaxAnalyser ==============================*/

void clearTmp(Productions& productions, ProductionRight& productionRight) {
	productionRight.clear();
	productions.first = "";
	productions.second.clear();
}

SyntaxAnalyser::SyntaxAnalyser() {
	itemSetRec.resize(G_MAXITEMSETSIZE);
	DFA.resize(G_MAXDFASIZE);
}

SyntaxAnalyser::~SyntaxAnalyser() {

}

void SyntaxAnalyser::readGrammar() {
	// 打开文件
	ifstream inFile(G_FileName, ios::in | ios::binary);
	if (!inFile.is_open()) {
		cerr << "ERROR: 无法打开grammar文件" << endl;
		exit(ERROR_OPEN_FILE);
	}

	// 赋初值
	S = G_SINIT;
	VT.insert("#");

	// 遍历文件
	while (!inFile.eof()) {
		bool flag = false; // 产生式的左右标识符
		Productions productions; // 行产生式
		ProductionRight productionRight; // 产生式右部

		// 读取一行
		string lineBuffer;
		getline(inFile, lineBuffer);

		// 遍历一行
		for (unsigned int i = 0; i < lineBuffer.size(); i++) {
			string v;
			switch (lineBuffer[i]) {
				case G_VNSTART:
					v.clear();
					i++;
					while (lineBuffer[i] != G_VNEND) {
						v += lineBuffer[i++];
					}
					VN.insert(v);
					if (S == G_SINIT) {
						S = v;
					}
					if (!flag) {
						productions.first = v;
					}
					else {
						productionRight.push_back(v);
					}
					break;
				case G_VTSTART:
					v.clear();
					i++;
					while (lineBuffer[i] != G_VTEND) {
						v += lineBuffer[i++];
					}
					VT.insert(v);
					productionRight.push_back(v);
					break;
				case G_ASSIGNSTART:
					while (lineBuffer[i] != G_ASSIGNEND) {
						i++;
					}
					flag = true;
					break;
				case G_MORE:
					productions.second.push_back(productionRight);
					productionRight.clear();
					break;
				default:
					break;
			}
		}
		productions.second.push_back(productionRight);
		vnProductionRightsMap[productions.first] = productions.second;
		clearTmp(productions, productionRight);
	}

	// 关闭文件
	inFile.close();

	// 验证
	if (VN.size() != vnProductionRightsMap.size()) {
		cerr << "ERROR: 文法的产生式与非终结符数目不对应" << endl;
		exit(ERROR_SYNTAX_ANALYSE_GRAMMAR_PRODUCTION_VN_MATCH);
	}
}

void SyntaxAnalyser::generateFIRST() {
	// 初始化VT终结符的FIRST
	for (auto i = VT.begin(); i != VT.end(); i++) {
		FIRST[(*i)] = { (*i) };
	}

	// 计算所有VN非终结符的FIRST
	bool isChange = true;
	while (isChange) {
		isChange = false;
		for (auto i = vnProductionRightsMap.begin(); i != vnProductionRightsMap.end(); i++) {
			vn LVn = i->first; // 取产生式左边的非终结符
			if (FIRST.find(LVn) == FIRST.end()) {
				FIRST[LVn] = {}; // FIRST中不含该VN非终结符，需要初始化
			}
			// 遍历该VN非终结符对应的每一个产生式右部
			for (unsigned int j = 0; j < i->second.size(); j++) {
				if (i->second[j].size() > 0) {
					string firstRV = i->second[j][0]; // 取产生式右部的首个字符
					// 遍历该字符对应的FIRST集合
					for (unsigned int k = 0; k < FIRST[firstRV].size(); k++) {
						if (count(FIRST[LVn].begin(), FIRST[LVn].end(), FIRST[firstRV][k]) == 0) {
							FIRST[LVn].push_back(FIRST[firstRV][k]);
							isChange = true;
						}
					}
				}
				else {
					cerr << "ERROR: 文法存在某条产生式" << LVn << "右部为空!" << endl;
					exit(ERROR_SYNTAX_ANALYSE_GRAMMAR_LACK_PRODUCTIONRIGHT);
				}
			}
		}
	}
}

/*
* Function Name:        CLOSURE
* Function Description: 根据当前的项目集族I生成该项目集族的闭包
*/
void SyntaxAnalyser::CLOSURE(LR1ItemSet& I) {
	// 遍历项目集I
	// 注意：先将set转化为vector，否则无法遍历到新增的元素
	vector<LR1Item> tmp_vec(I.ItemSet.begin(), I.ItemSet.end());
	for (unsigned int i = 0; i < tmp_vec.size(); i++) {
		if (tmp_vec[i].dotPos >= (int)tmp_vec[i].production.second.size()) {
			continue;
		}

		string v = tmp_vec[i].production.second[tmp_vec[i].dotPos];
		// 若.后面对应的是非终结符，将该非终结符的所有产生式加入I
		if (VN.count(v)) {
			// 遍历v的每一条产生式
			for (unsigned int j = 0; j < vnProductionRightsMap[v].size(); j++) {
				LR1Item item(pair<vn, ProductionRight>(v, vnProductionRightsMap[v][j])); // 根据该产生式生成一个LR1项目
				// 产生向前搜索串
				if (tmp_vec[i].dotPos + 1 == tmp_vec[i].production.second.size()) {
					// 将*i的forwardSearchStrSet插入到item的向前搜索串中
					for (auto k = tmp_vec[i].forwardSearchStrSet.begin(); k != tmp_vec[i].forwardSearchStrSet.end(); k++) {
						item.forwardSearchStrSet.insert(*k);
					}
				}
				else {
					// 将FIRST[下一个字符]插入到item的向前搜索串中
					string nextV = tmp_vec[i].production.second[tmp_vec[i].dotPos + 1];
					for (unsigned int k = 0; k < FIRST[nextV].size(); k++) {
						item.forwardSearchStrSet.insert(FIRST[nextV][k]);
					}
				}
				//去重
				bool flag = false;
				for (unsigned int k = 0; k < tmp_vec.size(); k++) {
					if (tmp_vec[k].production == item.production && tmp_vec[k].dotPos == item.dotPos) {
						flag = true;
						set<ForwardSearchStr> t;
						set_union(tmp_vec[k].forwardSearchStrSet.begin(), tmp_vec[k].forwardSearchStrSet.end(), item.forwardSearchStrSet.begin(), item.forwardSearchStrSet.end(), inserter(t, t.begin()));
						tmp_vec[k].forwardSearchStrSet = t;
						break;
					}
				}
				if (!flag) {
					tmp_vec.push_back(item);
				}
			}
		}
	}
	I.ItemSet.clear();
	for (unsigned int i = 0; i < tmp_vec.size(); i++) {
		I.ItemSet.insert(tmp_vec[i]);
	}
}

/*
* Function Name:        GO
* Function Description: 若I项目集族中.后面的字符恰好为v，则将.向后移动一位
*/
LR1ItemSet SyntaxAnalyser::GO(const LR1ItemSet& I, const string& v) {
	LR1ItemSet ret;
	// 遍历I中所有的元素
	for (auto i = I.ItemSet.begin(); i != I.ItemSet.end(); i++) {
		if ((*i).dotPos < (int)(*i).production.second.size() && (*i).production.second[(*i).dotPos] == v) {
			LR1Item item = (*i);
			item.dotPos++;
			ret.ItemSet.insert(item);
		}
	}
	CLOSURE(ret);
	return ret;
}

vector<string> SyntaxAnalyser::generateNextV(const LR1ItemSet& I) {
	vector<string> ret;
	for (auto i = I.ItemSet.begin(); i != I.ItemSet.end(); i++) {
		if ((*i).dotPos < (int)(*i).production.second.size()) {
			ret.push_back((*i).production.second[(*i).dotPos]);
		}
	}
	// 去重
	vector<string>::iterator pos = unique(ret.begin(), ret.end());
	ret.erase(pos, ret.end());
	return ret;
}

void SyntaxAnalyser::show() {
	if (ACTION.size() != GOTO.size())
	{
		//cerr << "ERROR: the ACTION table can not match GOTO table\n";
		//cerr << "ERROR: ACTION表和GOTO表不对应\n";
		//exit(-1);
		throw string("ERROR: 内部错误:ACTION表和GOTO表不对应\n");
	}
	map<int, vector<ActionItem>>::iterator p;
	for (p = ACTION.begin(); p != ACTION.end(); p++)
	{
		cout << p->first << '\t';
		for (unsigned int i = 0; i < p->second.size(); i++)
		{
			switch (p->second[i].status)
			{
				case ActionItem::A_ACC:cout << "ACC\t"; break;
				case ActionItem::A_SHIFT:cout << "s" << p->second[i].nextState << "\t"; break;
				case ActionItem::A_REDUCTION:
					cout << "r:" << p->second[i].production.first << "->";
					for (unsigned int j = 0; j < p->second[i].production.second.size(); j++)
					{
						cout << p->second[i].production.second[j] << ' ';
					}
					break;
				case ActionItem::A_ERROR:cout << "\t"; break;
				default:break;
			}
			cout << "\t";
		}
		cout << '\t';
		for (unsigned int i = 0; i < GOTO[p->first].size(); i++)
			cout << GOTO[p->first][i] << ' ';
		cout << endl;
	}
}


void SyntaxAnalyser::generateDFA() {
	stack<int> stk;

	int cnt = 0;
	LR1ItemSet I;
	I.ItemSet.insert(LR1Item(pair<vn, ProductionRight>("S'", { S }), 0, { "#" }));
	CLOSURE(I);
	itemSetRec[cnt] = I;

	stk.push(cnt++);

	while (!stk.empty()) {
		// 取stk当前的栈顶
		int now = stk.top();
		stk.pop();

		// 取栈顶项目集的.后面的字符
		vector<string> nextV = generateNextV(itemSetRec[now]);

		// 遍历所有字符
		for (unsigned int i = 0; i < nextV.size(); i++) {
			LR1ItemSet item = GO(itemSetRec[now], nextV[i]);
			int index = find(itemSetRec.begin(), itemSetRec.end(), item) - itemSetRec.begin();
			if (index == itemSetRec.size()) {
				DFA[now].go[nextV[i]] = cnt;
				itemSetRec[cnt] = item;
				stk.push(cnt++);
			}
			else {
				DFA[now].go[nextV[i]] = index;
			}
		}
	}
}

void SyntaxAnalyser::generateLR1Table() {
	for (unsigned int i = 0; i < itemSetRec.size(); i++) {
		vector<ActionItem> Action(VT.size());
		vector<int> Goto(VN.size(), -1);
		set<LR1Item> I = itemSetRec[i].ItemSet;
		map<string, int> next = DFA[i].go;
		for (auto j = next.begin(); j != next.end(); j++) {
			bool flag = false;
			// 遍历VN集
			int pos = 0;
			for (auto k = VN.begin(); k != VN.end(); k++, pos++) {
				if ((*k) == j->first) {
					Goto[pos] = j->second;
					flag = true;
					break;
				}
			}
			// 遍历VT集
			if (!flag) {
				pos = 0;
				for (auto k = VT.begin(); k != VT.end(); k++, pos++) {
					if ((*k) == j->first) {
						Action[pos].status = ActionItem::A_SHIFT;
						Action[pos].nextState = j->second;
						flag = true;
						break;
					}
				}
			}
			// 不存在
			if (!flag) {
				cerr << "ERROR: 生成ACTION_GOTO表错误!" << endl;
				exit(ERROR_SYNTAX_ANALYSE);
			}
		}
		for (auto j = I.begin(); j != I.end(); j++) {
			if ((*j).dotPos == (*j).production.second.size()) {
				if ((*j).production.first == "S'") {
					(Action.end() - 1)->status = ActionItem::A_ACC;
				}
				else {
					LR1Item item = (*j);
					for (auto k = item.forwardSearchStrSet.begin(); k != item.forwardSearchStrSet.end(); k++) {
						int pos = 0;
						for (auto p = VT.begin(); p != VT.end(); p++, pos++) {
							if ((*p) == (*k)) {
								Action[pos].status = ActionItem::A_REDUCTION;
								Action[pos].production = item.production;
								break;
							}
						}
					}
				}
			}
		}
		ACTION[i] = Action;
		GOTO[i] = Goto;
	}
}

void SyntaxAnalyser::analyse() {
	time_t begin, end;

	cout << endl << "语法分析:正在读取语法..." << endl;
	begin = clock();
	this->readGrammar();
	end = clock();
	cout << "读取语法成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

	cout << endl << "语法分析:正在生成FIRST集合..." << endl;
	begin = clock();
	this->generateFIRST();
	end = clock();
	cout << "生成FIRST集合成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

	cout << endl << "语法分析:正在生成DFA..." << endl;
	begin = clock();
	this->generateDFA();
	end = clock();
	cout << "生成DFA成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

	cout << endl << "语法分析:正在生成LR1分析表..." << endl;
	begin = clock();
	this->generateLR1Table(); 
	end = clock();
	cout << "生成LR1分析表成功，用时" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;
}

int SyntaxAnalyser::getVTPos(const string& s) {

	
	// 判断s是否在VT集合中
	if (find(VT.begin(), VT.end(), s) == VT.end()) {
		return NOT_FOUND;
	}
	

	// 遍历VT集合，因为定义过比较函数，所以VT是有序的set
	int ret = 0;
	for (auto i = VT.begin(); i != VT.end(); i++, ret++) {
		if ((*i) == s) {
			return ret;
		}
	}

	return NOT_FOUND;
}

int SyntaxAnalyser::getVNPos(const string& s) {
	// 遍历VN集合，因为定义过比较函数，所以VT是有序的set
	int ret = 0;
	for (auto i = VN.begin(); i != VN.end(); i++, ret++) {
		if ((*i) == s) {
			return ret;
		}
	}

	return NOT_FOUND;
}

/*============================== SyntaxAnalyser ==============================*/