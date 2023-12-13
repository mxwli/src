#include <ncurses.h>

#include "ncursesviewer.h"
#include "vm.h"
#include "keyboardcontroller.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	
	std::string fileName = "";
	bool enableEnhancement = true;
	for(int i = 1; i < argc; i++) {
		if(std::string(argv[i]) == "-e") enableEnhancement = false;
		else fileName = std::string(argv[i]);
	}

	// MVC
	VM model(fileName);
	NCursesViewer view;
	KeyboardController control(enableEnhancement);
	
	control.setModel(&model);
	model.addView(&view);

	view.notify(&model); // initial notification to trigger draw
	
	while(!model.hasQuitSignal()) {
		control.handleKeystroke(getch());
	}
}
