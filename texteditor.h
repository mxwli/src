#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

struct Cursor;
class TextEditor;
class VM;

#include "textbase.h"
#include "textoperation.h"

struct Cursor {
	int line, visLine, column, visColumn, viewBegin, numLinesLastViewed;
	auto friend operator<=>(const Cursor& cur, const Cursor& other) {
		return std::pair<int, int>(cur.line, cur.column)
			<=> std::pair<int, int>(other.line, other.column);
	}
	bool friend operator==(const Cursor& cur, const Cursor& other) {
		return std::pair<int, int>(cur.line, cur.column)
			== std::pair<int, int>(other.line, other.column);
	}
	void updateVisuals() {visLine = line; visColumn = column;}
};

class TextEditor {
	TextBase* base;
	const TextBase* constBase; // for access without modifications
	std::vector<Cursor> cursorHistory;
	std::string copyRegister;
	bool copyRegisterIsLinewise = false;
	Cursor cursor, insertTo;
	bool isInInsertMode = false;
private: // separator here just for helper functions
	bool advanceCursor(Cursor& c, int lr); // returns whether or not we've hit the beginning
	Cursor getNextWord();
	Cursor getPreviousWord();
	std::string getIndentation(int line); // returns string representing indentation
public:
	TextEditor(TextBase* base): base(base), constBase(base), cursor(0,0,0) {
		cursorHistory.push_back(cursor);
	}

	// cursor navigation
	void reAdjustCursor(bool restrictCol);
	void moveCursor(int ud, int lr, bool vis = false);
	void setCursor(int line, int col);
	Cursor& getCursor() {return cursor;}
	const Cursor& getCursor() const {return cursor;}
	void jumpByWord(int words);
	void jumpToNonWhitespace(bool linewise);
	void jumpRight(char c);
	void jumpLeft(char c);
	void jumpC();
	// moves cursor to the next occurence of the special set of patterns
	// defined in %
	bool findNextOccurence(std::string str);
	bool findPrevOccurence(std::string str);
	// returns whether or not an occurence was found
	void saveCursor();
	void rollbackCursor();


	// "insert mode" in the context of texteditor merely means to create
	// a temporary copy to display live edits
	void enterInsertMode();
	void exitInsertMode();
	void insert(const std::string& s, bool autoIndent=true);
	void insert(char c, bool autoIndent=true);
	void erase(size_t left, size_t right);
	void eraseInLine(size_t right);
	void eraseLine(size_t cnt);
	void undo();

	// motion range commands
	// eg. c{motion}, d{motion}, y{motion}, etc.
	void copyTo(Cursor endPosition, bool linewise);
	void pasteAfter();
	void pasteBefore();
	void deleteTo(Cursor endPosition, bool linewise, bool saveLine=false);

	void join();
};

#endif
