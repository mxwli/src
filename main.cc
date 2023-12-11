#include <ncurses.h>

#include "ncursesviewer.h"
#include "vm.h"
#include "keyboardcontroller.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	WINDOW* win = nullptr;
	win = initscr();// ncurses initialization
	cbreak(); // get character-at-a-time input
	noecho(); // do not echo typed characters
	keypad(stdscr, TRUE); // enable special characters
	set_tabsize(4); // the only correct size
	ESCDELAY = 0;
	
	// MVC
	VM model;

	std::string fileName = "";
	bool enableEnhancement = false;
	for(int i = 1; i < argc; i++) {
		if(std::string(argv[i]) == "-e") enableEnhancement = true;
		else fileName = std::string(argv[i]);
	}
	if(fileName != "") {
		model.readFromFile(fileName);
	}

	NCursesViewer view(win);
	KeyboardController control(enableEnhancement);
	
	control.setModel(&model);
	model.addView(&view);

	view.notify(&model); // initial notification to trigger draw
	
	while(!model.hasQuitSignal()) {
		control.handleKeystroke(getch());
	}
	
	endwin();
}