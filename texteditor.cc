#include "texteditor.h"
#include <iostream>
#include <cassert>

void TextEditor::reAdjustCursor(bool restrictCol)  {
	// if the cursor is out of bounds in some way, move it back in bounds
	// restrictCol can be set to false to let the cursor keep its column
	// when traversing through a row of length < col
	if(cursor.line < 0) cursor.line = 0;
	if(cursor.line >= constBase->numLines())
		cursor.line = constBase->numLines()-1;
	if(cursor.column < 0) cursor.column = 0;
	if(restrictCol) {
		if(cursor.column >= (int)(*constBase)[cursor.line].size())
			cursor.column = (*constBase)[cursor.line].size()-1;
	}
}

void TextEditor::moveCursor(int ud, int lr, bool vis) {
	reAdjustCursor(lr != 0);
	cursor.line += ud;
	cursor.column += lr;
	reAdjustCursor(lr != 0);
	if(lr != 0) cursor.updateVisuals();
	if(lr == 0 && vis) {// attempt to preserve visual location
		size_t visloc = 0;
		for(size_t i = 0; i < cursor.visColumn && i < (*constBase)[cursor.visLine].size(); i++) {
			if((*constBase)[cursor.visLine][i] == '\t') {
				visloc = (visloc+4)/4*4;
			}
			else visloc++;
		}
		size_t nvisloc = 0;
		for(size_t i = 0; i < (*constBase)[cursor.line].size(); i++) {
			if((*constBase)[cursor.line][i] == '\t') {
				nvisloc = (nvisloc+4)/4*4;
			}
			else nvisloc++;
			if(nvisloc >= visloc) {
				cursor.column = i+1;
				break;
			}
		}
	}
}

void TextEditor::setCursor(int line, int col) {
	cursor.line = line;
	cursor.column = col;
	reAdjustCursor(true);
}

bool TextEditor::advanceCursor(Cursor& c, int lr) { // returns whether or not we've hit the end
	if(c.line < 0) c.line = 0;
	if(c.line >= constBase->numLines()) c.line = constBase->numLines()-1;
	if(c.column < 0) c.column = 0;
	if(c.column >= (int)(*constBase)[c.line].size()) c.column = (*constBase)[c.line].size()-1;
	if(lr < 0) {
		lr = -lr;
		while(lr > 0) {
			if(c.column < lr) {
				lr -= c.column+1;
				c.column = 0;
				if(c.line == 0) return true;
				else c.line--;
				c.column = (*constBase)[c.line].size()-1;
			}
			else {
				c.column -= lr;
				lr = 0;
			}
		}
	}
	else {
		while(lr > 0) {
			c.column += lr;
			lr = 0;
			if(c.column >= (*constBase)[c.line].size()) {
				lr = c.column - (*constBase)[c.line].size();
				c.column = (*constBase)[c.line].size()-1;
				if(c.line+1 == constBase->numLines()) return true;
				else c.line++;
				c.column = 0;
			}
		}
	}
	return false;
}

bool isWhitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
int getGroup(char c) {
	return (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')?0:
		(isWhitespace(c)?1:2));
}


Cursor TextEditor::getPreviousWord() {
	reAdjustCursor(true);
	Cursor cur = cursor, prv = cur;
	if(cur.line == 0 && cur.column == 0) return cur;
	advanceCursor(cur, -1);
	while(cur.line > 0 || cur.column > 0) {
		prv = cur;
		char curC = (*constBase)[cur.line][cur.column];
		advanceCursor(cur, -1);
		if(!isWhitespace(curC)) {
			char nxtC = (*constBase)[cur.line][cur.column];
			if(getGroup(curC) != getGroup(nxtC)) return prv;
		}
	}
	return cur;
}
std::string TextEditor::getIndentation(int line) {
	std::string ret = "";
	for(char c: (*constBase)[line]) {
		if(c == '\t' || c == ' ') ret.push_back(c);
		else return ret;
	}
	return ret;
}

Cursor TextEditor::getNextWord() {
	// vim appears to use the following method:
	// when characters transition from '[a-zA-Z0-9] into anyting else, 
	// then a split occurs. strings in the resulting split
	// that only contain whitespace are ignored.
	reAdjustCursor(true);
	Cursor cur = cursor;
	while(cur.line+1 < constBase->numLines() || cur.column+1 < (int)(*constBase)[cur.line].size()) {
		char curC = (*constBase)[cur.line][cur.column];
		advanceCursor(cur, 1);
		char nxtC = (*constBase)[cur.line][cur.column];
		if(getGroup(curC) != getGroup(nxtC) && !isWhitespace(nxtC)) return cur;
	}
	return cur;
}

void TextEditor::jumpToNonWhitespace(bool linewise) {
	reAdjustCursor(true);
	Cursor& cur = cursor;
	while(cur.line+1 < constBase->numLines() || cur.column+1 < (int)(*constBase)[cur.line].size()) {
		char curC = (*constBase)[cur.line][cur.column];
		if(!isWhitespace(curC)) return;
		if(cur.column+1 == (int)(*constBase)[cur.line].size()) {
			if(linewise) {
				cur.line = std::min((int)constBase->numLines()-1, cur.line+1);
				cur.column=0;
			}
			else return;
		}
		else cur.column++;
	}
}

void TextEditor::jumpByWord(int words) {
	reAdjustCursor(true);
	while(words > 0) cursor = getNextWord(), words--;
	while(words < 0) cursor = getPreviousWord(), words++;
}

void TextEditor::jumpRight(char c) {
	reAdjustCursor(true);
	Cursor old = cursor;
	moveCursor(0, 1);
	while(cursor.column+1 < (int)(*constBase)[cursor.line].size() &&
		(*constBase)[cursor.line][cursor.column] != c) {
		moveCursor(0, 1);
	}
	if((*constBase)[cursor.line][cursor.column] != c) {// c not found
		cursor = old;
	}
}
void TextEditor::jumpLeft(char c) {
	reAdjustCursor(true);
	Cursor old = cursor;
	moveCursor(0, -1);
	while(cursor.column > 0 &&
		(*constBase)[cursor.line][cursor.column] != c) {
		moveCursor(0, -1);
	}
	if((*constBase)[cursor.line][cursor.column] != c) {// c not found
		cursor = old;
	}
}
std::string isolateFlatC(const std::string& line) {
	std::string linecpy = line;
	size_t start = 0;
	while(start < linecpy.size() && (linecpy[start] == ' ' || linecpy[start] == '\t')) start++;
	linecpy.erase(linecpy.begin(), linecpy.begin()+start);
	while(linecpy.size()>0 && (linecpy.back() == ' ' || linecpy.back() == '\t')) linecpy.pop_back();
	auto match = [](const std::string& s, const std::string& p)->bool{return s.substr(0, p.size()) == p;};
	std::vector<std::string> lookfor = {"#ifdef", "#if", "#else", "#elif", "#endif"};
	for(auto& i: lookfor) if(match(linecpy, i)) return i;
	return "";
}
bool isNewOpening(const std::string& line, const std::string& pattern) {
	auto match = isolateFlatC(line);
	if(pattern == "#endif") return match == pattern;
	else return match == "#if" || match == "#ifdef";
}
bool isClosing(const std::string& line, const std::string& pattern) {
	auto match = isolateFlatC(line);
	if(pattern == "#endif") return match == "#if" || match == "#ifdef";
	else return match == "#endif";
}
bool flatCMatches(const std::string& line, const std::string& pattern) {
	auto match = isolateFlatC(line);
	if(match == "") return false;
	if(pattern == "#if" || pattern == "#ifdef" || pattern == "#elif") {
		if(match == "#if" || match == "#ifdef") return false;
		else return true;
	}
	if(pattern == "#else") {
		if(match == "#endif") return true;
		else return false;
	}
	if(pattern == "#endif") {
		return match == "#if" || match == "#ifdef";
	}
	return false;
}
std::string isolateC(const std::string& line, size_t position) { // helper function for jumpC
	if(position < 0 || position >= (int)line.size()) return ""; // found nothing
	if(line[position] == '{') return "{";
	if(line[position] == '}') return "}";
	if(line[position] == '[') return "[";
	if(line[position] == ']') return "]";
	if(line[position] == '(') return "(";
	if(line[position] == ')') return ")";
	if(position > 0 && line[position-1] == '/' && line[position] == '*') return "/*";
	if(position+1 < (int)line.size() && line[position] == '*' && line[position+1] == '/') return "*/";
	return "";
}
int CMatches(const std::string& line, const std::string& pattern, size_t idx) { // helper function for jumpC
	// returns -1 if closing braces are found
	// returns 1 if new opening is found
	// returns 0 if nothing is found
		auto match = isolateC(line, idx);
		if(match == "") return 0;
		if(match == pattern) return -1;
		if((match == "{" && pattern == "}") || 
			(match == "(" && pattern == ")") ||
			(match == "[" && pattern == "]") ||
			(match == "}" && pattern == "{") || 
			(match == ")" && pattern == "(") ||
			(match == "]" && pattern == "[") ||
			(match == "/*" && pattern == "*/") ||
			(match == "*/" && pattern == "/*")) return 1;
		return 0;
}
void TextEditor::jumpC() {
	reAdjustCursor(true);
	// we're looking for:
	// ({[]})
	// /* */
	// #if, #ifdef, #else, #elif, #endif
	std::string C = isolateC((*constBase)[cursor.line], cursor.column);
	if(C != "") {
		int dir = (C == "(" || C == "{" || C == "[" || C == "/*")? (int)C.size():(0-(int)C.size());
		Cursor tmp = cursor;
		int accum = 1;
		while(accum>0) {
			bool flag = advanceCursor(tmp, dir);
			accum -= CMatches((*constBase)[tmp.line], C, tmp.column);
			if(flag) break;
		}
		if(CMatches((*constBase)[tmp.line], C, tmp.column) == 1) setCursor(tmp.line, tmp.column);
	}
	else {
		std::string flatC = isolateFlatC((*constBase)[cursor.line]);
		if(flatC == "") return;
		int dir = (flatC != "#endif")? 1:-1;
		Cursor tmp = cursor;
		tmp.column = 0;
		int accum = 0;
		while(tmp.line+dir<constBase->numLines() && tmp.line+dir>=0) {
			tmp.line += dir;
			if(isNewOpening((*constBase)[tmp.line], flatC)) {
				accum++;
			}
			if(accum == 0) {
				//std::cout << (*constBase)[tmp.line] << " " << flatC << " " << flatCMatches((*constBase)[tmp.line], flatC) << "\n";
				if(flatCMatches((*constBase)[tmp.line], flatC)) break;
			}
			else if(isClosing((*constBase)[tmp.line], flatC)) accum--;
		}
		if(accum == 0 && flatCMatches((*constBase)[tmp.line], flatC)) {
			setCursor(tmp.line, tmp.column);
			jumpToNonWhitespace(false);
		}
	}
}


bool TextEditor::findNextOccurence(std::string str) {
	Cursor beginning = cursor;
	if(advanceCursor(cursor, 1)) {cursor.line = cursor.column = 0;}
	while((*constBase)[cursor.line].substr(cursor.column, str.size()) != str) {
		if(cursor == beginning) {
			return false;
		}
		if(advanceCursor(cursor, 1)) {cursor.line = cursor.column = 0;}
	}
	return true;
}
bool TextEditor::findPrevOccurence(std::string str) {
	Cursor beginning = cursor;
	if(advanceCursor(cursor, -1)) {cursor.line = cursor.column = 0;}
	while((*constBase)[cursor.line].substr(cursor.column, str.size()) != str) {
		if(cursor == beginning) {
			return false;
		}
		if(advanceCursor(cursor, -1)) {
			cursor.line = constBase->numLines()-1;
			cursor.column = (*constBase)[cursor.line].size()-1;
		}
	}
	return true;
}


void TextEditor::saveCursor() {
	cursorHistory.push_back(cursor);
}
void TextEditor::rollbackCursor() {
	if(cursorHistory.size()>0) {
		cursor = cursorHistory.back();
		cursorHistory.pop_back();
	}
}


void TextEditor::enterInsertMode() {
	assert(!isInInsertMode);
	insertTo = cursor;
	base->newSave();
	isInInsertMode = true;
}

void TextEditor::exitInsertMode() {
	if(isInInsertMode)
		base->rollback(1);
	base->newSave();
	if(isInInsertMode)
		setCursor(insertTo.line, insertTo.column);
	saveCursor();
	isInInsertMode = false;
}

void TextEditor::erase(size_t left, size_t right) {
	reAdjustCursor(true);
	while(left > 0) {
		std::string& cur = (*base)[cursor.line];
		if(left > (size_t)cursor.column) {
			cur.erase(cur.begin(), cur.begin()+cursor.column);
			if(cursor.line > 0) {
				left -= cursor.column + 1;
				std::string curcopy = cur;
				base->eraseLine(cursor.line, 1);
				cursor.line--;
				std::string& newcur = (*base)[cursor.line];
				cursor.column = newcur.size()-1;
				newcur.pop_back();
				newcur += curcopy;
			}
			else {
				cursor.column = std::max(0, cursor.column-(int)left);
				left = 0;
				// nothing left to erase
			}
		}
		else {
			cur.erase(cur.begin()+cursor.column-left, cur.begin()+cursor.column);
			cursor.column-=left;
			left = 0;
		}
	}
	while(right > 0) {
		std::string& cur = (*base)[cursor.line];
		if(cursor.line + 1 < base->numLines() && right + cursor.column >= cur.size()) {
			int numErased = cur.size()-cursor.column;
			cur.erase(cursor.column);
			cur += base->operator[](cursor.line+1);
			base->eraseLine(cursor.line+1, 1);
			right -= numErased;
		}
		else if (cursor.line+1 < base->numLines() || right+cursor.column < cur.size()) {
			cur.erase(cur.begin()+cursor.column, cur.begin()+cursor.column+right);
			right = 0;
		}
		else if (cursor.line+1 == base->numLines() && right+cursor.column >= cur.size()) {
			cur.erase(cur.begin()+cursor.column, cur.end()-1);
			right = 0;
		}
		else {
			right = 0;
		}
	}
}

void TextEditor::eraseInLine(size_t right) {
	std::string& cur = (*base)[cursor.line];
	if(cursor.column+right+1 > cur.size()) {
		cur.erase(cursor.column);
		cur.push_back('\n');
	}
	else cur.erase(cur.begin()+cursor.column, cur.begin()+cursor.column+right);
}

void TextEditor::eraseLine(size_t cnt) {
	base->eraseLine(cursor.line, cnt);
}

void TextEditor::insert(const std::string& s, bool autoIndent) {
	reAdjustCursor(true);
	for(size_t i = 0; i < s.size(); i++) if(s[i] == '\n') {
		insert(s.substr(0, i));
		if(autoIndent) {
			std::string prefix = getIndentation(cursor.line);
			std::string suffix = (*base)[cursor.line].substr(cursor.column);
			if(cursor.column < prefix.size()) {
				suffix.erase(0, prefix.size()-cursor.column);
				prefix.resize(cursor.column);
			}
			(*base)[cursor.line].erase(cursor.column);
			(*base)[cursor.line] += '\n';
			cursor.line++; cursor.column = prefix.size();
			base->newLine(cursor.line, 1);
			(*base)[cursor.line] = prefix + suffix;
		}
		else {
			std::string suffix = (*base)[cursor.line].substr(cursor.column);
			(*base)[cursor.line].erase(cursor.column);
			(*base)[cursor.line] += '\n';
			cursor.line++; cursor.column = 0;
			base->newLine(cursor.line, 1);
			(*base)[cursor.line] = suffix;
		}

		insert(s.substr(i+1), autoIndent);
		return;
	}
	std::string& cur = (*base)[cursor.line];
	cur.insert(cursor.column, s.c_str());
	moveCursor(0, s.size());
}
void TextEditor::insert(char c, bool autoIndent) {
	reAdjustCursor(true);
	if(c == '\n') {
		// for some reason, the way autoindent works is that
		// if the cursor is past the indentation, it works as expected
		// if the cursor is before the indentation, then the new line
		// only receives the indentation before the cursor
		if(autoIndent) {
			std::string prefix = getIndentation(cursor.line);
			std::string suffix = (*base)[cursor.line].substr(cursor.column);
			if(cursor.column < prefix.size()) {
				suffix.erase(0, prefix.size()-cursor.column);
				prefix.resize(cursor.column);
			}
			(*base)[cursor.line].erase(cursor.column);
			(*base)[cursor.line] += '\n';
			cursor.line++; cursor.column = prefix.size();
			base->newLine(cursor.line, 1);
			(*base)[cursor.line] = prefix + suffix;
		}
		else {
			std::string suffix = (*base)[cursor.line].substr(cursor.column);
			(*base)[cursor.line].erase(cursor.column);
			(*base)[cursor.line] += '\n';
			cursor.line++; cursor.column = 0;
			base->newLine(cursor.line, 1);
			(*base)[cursor.line] = suffix;
		}
		return;
	}
	std::string& cur = (*base)[cursor.line];
	cur.insert(cur.begin()+cursor.column, c);
	moveCursor(0, 1);
}

void TextEditor::undo() {
	base->rollback(1);
	rollbackCursor();
	//reAdjustCursor(true);
}


void TextEditor::copyTo(Cursor endPosition, bool linewise) {
	reAdjustCursor(true);
	copyRegister.clear();
	copyRegisterIsLinewise = linewise;
	Cursor begin = std::min(cursor, endPosition), end = std::max(cursor, endPosition);
	if(linewise) {
		while(begin.line <= end.line) {
			copyRegister += (*constBase)[begin.line];
			begin.line++;
		}
	}
	else {
		while(begin < end) {
			if(begin.line < end.line) {
				copyRegister += (*constBase)[begin.line].substr(begin.column);
				begin.line++; begin.column = 0;
			}
			else {
				copyRegister += (*constBase)[begin.line].substr(begin.column, end.column-begin.column);
				advanceCursor(begin, end.column-begin.column);
			}
		}
	}
}

void TextEditor::pasteAfter() {
	if(copyRegisterIsLinewise) {
		if(cursor.line+1 == (*constBase).numLines()) {
			base->newLine(cursor.line+1, 1);
		}
		moveCursor(1, -1e9);
		insert(copyRegister, false);
		if(cursor.line+1 == (*constBase).numLines()) {
			erase(1, 0);
		}
	}
	else {
		moveCursor(0, 1);
		insert(copyRegister);
	}
}
void TextEditor::pasteBefore() {
	if(copyRegisterIsLinewise) {
		moveCursor(0, -1e9);
		insert(copyRegister, false);
	}
	else {
		insert(copyRegister, false);
	}
}

void TextEditor::deleteTo(Cursor endPosition, bool linewise, bool saveLine) {
	reAdjustCursor(true);
	Cursor begin = std::min(cursor, endPosition), end = std::max(cursor, endPosition);
	if(linewise) {
		setCursor(begin.line, begin.column);
		if(saveLine) {
			eraseLine(end.line-begin.line);
			(*base)[cursor.line] = "\n";
		}
		else {
			eraseLine(end.line-begin.line+1);
		}
	}
	else {
		setCursor(end.line, end.column);
		while(begin < cursor) {
			if(begin.line < cursor.line) {
				erase(cursor.column+1, 0);
			}
			else {
				erase(cursor.column - begin.column, 0);
			}
		}
	}
	reAdjustCursor(true);
}

void TextEditor::join() {
	reAdjustCursor(true);
	if(cursor.line+1 == (*constBase).numLines()) return; // nothing else to join
	moveCursor(1, -1e9);
	Cursor tmp = cursor;
	jumpToNonWhitespace(false);
	deleteTo(tmp, false);
	erase(1, 0);
	insert(' ');
	moveCursor(0, -1);
}