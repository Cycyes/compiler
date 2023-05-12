#include "IntermediateCodeGenerater.h"

syntaxAnalysis::~syntaxAnalysis()
{
	if (reductionTreeRoot != NULL)
	{
		vector<SyntaxTreeNode*> idstack;
		int nowlevel = reductionTreeRoot->level;
		idstack.push_back(reductionTreeRoot);
		while (!idstack.empty())
		{
			SyntaxTreeNode* nownode = idstack.front();
			idstack.erase(idstack.begin());
			for (int i = 0; i < nownode->children.size(); i++)
			{
				idstack.push_back(nownode->children[i]);
			}
			delete nownode;
		}
	}
}
void syntaxAnalysis::initializeLR1() {
	G.analyse();
}
void syntaxAnalysis::getInput(string input)
{
	//L.setFileString(input);
	L.setSourceCode(input);
}
void syntaxAnalysis::analysis()
{
	vector<int>state;
	vector<pair<string, int>>symbol;
	state.push_back(0);
	symbol.push_back(pair<string, int>(string("#"), -1));
	pair<string, int> epsilon_lexis = pair<string, int>("epsilon", -1);
	pair<string, int> epsilon_next_lexis;
	//pair<string, int> lexis = L.getLexic();
	Token tt = L.getNextToken();
	pair<string, int> lexis = pair<string, int>(tt.str, tt.code);
	retcode = 1;
	stack<SyntaxTreeNode*> treeNodeStack;
	SyntaxTreeNode* tp;

	vector<string> strstate;
	vector<string> strsymbol;
	vector<string> input;
	do
	{
		if (lexis.first == "NL")
		{
			retcode++;
			Token tt = L.getNextToken();
			lexis = pair<string, int>(tt.str, tt.code);
			//lexis = L.getLexic();
			continue;
		}

		if (find(G.VT.begin(), G.VT.end(), lexis.first) == G.VT.end())
		{
			//cerr << "ERROR: " << lexis.first << "is not belong to VT" << endl;
			//cerr << "ERROR: " << lexis.first << "不是终结符" << endl;
			string expmsg = string("ERROR: 语法分析器发现") + lexis.first + string("不是终结符\n");
			throw expmsg;
		}
		int pos = 0;
		for (auto i = G.VT.begin(); i != G.VT.end(); i++, pos++) {
			if (lexis.first == (*i)) {
				break;
			}
		}
		ActionItem item = G.ACTION[state.back()][pos];
		// ActionItem item = G.ACTION[state.back()][find(G.VT.begin(), G.VT.end(), lexis.first) - G.VT.begin()];
		//actionItem item = G.ACTION[state.back()][find(G.VT.begin(), G.VT.end(), lexis.first) - G.VT.begin()];

		/*vector<actionItem> state_actions;
		vector<string> prod;*/

		// switch (item.status)
		switch (item.status)
		{
			case ActionItem::A_ACC:
				_genTreeLevel(reductionTreeRoot, 0);
				return;
				break;
			case ActionItem::A_ERROR:
				if (lexis != epsilon_lexis)
				{
					epsilon_next_lexis = lexis;
					lexis = epsilon_lexis;
				}
				else
				{
					//cerr << "ERROR: an error has occured when it meets " << lexis.first << " in line " << retcode << endl;
					//cerr << "ERROR: 扫描到第" << retcode << "行的单词" << lexis.first << "时发生错误" << endl;
					//cerr << "now the symbol stack is\n";
					//cerr << "目前符号栈内容为\n";
					//cerr << "|---stack---\n";
					//while (!symbol.empty())
					//{
					//	cerr << "|---" << symbol.back().first << endl;
					//	symbol.pop_back();
					//}
					//cerr << "|---stack---\n";
					string expmsg;
					expmsg = string("ERROR: 语法分析器扫描到第") + to_string(retcode) + string("行的单词") + lexis.first + string("时发生错误\n");
					expmsg += string("目前符号栈内容为\n|---stack---\n");
					while (!symbol.empty())
					{
						expmsg += string("|---") + symbol.back().first + string("\n");
						symbol.pop_back();
					}
					expmsg += string("|---stack---\n");
					throw expmsg;
				}
				break;
			case ActionItem::A_SHIFT:
				tp = new(nothrow)SyntaxTreeNode;
				if (!tp) { exit(-2); }
				tp->content = lexis;
				treeNodeStack.push(tp);

				state.push_back(item.nextState);
				symbol.push_back(lexis);

				if (lexis == epsilon_lexis)
				{
					lexis = epsilon_next_lexis;
				}
				else
				{
					Token tt = L.getNextToken();
					lexis = pair<string, int>(tt.str, tt.code);
					//lexis = L.getLexic();
				}
				break;
			case ActionItem::A_REDUCTION:
			{
				for (int i = item.production.second.size() - 1; i >= 0; i--)
				{
					if (symbol.back().first == item.production.second[i])
					{
						symbol.pop_back();
						state.pop_back();
					}
					else
					{
						//cerr << "ERROR: an error has occured when " << symbol.back().first << "is not equal with " << item.p.second[i] << endl;
						//cerr << "ERROR: 发生了一个规约错误：" << symbol.back().first << "与" << item.p.second[i] << "不相同\n";
						//cerr << "you can correct the error according to the following production\n";
						//cerr << "你可以参考以下产生式改正错误\n";
						//cerr << item.p.first << "->";
						//for (auto k = 0; k < item.p.second.size(); k++)
						//{
						//	cout << item.p.second[k] << ' ';
						//}

						string expmsg;
						expmsg = string("ERROR: 语法分析器扫描到第") + to_string(retcode) + string("行的单词") + lexis.first + string("时发生错误\n");
						expmsg += string("发生了一个规约错误：") + symbol.back().first + string("与") + item.production.second[i] + string("不相同\n");
						expmsg += string("参考以下产生式改正错误\n");
						expmsg += item.production.first + string("->");
						for (auto k = 0; k < item.production.second.size(); k++)
						{
							expmsg += item.production.second[k] + string(" ");
						}
						expmsg += string("\n");
						throw expmsg;
					}
				}
				tp = new(nothrow)SyntaxTreeNode;
				if (!tp) { exit(-2); }
				tp->content = pair<string, int>(item.production.first, -1);
				for (int i = 0; i < item.production.second.size(); i++)
				{
					treeNodeStack.top()->parent = tp;
					tp->children.push_back(treeNodeStack.top());
					treeNodeStack.pop();
				}
				reverse(tp->children.begin(), tp->children.end());
				reductionTreeRoot = tp;
				treeNodeStack.push(tp);

				{
					string token = "<" + item.production.first + ">::=";
					for (auto i = 0; i < item.production.second.size(); i++)
					{
						if (find(G.VN.begin(), G.VN.end(), item.production.second[i]) != G.VN.end())
							token += "<" + item.production.second[i] + ">";
						else
							token += "'" + item.production.second[i] + "'";
					}
					try {
						S.analyse(token, tp, L.varTable);
						//S.analysis(token, tp, L.nameTable);
					}
					catch (string expmsg)
					{
						expmsg = string("ERROR: 语法分析器扫描到第") + to_string(retcode) + string("行的单词") + lexis.first + string("时发生错误\n") + expmsg;
						throw expmsg;
					}
				}

				symbol.push_back(pair<string, int>(item.production.first, -1));
				int pos = 0;
				for (auto i = G.VN.begin(); i != G.VN.end(); i++, pos++) {
					if ((*i) == symbol.back().first) {
						break;
					}
				}
				state.push_back(G.GOTO[state.back()][pos]);
				//state.push_back(G.GOTO[state.back()][find(G.VN.begin(), G.VN.end(), symbol.back().first) - G.VN.begin()]);
				break;
			}
			default:break;
		}

		strstate.clear();
		strsymbol.clear();
		input.clear();
		for (int i = 0; i < state.size(); i++)
		{
			strstate.push_back(to_string(state[i]));
		}
		for (int i = 0; i < symbol.size(); i++)
		{
			strsymbol.push_back(symbol[i].first);
		}
		input.push_back(lexis.first);
		/*for (int i = pos; i < inputString.size(); i++)
		{
			input.push_back(inputString[i].first);
		}*/
		history.push_back({ strstate, strsymbol, input });
	} while (lexis.first != "ERROR");
}
void syntaxAnalysis::showHistory()
{
	for (int i = 0; i < history.size(); i++)
	{
		cout << "---Step:" << i << "---";
		for (int j = 0; j < history[i].size(); j++)
		{
			for (int k = 0; k < history[i][j].size(); k++)
			{
				cout << history[i][j][k] << " ";
			}
			cout << "\t\t\t";
		}
		cout << endl;
	}
}
void syntaxAnalysis::_genTreeLevel(SyntaxTreeNode* nownode, int nowlevel)
{
	if (nowlevel > maxTreeLevel)
		maxTreeLevel = nowlevel;
	if (nownode->children.size() == 0)
		leafNum++;
	nownode->level = nowlevel;
	for (int i = 0; i < nownode->children.size(); i++)
	{
		_genTreeLevel(nownode->children[i], nowlevel + 1);
	}
}
void syntaxAnalysis::showTree()
{
	cout << "----------------ReductionTree-------------------\n";
	vector<SyntaxTreeNode*> idstack;
	int nowlevel = reductionTreeRoot->level;
	idstack.push_back(reductionTreeRoot);
	while (!idstack.empty())
	{
		SyntaxTreeNode* nownode = idstack.front();
		idstack.erase(idstack.begin());
		if (nownode->level == nowlevel)
			cout << nownode->level << ' ';
		else
		{
			cout << '\n' << nownode->level << ' ';
			nowlevel++;
		}
		for (int i = 0; i < nownode->children.size(); i++)
		{
			idstack.push_back(nownode->children[i]);
		}
	}
	cout << "----------------End-------------------\n";
}
void syntaxAnalysis::_showTree2(SyntaxTreeNode* nownode)
{
	for (int i = 0; i < nownode->level; i++)
		cout << string("|--");
	cout << "< " << nownode->content.first << ", ";
	if (nownode->content.first == string("ID"))
	{
		cout << L.varTable.find(nownode->content.second)->second;
		//cout << L.nameTable.find(nownode->content.second)->second;
	}
	else {
		cout << nownode->content.second;
	}
	cout << " >" << endl;
	for (int i = 0; i < nownode->children.size(); i++)
		_showTree2(nownode->children[i]);
}
void syntaxAnalysis::showTree2()
{
	cout << "----------------ReductionTree2-------------------\n";
	_showTree2(reductionTreeRoot);
	cout << "----------------End-------------------\n";
}