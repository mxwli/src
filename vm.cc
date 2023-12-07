#include "vm.h"
#include <fstream>
#include <iomanip>

VM::VM(): editor(&base), lastChange([](Model*, int cnt)->void{}, false, 1) {
	isRecording = false; recordingInto = 0;
	quitSignal = false;
}

void VM::notify(Operation* o) {
	TextOperation* to = dynamic_cast<TextOperation*>(o);
	if(to != nullptr) {
		if(to->isChange()) {
			setLastChange(*to);
		}
		if(isRecording) {
			recordings[recordingInto].push_back(*to);
		}
		(*to)(this);
		notifyViewers();
	}
}

void VM::readFromFile(std::string fileName) {
	std::ifstream file(fileName);
	file >> std::noskipws;

	bottomDisplay = "\""+fileName+"\"";
	bottomDisplaySuffix = "";
	base = TextBase();
	editor = TextEditor(&base);

	char c;
	while(file >> c) {
		editor.insert(c);
	}
	if(c != '\n') editor.insert('\n');
	// if the last char isn't a newline, we insert one
}
