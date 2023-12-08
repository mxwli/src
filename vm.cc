#include "vm.h"
#include <fstream>
#include <iomanip>
#include <filesystem>

VM::VM(): editor(&base), lastChange([](Model*, int cnt)->void{}, false, 1) {
	isRecording = false; recordingInto = 0;
	quitSignal = false;
}

void VM::notify(Operation* o) {
	TextOperation* to = dynamic_cast<TextOperation*>(o);
	if(to != nullptr) {
		if(to->isChange()) {
			setLastChange(*to);
			lastChangeIsSaved = false;
		}
		if(isRecording) {
			recordings[recordingInto].push_back(*to);
		}
		(*to)(this);
		notifyViewers();
	}
}

void VM::attemptQuit() {
	if(lastChangeIsSaved) {
		quitSignal = true;
	}
	else {
		bottomDisplay = "Error: No write since last change (! to override)";
	}
}
void VM::forceQuit() {
	quitSignal = true;
}

void VM::readFromFile(std::string fileName) {
	this->fileName = fileName;
	if(std::filesystem::exists(fileName)) {
		std::ifstream file(fileName);

		file >> std::noskipws;

		bottomDisplay = "\""+fileName+"\"";
		bottomDisplaySuffix = "";
		base = TextBase();
		editor = TextEditor(&base);
		
		size_t bytes = 0, lines = 0;

		char c;
		while(file >> c) {
			editor.insert(c, false);
			bytes++;
			if(c == '\n') lines++;
		}
		if(c != '\n') {
			bottomDisplay += " [noeol]";
			lines++;
		}
		else {
			editor.erase(1, 0); // erase the last line
		}
		// if the last char isn't a newline, we insert one

		std::stringstream bytelinecnt;
		bytelinecnt << lines << "L " << bytes << "B";

		std::string S; std::getline(bytelinecnt, S);

		bottomDisplay += " " + S;
	}
	else {
		bottomDisplay = fileName + " [New]";
	}
}

void VM::writeToFile(std::string str) {
	fileName = str;
	writeToFile();
}

void VM::writeToFile() {
	if(fileName == "") {
		bottomDisplay = "Error: No file name";
		return;
	}
	bottomDisplay = fileName;
	if(!std::filesystem::exists(fileName)) bottomDisplay += " [New]";
	size_t lines = 0, bytes = 0;
	std::ofstream fout(fileName);
	const TextBase& constBase = base;
	for(size_t i = 0; i < base.numLines(); i++) {
		fout << constBase[i];
		lines++;
		bytes += constBase[i].size();
	}
	std::stringstream bytelinecnt;
	bytelinecnt << lines << "L " << bytes << "B";
	std::string S; std::getline(bytelinecnt, S);
	bottomDisplay += " " + S;
	lastChangeIsSaved = true;
}

void VM::insertFromFile(std::string fileName) {
	if(!std::filesystem::exists(fileName)) {
		bottomDisplay = "Error: " + fileName + " does not exist.";
	}
	else {
		std::ifstream file(fileName);

		if(!file.good()) {
			bottomDisplay = "Error: opening \"" + fileName + "\"";
			return;
		}

		file >> std::noskipws;

		bottomDisplay = "\""+fileName+"\"";
		bottomDisplaySuffix = "";
		
		size_t bytes = 0, lines = 0;

		char c;

		editor.moveCursor(0, 1e9);
		editor.insert('\n', false);
		while(file >> c) {
			editor.insert(c, false);
			bytes++;
			if(c == '\n') lines++;
		}
		if(c != '\n') {
			editor.insert('\n', false);
			bottomDisplay += " [noeol]";
			lines++;
		}
		// if the last char isn't a newline, we insert one

		std::stringstream bytelinecnt;
		bytelinecnt << lines << "L " << bytes << "B";

		std::string S; std::getline(bytelinecnt, S);

		bottomDisplay += " " + S;
	}
}
