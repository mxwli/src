#ifndef NCURSESVIEWER_H
#define NCURSESVIEWER_H

class NCursesViewer;

#include <ncurses.h>
#include "mvcabstract.h"
#include "vm.h"

class NCursesViewer : public View {
	WINDOW* targetWin = nullptr;
	size_t displayRows = 0, displayCols = 0, numTotalLines = 0, idxWidth = 0, numLinesToPrint = 0;
	Cursor* cursor = 0;
public:
	NCursesViewer(WINDOW* win): targetWin(win) {}
	void notify(Model* m) override;
private:
	// helper functions

	std::string intToString(int i) const;
	size_t rowsRequiredForLine(const std::string& str)const ;
	void focusCursor(const TextBase& tb);
	void findNumLinesToPrint(const TextBase& tb);
	void printLine(size_t idx, const std::string& line, size_t atRow);
};

#endif