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
	// ���ļ�
	ifstream inFile(G_FileName, ios::in | ios::binary);
	if (!inFile.is_open()) {
		cerr << "ERROR: �޷���grammar�ļ�" << endl;
		exit(ERROR_OPEN_FILE);
	}

	// ����ֵ
	S = G_SINIT;
	VT.insert("#");

	// �����ļ�
	while (!inFile.eof()) {
		bool flag = false; // ����ʽ�����ұ�ʶ��
		Productions productions; // �в���ʽ
		ProductionRight productionRight; // ����ʽ�Ҳ�

		// ��ȡһ��
		string lineBuffer;
		getline(inFile, lineBuffer);

		// ����һ��
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

	// �ر��ļ�
	inFile.close();

	// ��֤
	if (VN.size() != vnProductionRightsMap.size()) {
		cerr << "ERROR: �ķ��Ĳ���ʽ����ս����Ŀ����Ӧ" << endl;
		exit(ERROR_SYNTAX_ANALYSE_GRAMMAR_PRODUCTION_VN_MATCH);
	}
}

void SyntaxAnalyser::generateFIRST() {
	// ��ʼ��VT�ս����FIRST
	for (auto i = VT.begin(); i != VT.end(); i++) {
		FIRST[(*i)] = { (*i) };
	}

	// ��������VN���ս����FIRST
	bool isChange = true;
	while (isChange) {
		isChange = false;
		for (auto i = vnProductionRightsMap.begin(); i != vnProductionRightsMap.end(); i++) {
			vn LVn = i->first; // ȡ����ʽ��ߵķ��ս��
			if (FIRST.find(LVn) == FIRST.end()) {
				FIRST[LVn] = {}; // FIRST�в�����VN���ս������Ҫ��ʼ��
			}
			// ������VN���ս����Ӧ��ÿһ������ʽ�Ҳ�
			for (unsigned int j = 0; j < i->second.size(); j++) {
				if (i->second[j].size() > 0) {
					string firstRV = i->second[j][0]; // ȡ����ʽ�Ҳ����׸��ַ�
					// �������ַ���Ӧ��FIRST����
					for (unsigned int k = 0; k < FIRST[firstRV].size(); k++) {
						if (count(FIRST[LVn].begin(), FIRST[LVn].end(), FIRST[firstRV][k]) == 0) {
							FIRST[LVn].push_back(FIRST[firstRV][k]);
							isChange = true;
						}
					}
				}
				else {
					cerr << "ERROR: �ķ�����ĳ������ʽ" << LVn << "�Ҳ�Ϊ��!" << endl;
					exit(ERROR_SYNTAX_ANALYSE_GRAMMAR_LACK_PRODUCTIONRIGHT);
				}
			}
		}
	}
}

/*
* Function Name:        CLOSURE
* Function Description: ���ݵ�ǰ����Ŀ����I���ɸ���Ŀ����ıհ�
*/
void SyntaxAnalyser::CLOSURE(LR1ItemSet& I) {
	// ������Ŀ��I
	// ע�⣺�Ƚ�setת��Ϊvector�������޷�������������Ԫ��
	vector<LR1Item> tmp_vec(I.ItemSet.begin(), I.ItemSet.end());
	for (unsigned int i = 0; i < tmp_vec.size(); i++) {
		if (tmp_vec[i].dotPos >= (int)tmp_vec[i].production.second.size()) {
			continue;
		}

		string v = tmp_vec[i].production.second[tmp_vec[i].dotPos];
		// ��.�����Ӧ���Ƿ��ս�������÷��ս�������в���ʽ����I
		if (VN.count(v)) {
			// ����v��ÿһ������ʽ
			for (unsigned int j = 0; j < vnProductionRightsMap[v].size(); j++) {
				LR1Item item(pair<vn, ProductionRight>(v, vnProductionRightsMap[v][j])); // ���ݸò���ʽ����һ��LR1��Ŀ
				// ������ǰ������
				if (tmp_vec[i].dotPos + 1 == tmp_vec[i].production.second.size()) {
					// ��*i��forwardSearchStrSet���뵽item����ǰ��������
					for (auto k = tmp_vec[i].forwardSearchStrSet.begin(); k != tmp_vec[i].forwardSearchStrSet.end(); k++) {
						item.forwardSearchStrSet.insert(*k);
					}
				}
				else {
					// ��FIRST[��һ���ַ�]���뵽item����ǰ��������
					string nextV = tmp_vec[i].production.second[tmp_vec[i].dotPos + 1];
					for (unsigned int k = 0; k < FIRST[nextV].size(); k++) {
						item.forwardSearchStrSet.insert(FIRST[nextV][k]);
					}
				}
				//ȥ��
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
* Function Description: ��I��Ŀ������.������ַ�ǡ��Ϊv����.����ƶ�һλ
*/
LR1ItemSet SyntaxAnalyser::GO(const LR1ItemSet& I, const string& v) {
	LR1ItemSet ret;
	// ����I�����е�Ԫ��
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
	// ȥ��
	vector<string>::iterator pos = unique(ret.begin(), ret.end());
	ret.erase(pos, ret.end());
	return ret;
}

void SyntaxAnalyser::show() {
	if (ACTION.size() != GOTO.size())
	{
		//cerr << "ERROR: the ACTION table can not match GOTO table\n";
		//cerr << "ERROR: ACTION���GOTO����Ӧ\n";
		//exit(-1);
		throw string("ERROR: �ڲ�����:ACTION���GOTO����Ӧ\n");
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
		// ȡstk��ǰ��ջ��
		int now = stk.top();
		stk.pop();

		// ȡջ����Ŀ����.������ַ�
		vector<string> nextV = generateNextV(itemSetRec[now]);

		// ���������ַ�
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
			// ����VN��
			int pos = 0;
			for (auto k = VN.begin(); k != VN.end(); k++, pos++) {
				if ((*k) == j->first) {
					Goto[pos] = j->second;
					flag = true;
					break;
				}
			}
			// ����VT��
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
			// ������
			if (!flag) {
				cerr << "ERROR: ����ACTION_GOTO�����!" << endl;
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

	cout << endl << "�﷨����:���ڶ�ȡ�﷨..." << endl;
	begin = clock();
	this->readGrammar();
	end = clock();
	cout << "��ȡ�﷨�ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

	cout << endl << "�﷨����:��������FIRST����..." << endl;
	begin = clock();
	this->generateFIRST();
	end = clock();
	cout << "����FIRST���ϳɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

	cout << endl << "�﷨����:��������DFA..." << endl;
	begin = clock();
	this->generateDFA();
	end = clock();
	cout << "����DFA�ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

	cout << endl << "�﷨����:��������LR1������..." << endl;
	begin = clock();
	this->generateLR1Table(); 
	end = clock();
	cout << "����LR1������ɹ�����ʱ" << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;
}

int SyntaxAnalyser::getVTPos(const string& s) {

	
	// �ж�s�Ƿ���VT������
	if (find(VT.begin(), VT.end(), s) == VT.end()) {
		return NOT_FOUND;
	}
	

	// ����VT���ϣ���Ϊ������ȽϺ���������VT�������set
	int ret = 0;
	for (auto i = VT.begin(); i != VT.end(); i++, ret++) {
		if ((*i) == s) {
			return ret;
		}
	}

	return NOT_FOUND;
}

int SyntaxAnalyser::getVNPos(const string& s) {
	// ����VN���ϣ���Ϊ������ȽϺ���������VT�������set
	int ret = 0;
	for (auto i = VN.begin(); i != VN.end(); i++, ret++) {
		if ((*i) == s) {
			return ret;
		}
	}

	return NOT_FOUND;
}

/*============================== SyntaxAnalyser ==============================*/