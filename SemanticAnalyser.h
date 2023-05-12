#ifndef SEMANTICANALYSER_H
#define SEMANTICANALYSER_H

#include "SyntaxAnalyser.h"

enum type { INT, VOID };
enum kind { VAR, FUNC, ARRAY };


struct treeNode {
	int level = 0;
	treeNode* parent = NULL;
	vector<treeNode*> children;
	pair<string, int> content;
	type t = INT;
	kind k = VAR;
	int n = 0;
	int width = 0;
	vector<int> dimension;
	vector<string> params;
	string place;
	int quad;
	int true_list;
	int false_list;
	int x = -1;//����ͼʹ��
	int y = -1;//����ͼʹ��
	treeNode() { clear(); }
	void clear()
	{
		level = -1;
		parent = NULL;
		children.clear();
		content = pair<string, int>("", -1);
	}
};


struct symbolTable;
struct symbolTableItem
{
	int id;
	type t;
	kind k;
	int offset;
	vector<int> dimension;
	symbolTable* proctable = NULL;
};
struct symbolTable {
	//˫����
	symbolTable* previous = NULL;
	symbolTable* next = NULL;
	//����
	symbolTable* parent = NULL;
	int width = 0;
	vector<symbolTableItem> table;
	void clear()
	{
		previous = NULL;
		next = NULL;
		width = 0;
		table.clear();
	}
	void enter(int id, type t, kind k, int offset)
	{
		symbolTableItem e;
		e.id = id;
		e.t = t;
		e.k = k;
		e.offset = offset;
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i].id == id)
			{
				//				cerr << "ERROR: " << id << "�ض���\n";
				//				exit(-1);
				throw string("ERROR: ����������ű����:") + to_string(id) + string("�ض���\n");
			}
		}
		table.push_back(e);
	}
	void enterdimension(int id, vector<int>dimension)
	{
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i].id == id && ((table[i].k == ARRAY) || (table[i].k == FUNC)))
			{
				table[i].dimension = dimension;
				break;
			}
		}
	}
	void enterproc(int id, symbolTable* newtable)
	{
		for (auto i = 0; i < table.size(); i++)
		{
			if (table[i].id == id && table[i].k == FUNC)
			{
				table[i].proctable = newtable;
			}
		}
	}
};
struct TASitem//three address statement item
{
	string op;
	string arg1;
	string arg2;
	string result;
};
class semanticAnalysis
{
private:

	vector<symbolTable*> table_stack;
	vector<int> offset_stack;
	int temp_counter = -1;
	string newtemp();
	string lookup(int id);
	symbolTableItem* find(int id);
	int nextstat();
	void EMIT(string op, string arg1, string arg2, string result);
public:
	symbolTable* last_table = NULL;
	symbolTable* global_table;
	vector<TASitem>intermediate_code;
	~semanticAnalysis();
	void analysis(string token, treeNode* root, map<int, string> nameTable);
	void showTables(map<int, string> name_table);
	void showIntermediateCode();
};

#endif