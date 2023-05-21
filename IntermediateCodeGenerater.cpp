#include "IntermediateCodeGenerater.h"

/*============================== �м���������� ==============================*/

IntermediateCodeGenerater::IntermediateCodeGenerater(const string& s) {
	lineCnt = 1;

	this->syntaxAnalyser.analyse();
	this->lexcalAnalyser.setSourceCode(s);
}

IntermediateCodeGenerater::~IntermediateCodeGenerater() {

}

string IntermediateCodeGenerater::generateProductionStr(const Production& p) {
	string ret = "<" + p.first + ">::=";
	for (int i = 0; i < p.second.size(); i++) {
		if (find(this->syntaxAnalyser.VN.begin(), this->syntaxAnalyser.VN.end(), p.second[i]) != this->syntaxAnalyser.VN.end()) {
			ret += "<" + p.second[i] + ">"; // ���ַ��ڷ��ս������
		}
		else {
			ret += "'" + p.second[i] + "'"; // ���ַ����ս������
		}
	}
	return ret;
}

void IntermediateCodeGenerater::analyse(const string& filename) {

	// ���ļ�
	ofstream outfile(filename, ios::out | ios::binary);
	if (!outfile.is_open()) {
		cerr << "ERROR: �м�����������޷�������ļ�!" << endl;
		exit(ERROR_OPEN_FILE);
	}
	outfile << "digraph mygraph {\n";
	int cnt = 0;
	vector<int> cntStack;

	// ��ʼ��ջ
	stack<int> stateStack; // ״̬ջ
	stack<Token> symbolStack; // ����ջ
	stateStack.push(0);
	symbolStack.push(Token("#", -1));
	
	Token epsilonToken = Token("epsilon", -1);
	Token nextEpsilonToken;
	Token token = this->lexcalAnalyser.getNextToken();

	SyntaxTreeNode* stnp;
	stack<SyntaxTreeNode*> syntaxTreeNodeStack;

	int pos;

	while (token.str != "ERROR") {

		// tokenΪ'\n'
		if (token.str == "NL") {
			this->lineCnt++;
			token = this->lexcalAnalyser.getNextToken();
			continue;
		}

		// �ж�token�Ƿ������ս�� 
		pos = this->syntaxAnalyser.getVTPos(token.str);
		if (pos == NOT_FOUND) {
			cerr << "ERROR: �м�������������ַ��ս��!" << endl;
			exit(ERROR_INTERMEDIATE_CODE_GENERATER);
		}

		// ȡaction����
		ActionItem actionItem = this->syntaxAnalyser.ACTION[stateStack.top()][pos];

		switch (actionItem.status) {
			case ActionItem::A_ACC: {
				// �ļ����
				outfile << "}";
				outfile.close();

				return;
				break;
			case ActionItem::A_ERROR:
				if (token != epsilonToken) {
					nextEpsilonToken = token;
					token = epsilonToken;
				}
				else {
					cerr << "ERROR: �м����������ɨ�赽��" << to_string(lineCnt) << "�еķ���" << nextEpsilonToken.str << "ʱ��������" << endl;
					exit(ERROR_INTERMEDIATE_CODE_GENERATER);
				}
				break;
			}
			case ActionItem::A_SHIFT: { // �ƽ�
				stnp = new(nothrow) SyntaxTreeNode;
				if (!stnp) {
					cerr << "ERROR: �м���������������﷨���ڵ�ʱ�ռ䲻��" << endl;
					exit(ERROR_NEW);
				}
				stnp->token = token;
				syntaxTreeNodeStack.push(stnp);

				// ��ջ
				stateStack.push(actionItem.nextState);
				symbolStack.push(token);

				// �ļ����
				cntStack.push_back(cnt);
				outfile << "n" << cnt++ << "[label=\"" << token.str << "\",color=red];" << endl;

				// ȡ��һ��token
				if (token == epsilonToken) {
					token = nextEpsilonToken;
				}
				else {
					token = this->lexcalAnalyser.getNextToken();
				}
				break;
			}
			case ActionItem::A_REDUCTION: { // ��Լ

				/* ���·���ջ��״̬ջ */
				// �������ACTION����Ĺ�Լʽ�Ҳ�
				vector<int> cntLeft;
				for (int i = actionItem.production.second.size() - 1; i >= 0; i--) {
					if (symbolStack.top().str == actionItem.production.second[i]) { // ��Լ��ջ
						symbolStack.pop();
						stateStack.pop();

						// 
						cntLeft.push_back(cntStack.back());
						cntStack.pop_back();
					}
					else {
						cerr << "ERROR: �м����������ɨ�赽��" << to_string(lineCnt) << "�еķ���" << token.str << "ʱ��������" << endl;
						exit(ERROR_INTERMEDIATE_CODE_GENERATER);
					}
				}
				symbolStack.push(Token(actionItem.production.first, -1));
				pos = this->syntaxAnalyser.getVNPos(symbolStack.top().str);
				stateStack.push(this->syntaxAnalyser.GOTO[stateStack.top()][pos]);

				// �ļ����
				cntStack.push_back(cnt);
				outfile << "n" << cnt++ << "[label=\"" << actionItem.production.first << "\"];\n";

				if (cntLeft.size() != 0) {
					for (auto t = cntLeft.begin(); t != cntLeft.end(); t++) {
						outfile << "n" << cnt - 1 << " -> " << "n" << *t << ";\n";
					}
				}
				else { //�մ�
					outfile << "e" << cnt << "[label=\"@\"];\n";
					outfile << "n" << cnt - 1 << " -> " << "e" << cnt << ";\n";
				}

				// �����﷨��ջ
				stnp = new(nothrow) SyntaxTreeNode;
				if (!stnp) {
					cerr << "ERROR: �м���������������﷨���ڵ�ʱ�ռ䲻��" << endl;
					exit(ERROR_NEW);
				}
				stnp->token = Token(actionItem.production.first, -1);
				for (int i = 0; i < actionItem.production.second.size(); i++) {
					syntaxTreeNodeStack.top()->parent = stnp;
					stnp->children.push_back(syntaxTreeNodeStack.top());
					syntaxTreeNodeStack.pop();
				}
				reverse(stnp->children.begin(), stnp->children.end());
				syntaxTreeNodeStack.push(stnp);

				// �������������
				this->semanticAnalyser.analyse(this->generateProductionStr(actionItem.production), stnp, this->lexcalAnalyser.varTable);

				break;
			}
			default: {
				break;
			}
		}
	}
}

void IntermediateCodeGenerater::drawSyntaxTree(const string& s, const string& d) {
	string cmd = "dot -Tpng " + s + " -o " + d;
	system(cmd.c_str());
}

void IntermediateCodeGenerater::showIntermediateCode(const string& filename, const bool& isOut) {
	ofstream outfile(filename, ios::out | ios::binary);
	if (!outfile.is_open()) {
		cerr << "ERROR: �м�����ļ���ʧ��!" << endl;
		exit(ERROR_OPEN_FILE);
	}

	for (int i = 0; i < this->semanticAnalyser.intermediateCode.size(); i++) {
		if (isOut) {
			cout << this->semanticAnalyser.intermediateCode[i] << endl;
		}
		outfile << this->semanticAnalyser.intermediateCode[i] << endl;
	}

	outfile.close();
}

/*============================== �м���������� ==============================*/