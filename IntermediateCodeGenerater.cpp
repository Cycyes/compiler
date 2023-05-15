#include "IntermediateCodeGenerater.h"

/*============================== �﷨����Ϣ ==============================*/

SyntaxTreeInfo::SyntaxTreeInfo() {
	maxTreeLevel = 0;
	leafNum = 0;

	root = NULL;
}

SyntaxTreeInfo::~SyntaxTreeInfo() {

}

void SyntaxTreeInfo::updateInfo(SyntaxTreeNode* n, const int& l) {
	// ����maxTreeLevel
	if (l > maxTreeLevel) {
		maxTreeLevel = l;
	}

	// ����Ҷ�ӽڵ�����
	if (n->children.size() == 0) {
		leafNum++;
	}
}

/*============================== �﷨����Ϣ ==============================*/



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

void IntermediateCodeGenerater::generateTreeLevel(SyntaxTreeNode* n, const int& l) {
	
	// ����������Ϣ
	this->syntaxTreeInfo.updateInfo(n, l);

	n->level = l;

	// ��������
	for (int i = 0; i < n->children.size(); i++) {
		generateTreeLevel(n->children[i], l + 1);
	}
}

void IntermediateCodeGenerater::analyse() {
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
			case ActionItem::A_ACC:
				// ��ɷ����������﷨��
				this->generateTreeLevel(this->syntaxTreeInfo.root, 0);
				return;
				break;
			case ActionItem::A_ERROR:
				if (token != epsilonToken) {
					nextEpsilonToken = token;
					token = epsilonToken;
				}
				else {
					cerr << "ERROR: �м����������ɨ�赽��" << to_string(lineCnt) << "�еķ���" << token.str << "ʱ��������" << endl;
					exit(ERROR_INTERMEDIATE_CODE_GENERATER);
				}
				break;
			case ActionItem::A_SHIFT: // �ƽ�
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

				// ȡ��һ��token
				if (token == epsilonToken) {
					token = nextEpsilonToken;
				}
				else {
					token = this->lexcalAnalyser.getNextToken();
				}
				break;
			case ActionItem::A_REDUCTION: // ��Լ

				/* ���·���ջ��״̬ջ */
				// �������ACTION����Ĺ�Լʽ�Ҳ�
				for (int i = actionItem.production.second.size() - 1; i >= 0; i--) {
					if (symbolStack.top().str == actionItem.production.second[i]) { // ��Լ��ջ
						symbolStack.pop();
						stateStack.pop();
					}
					else {
						cerr << "ERROR: �м����������ɨ�赽��" << to_string(lineCnt) << "�еķ���" << token.str << "ʱ��������" << endl;
						exit(ERROR_INTERMEDIATE_CODE_GENERATER);
					}
				}
				symbolStack.push(Token(actionItem.production.first, -1));
				pos = this->syntaxAnalyser.getVNPos(symbolStack.top().str);
				stateStack.push(this->syntaxAnalyser.GOTO[stateStack.top()][pos]);

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

				this->syntaxTreeInfo.root = stnp;

				// �������������
				this->semanticAnalyser.analyse(this->generateProductionStr(actionItem.production), stnp, this->lexcalAnalyser.varTable);

				break;
			default:
				break;
		}
	}
}

/*============================== �м���������� ==============================*/