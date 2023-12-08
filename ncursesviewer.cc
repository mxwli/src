#include "ncursesviewer.h"

#include <algorithm>
#include <iostream>

std::string NCursesViewer::intToString(int i) const {
	if(i == 0) return "0";
	std::string ret = i<0? "-" : "";
	i = std::abs(i);
	int begin = 1000000000;
	while((i/begin)%10 == 0) begin /= 10;
	for(int j = begin; j > 0; j /= 10)
		ret += (i/j)%10+'0';
	return ret;
}

size_t NCursesViewer::rowsRequiredForLine(const std::string& str) const {
	size_t ret = 1, curcol = 0;
	for(char c: str) {
		if(c == '\t') curcol = (curcol+4)/4*4;
		else curcol++;
		if(curcol >= displayCols) {ret++; curcol = 0;}
	}
	return ret;
}

void NCursesViewer::focusCursor(const TextBase& tb) {
	if(cursor->viewBegin < 0) cursor->viewBegin = 0;
	cursor->viewBegin = std::min(cursor->viewBegin, cursor->line);
	size_t rowsAvailable = displayRows - std::min(displayRows, rowsRequiredForLine(tb[cursor->line]));
	size_t setmax = cursor->line;
	while(rowsAvailable > 0 && setmax > 0) {
		size_t trynext = setmax-1;
		if(rowsRequiredForLine(tb[trynext]) > rowsAvailable) break;
		else {
			rowsAvailable -= rowsRequiredForLine(tb[trynext]);
			setmax = trynext;
		}
	}
	cursor->viewBegin = std::max(cursor->viewBegin, (int)setmax);
}

void NCursesViewer::findNumLinesToPrint(const TextBase& tb) {
	size_t rowsAvailable = displayRows - std::min(displayRows, rowsRequiredForLine(tb[cursor->viewBegin]));
	size_t ret = 1, cur = cursor->viewBegin;
	while(rowsAvailable > 0 && cur+1 < numTotalLines) {
		cur++;
		if(rowsRequiredForLine(tb[cur]) > rowsAvailable) break;
		else {
			rowsAvailable -= rowsRequiredForLine(tb[cur]);
			ret++;
		}
	}
	numLinesToPrint = ret;
}

void NCursesViewer::printLine(size_t idx, const std::string& line, size_t atRow) {
	std::string idxStr = intToString(idx);
	move(atRow, 0);
	for(size_t i = 0; i < idxWidth-idxStr.size(); i++) addch(' ');
	for(char c: idxStr) addch(c);
	addch(' ');
	size_t curcol = 0, currow = atRow;
	for(char c: line) {
		if(c == '\t') {
			for(int i = curcol; i < (curcol+4)/4*4; i++) addch(' ');
			curcol = (curcol+4)/4*4;
		}
		else {
			addch(c);
			curcol++;
		}
		if(curcol >= displayCols) {
			currow++;
			curcol = 0;
			move(currow, idxWidth+1);
		}
	}
}

void NCursesViewer::notify(Model* m) {
	VM* v = dynamic_cast<VM*>(m);
	if(v == nullptr) return;
	if(targetWin == nullptr) return;
	cursor = &(v->getCursor());
	const TextBase& tb = v->getBase();
	numTotalLines = tb.numLines();
	idxWidth = std::max((size_t)3, intToString(numTotalLines).size());
	getmaxyx(targetWin, displayRows, displayCols);
	displayRows = std::max(displayRows, (size_t)5);
	displayCols = std::max(displayCols, (size_t)10);
	displayRows--; // we need the last row for status
	displayCols -= idxWidth+1; // we need the first few columns for index
	erase();
	mvaddstr(displayRows, 0, v->getBottomDisplay().c_str());

	// begin drawing
	focusCursor(tb);
	findNumLinesToPrint(tb);

	size_t row = 0;
	size_t printRow = cursor->line, printCol = 0;
	for(int i = 0; i < numLinesToPrint; i++) {
		printLine(i+cursor->viewBegin+1, tb[i+cursor->viewBegin], row);
		if(i+cursor->viewBegin == cursor->line) {
			printRow = row;
			for(int i2 = 0; i2 < std::min(tb[cursor->line].size(), (size_t)cursor->column+1); i2++) {
				if(tb[cursor->line][i2] == '\t') printCol = (printCol+4)/4*4;
				else printCol++;
				if(printCol >= displayCols) {
					printCol = 0;
					printRow++;
				}
			}
		}
		row += rowsRequiredForLine(tb[i+cursor->viewBegin]);
	}
	while(row+1 < displayRows) {
		if(cursor->viewBegin+numLinesToPrint < numTotalLines)
			mvaddch(row, idxWidth, '@');
		else
			mvaddch(row, 0, '~');
		row++;
	}
	move(printRow, printCol+idxWidth);
	refresh();

	cursor->numLinesLastViewed = numLinesToPrint;
}

/*
void NCursesViewer::notify(Model* m)  {
	// setup
	VM* v = dynamic_cast<VM*>(m);
	if(v == nullptr) return;
	Cursor& c = v->getCursor();
	const TextBase& tb = v->getBase();
	size_t numLines = tb.numLines();
	size_t windowWidth, windowHeight;
	getmaxyx(targetWin, windowHeight, windowWidth);
	windowHeight--;
	if(c.viewBegin > c.line) c.viewBegin = c.line;
	if(c.viewBegin + windowHeight <= c.line) c.viewBegin = c.line - windowHeight+1;

	// drawing onto screen
	erase();
	// draw bottom text
	mvaddstr(windowHeight-1, 0, v->getBottomDisplay().c_str());
	// draw lines
	std::string maxLine = intToString(numLines);
	for(int i = c.viewBegin; i < numLines && i-c.viewBegin < windowHeight; i++) {
		std::string lineNumber = intToString(i+1);
		move(i-c.viewBegin, 0);
		for(int j = 0; lineNumber.size()+j+1 < std::max((size_t)4, maxLine.size()); j++) {
			if(j == 0) addch('~');
			else addch(' ');
		}
		for(char c: lineNumber) addch(c);
		addch(' ');
		const std::string& str = tb[i];
			for(char c: str) addch(c);
	}
	move(c.line-c.viewBegin, c.column+std::max((size_t)4, maxLine.size()));
	refresh();
}
*/