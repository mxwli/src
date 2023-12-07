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
	if(argc > 1) {
		model.readFromFile(argv[1]);
	}
	

	NCursesViewer view(win);
	KeyboardController control;
	
	control.setModel(&model);
	model.addView(&view);

	view.notify(&model); // initial notification to trigger draw
	
	while(!model.hasQuitSignal()) {
		control.handleKeystroke(getch());
	}
	
	endwin();
}