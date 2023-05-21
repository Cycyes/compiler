#include "IntermediateCodeGenerater.h"

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

void IntermediateCodeGenerater::analyse(const string& filename) {

	// 打开文件
	ofstream outfile(filename, ios::out | ios::binary);
	if (!outfile.is_open()) {
		cerr << "ERROR: 中间代码生成器无法打开输出文件!" << endl;
		exit(ERROR_OPEN_FILE);
	}
	outfile << "digraph mygraph {\n";
	int cnt = 0;
	vector<int> cntStack;

	// 初始化栈
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
			case ActionItem::A_ACC: {
				// 文件输出
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
					cerr << "ERROR: 中间代码生成器扫描到第" << to_string(lineCnt) << "行的符号" << nextEpsilonToken.str << "时发生错误" << endl;
					exit(ERROR_INTERMEDIATE_CODE_GENERATER);
				}
				break;
			}
			case ActionItem::A_SHIFT: { // 移进
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

				// 文件输出
				cntStack.push_back(cnt);
				outfile << "n" << cnt++ << "[label=\"" << token.str << "\",color=red];" << endl;

				// 取下一个token
				if (token == epsilonToken) {
					token = nextEpsilonToken;
				}
				else {
					token = this->lexcalAnalyser.getNextToken();
				}
				break;
			}
			case ActionItem::A_REDUCTION: { // 规约

				/* 更新符号栈和状态栈 */
				// 逆向遍历ACTION表项的规约式右部
				vector<int> cntLeft;
				for (int i = actionItem.production.second.size() - 1; i >= 0; i--) {
					if (symbolStack.top().str == actionItem.production.second[i]) { // 规约出栈
						symbolStack.pop();
						stateStack.pop();

						// 
						cntLeft.push_back(cntStack.back());
						cntStack.pop_back();
					}
					else {
						cerr << "ERROR: 中间代码生成器扫描到第" << to_string(lineCnt) << "行的符号" << token.str << "时发生错误" << endl;
						exit(ERROR_INTERMEDIATE_CODE_GENERATER);
					}
				}
				symbolStack.push(Token(actionItem.production.first, -1));
				pos = this->syntaxAnalyser.getVNPos(symbolStack.top().str);
				stateStack.push(this->syntaxAnalyser.GOTO[stateStack.top()][pos]);

				// 文件输出
				cntStack.push_back(cnt);
				outfile << "n" << cnt++ << "[label=\"" << actionItem.production.first << "\"];\n";

				if (cntLeft.size() != 0) {
					for (auto t = cntLeft.begin(); t != cntLeft.end(); t++) {
						outfile << "n" << cnt - 1 << " -> " << "n" << *t << ";\n";
					}
				}
				else { //空串
					outfile << "e" << cnt << "[label=\"@\"];\n";
					outfile << "n" << cnt - 1 << " -> " << "e" << cnt << ";\n";
				}

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

				// 调用语义分析器
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
		cerr << "ERROR: 中间代码文件打开失败!" << endl;
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

/*============================== 中间代码生成器 ==============================*/