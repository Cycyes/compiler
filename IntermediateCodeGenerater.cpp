#include "IntermediateCodeGenerater.h"

/*============================== 语法树信息 ==============================*/

SyntaxTreeInfo::SyntaxTreeInfo() {
	maxTreeLevel = 0;
	leafNum = 0;

	root = NULL;
}

SyntaxTreeInfo::~SyntaxTreeInfo() {

}

void SyntaxTreeInfo::updateInfo(SyntaxTreeNode* n, const int& l) {
	// 更新maxTreeLevel
	if (l > maxTreeLevel) {
		maxTreeLevel = l;
	}

	// 更新叶子节点数量
	if (n->children.size() == 0) {
		leafNum++;
	}
}

/*============================== 语法树信息 ==============================*/



/*============================== 中间代码生成器 ==============================*/

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
			ret += "<" + p.second[i] + ">"; // 该字符在非终结符集中
		}
		else {
			ret += "'" + p.second[i] + "'"; // 该字符在终结符集中
		}
	}
	return ret;
}

void IntermediateCodeGenerater::generateTreeLevel(SyntaxTreeNode* n, const int& l) {
	
	// 更新树的信息
	this->syntaxTreeInfo.updateInfo(n, l);

	n->level = l;

	// 遍历子树
	for (int i = 0; i < n->children.size(); i++) {
		generateTreeLevel(n->children[i], l + 1);
	}
}

void IntermediateCodeGenerater::analyse() {
	stack<int> stateStack; // 状态栈
	stack<Token> symbolStack; // 符号栈
	stateStack.push(0);
	symbolStack.push(Token("#", -1));
	
	Token epsilonToken = Token("epsilon", -1);
	Token nextEpsilonToken;
	Token token = this->lexcalAnalyser.getNextToken();

	SyntaxTreeNode* stnp;
	stack<SyntaxTreeNode*> syntaxTreeNodeStack;

	int pos;

	while (token.str != "ERROR") {

		// token为'\n'
		if (token.str == "NL") {
			this->lineCnt++;
			token = this->lexcalAnalyser.getNextToken();
			continue;
		}

		// 判断token是否属于终结符 
		pos = this->syntaxAnalyser.getVTPos(token.str);
		if (pos == NOT_FOUND) {
			cerr << "ERROR: 中间代码生成器发现非终结符!" << endl;
			exit(ERROR_INTERMEDIATE_CODE_GENERATER);
		}

		// 取action表项
		ActionItem actionItem = this->syntaxAnalyser.ACTION[stateStack.top()][pos];

		switch (actionItem.status) {
			case ActionItem::A_ACC:
				// 完成分析，生成语法树
				this->generateTreeLevel(this->syntaxTreeInfo.root, 0);
				return;
				break;
			case ActionItem::A_ERROR:
				if (token != epsilonToken) {
					nextEpsilonToken = token;
					token = epsilonToken;
				}
				else {
					cerr << "ERROR: 中间代码生成器扫描到第" << to_string(lineCnt) << "行的符号" << token.str << "时发生错误" << endl;
					exit(ERROR_INTERMEDIATE_CODE_GENERATER);
				}
				break;
			case ActionItem::A_SHIFT: // 移进
				stnp = new(nothrow) SyntaxTreeNode;
				if (!stnp) {
					cerr << "ERROR: 中间代码生成器生成语法树节点时空间不足" << endl;
					exit(ERROR_NEW);
				}
				stnp->token = token;
				syntaxTreeNodeStack.push(stnp);

				// 进栈
				stateStack.push(actionItem.nextState);
				symbolStack.push(token);

				// 取下一个token
				if (token == epsilonToken) {
					token = nextEpsilonToken;
				}
				else {
					token = this->lexcalAnalyser.getNextToken();
				}
				break;
			case ActionItem::A_REDUCTION: // 规约

				/* 更新符号栈和状态栈 */
				// 逆向遍历ACTION表项的规约式右部
				for (int i = actionItem.production.second.size() - 1; i >= 0; i--) {
					if (symbolStack.top().str == actionItem.production.second[i]) { // 规约出栈
						symbolStack.pop();
						stateStack.pop();
					}
					else {
						cerr << "ERROR: 中间代码生成器扫描到第" << to_string(lineCnt) << "行的符号" << token.str << "时发生错误" << endl;
						exit(ERROR_INTERMEDIATE_CODE_GENERATER);
					}
				}
				symbolStack.push(Token(actionItem.production.first, -1));
				pos = this->syntaxAnalyser.getVNPos(symbolStack.top().str);
				stateStack.push(this->syntaxAnalyser.GOTO[stateStack.top()][pos]);

				// 更新语法树栈
				stnp = new(nothrow) SyntaxTreeNode;
				if (!stnp) {
					cerr << "ERROR: 中间代码生成器生成语法树节点时空间不足" << endl;
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

				// 调用语义分析器
				this->semanticAnalyser.analyse(this->generateProductionStr(actionItem.production), stnp, this->lexcalAnalyser.varTable);

				break;
			default:
				break;
		}
	}
}

/*============================== 中间代码生成器 ==============================*/