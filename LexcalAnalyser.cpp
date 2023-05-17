#include "LexcalAnalyser.h"

/*============================== Token ==============================*/

Token::Token(const string s, const int c) {
	str = s;
	code = c;
}

Token::~Token() {

}

bool Token::operator == (const Token& t) {
	return str == t.str && code == t.code;
}

bool Token::operator != (const Token& t) {
	return (str != t.str) || (code != t.code);
}

/*============================== Token ==============================*/



/*============================== LexcalAnalyser ==============================*/

LexcalAnalyser::LexcalAnalyser() {
	this->sourceCode = "";
	this->sourceCodePos = 0;
	this->state = LA_INIT;
	this->varCnt = 0;
	this->lineCnt = 1;
	this->varTable.clear();
}

LexcalAnalyser::~LexcalAnalyser() {

}

void LexcalAnalyser::setSourceCode(string s) {
	sourceCode = s;
}

Token LexcalAnalyser::getNextToken() {
	string buffer;

	while (sourceCodePos < sourceCode.size()) {
		// 获取当前字符和下一个字符
		string ch, nextCh;
		ch = sourceCode[sourceCodePos];
		nextCh = sourceCodePos + 1 < sourceCode.size() ? sourceCode[sourceCodePos + 1] : '\0';

		// 词法分析
		switch (state) {
			case LA_INIT:
				if (skipCharSet.count(ch)) {
					sourceCodePos++;
				}
				else if (ch == "\n") {
					sourceCodePos++;
					lineCnt++;
					return strTokenMap.at(ch);
				}
				else if (ch == "/") {
					if (nextCh == "/") {
						sourceCodePos += 2;
						state = LA_COMMENT_LINE;
					}
					else if (nextCh == "*") {
						sourceCodePos += 2;
						state = LA_COMMENT_SEG;
					}
					else {
						state = LA_OPERATOR;
					}
				}
				else if (ch[0] >= '0' && ch[0] <= '9') {
					buffer.erase();
					buffer += ch;
					sourceCodePos++;
					state = LA_NUMBER;
				}
				else if (unaryOperatorSet.count(ch)) {
					state = LA_OPERATOR;
				}
				else if ((ch[0] >= 'a' && ch[0] <= 'z') || (ch[0] >= 'A' && ch[0] <= 'Z')) {
					buffer.erase();
					buffer += ch;
					sourceCodePos++;
					state = LA_STR;
				}
				else {
					state = LA_ERROR;
				}
				break;
			case LA_COMMENT_LINE:
				if (ch != "\n") {
					sourceCodePos++;
				}
				else {
					sourceCodePos++;
					state = LA_INIT;
					lineCnt++;
					return strTokenMap.at(ch);
				}
				break;
			case LA_COMMENT_SEG:
				if (ch == "*" && nextCh == "/") {
					sourceCodePos += 2;
					state = LA_INIT;
				}
				else if (ch == "\n") {
					sourceCodePos++;
					lineCnt++;
					return strTokenMap.at(ch);
				}
				else {
					sourceCodePos++;
				}
				break;
			case LA_NUMBER:
				if (ch[0] >= '0' && ch[0] <= '9') {
					buffer += ch;
					sourceCodePos++;
				}
				else {
					state = LA_INIT;
					return Token("NUM", atoi(buffer.c_str()));
				}
				break;
			case LA_OPERATOR:
				if (binaryOperatorSet.count(ch + nextCh)) {
					sourceCodePos += 2;
					state = LA_INIT;
					return strTokenMap.at(ch + nextCh);
				}
				else {
					sourceCodePos++;
					state = LA_INIT;
					return strTokenMap.at(ch);
				}
				break;
			case LA_STR:
				if ((ch[0] >= 'a' && ch[0] <= 'z') || (ch[0] >= 'A' && ch[0] <= 'Z') || ch[0] == '_' || (ch[0] >= '0' && ch[0] <= '9')) {
					buffer += ch;
					sourceCodePos++;
				}
				else {
					if (reservedWordSet.count(buffer)) {
						state = LA_INIT;
						return strTokenMap.at(buffer);
					}
					else {
						state = LA_INIT;
						bool flag = false;
						int varId = 0;

						for (auto i = varTable.begin(); i != varTable.end(); i++) {
							if (i->second == buffer) {
								flag = true;
								varId = i->first;
							}
						}
						if (!flag) {
							varTable[varCnt] = buffer;
							return Token("ID", varCnt++);
						}
						else {
							return Token("ID", varId);
						}
					}
				}
				break;
			case LA_ERROR:
				cerr << "ERROR: 词法分析器第" << lineCnt << "行的字符: " << ch << "出错!" << endl;
				exit(ERROR_LEXCAL_ANALYSE);
		}
	}
	return Token("#", -1);
}

/*============================== LexcalAnalyser ==============================*/