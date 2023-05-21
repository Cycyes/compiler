#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <ctime>

#include <algorithm>

#include <set>
#include <vector>
#include <map>
#include <stack>

#define NOT_FOUND -1

#define ERROR_OPEN_FILE -1
#define ERROR_NEW       -2

#define ERROR_LEXCAL_ANALYSE -1
#define ERROR_SYNTAX_ANALYSE -2
#define ERROR_SYNTAX_ANALYSE_GRAMMAR_LACK_PRODUCTIONRIGHT -3
#define ERROR_SYNTAX_ANALYSE_GRAMMAR_PRODUCTION_VN_MATCH -4
#define ERROR_SEMANTIC_ANALYSE -5
#define ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE -6
#define ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_REDEFINE -7
#define ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTDEFINE -8
#define ERROR_SEMANTIC_ANALYSE_SYMBOLTABLE_NOTCOMPLETEINDEX -9
#define ERROR_SEMANTIC_ANALYSE_FUNCPARA -10
#define ERROR_SEMANTIC_ANALYSE_ARRAYSHAPE -11

#define ERROR_INTERMEDIATE_CODE_GENERATER  -12

#define ERROR_OPTIMIZER - 13
#define ERROR_OPTIMIZER_REDEFINEMAIN -14
#define ERROR_OPTIMIZER_NOTDEFINEMAIN -15

#define ERROR_OBJECTCODEGENERATER -16

using namespace std;

bool is_num(string str);

#endif