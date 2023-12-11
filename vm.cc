#include "vm.h"
#include <fstream>
#include <iomanip>
#include <filesystem>

VM::VM(std::string fileName) {
	if(fileName != "") readFromFile(fileName);
	quitSignal = false;
}

void VM::notify(Operation* o) {
	TextOperation* to = dynamic_cast<TextOperation*>(o);
	if(to != nullptr) {
		if(to->isChange()) {
			record.setLastChange(*to);
			lastChangeIsSaved = false;
			editor.getCursor().updateVisuals();
		}
		if(record.isRecording() && to->isRecordable()) {
			record.recordTextOperation(*to);
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
		base = TextBase();
		editor = TextEditor(&base);
		
		size_t bytes = 0, lines = 0;

		char c;
		while(file >> c) {
			editor.insert(c, false);
			bytes++;
			if(c == '\n') lines++;
		}
		// if the last char isn't a newline, we insert one
		if(c != '\n') {
			noeol = true;
			lines++;
		}
		else {
			editor.erase(1, 0); // erase the last line
		}

		newFile = false;
		getFileInfo();
	}
	else {
		bottomDisplay = fileName + " [New]";
		newFile = true;
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
	if(!std::filesystem::exists(fileName)) newFile = true;
	size_t lines = 0, bytes = 0;
	std::ofstream fout(fileName);
	const TextBase& constBase = base;
	for(size_t i = 0; i < constBase.numLines(); i++) {
		fout << constBase[i];
		lines++;
		bytes += constBase[i].size();
	}
	lastChangeIsSaved = true;
	getFileInfo();
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

void VM::getFileInfo() {
	size_t lines = 0, bytes = 0;
	if(fileName != "") {
		bottomDisplay = "\"" + fileName + "\" ";
	} else {
		bottomDisplay = "\"[No Name]\" ";
	}
	if(readonly) bottomDisplay += "[readonly]";
	if(permissionDenied) bottomDisplay += "[Permission Denied]";
	if(noeol) bottomDisplay += "[noeol]";
	if(newFile) bottomDisplay += "[New]";
	if(!lastChangeIsSaved) bottomDisplay += "[Modified]";

	const TextBase& constBase = base;
	for(size_t i = 0; i < constBase.numLines(); i++) {
		lines++;
		bytes += constBase[i].size();
	}

	std::stringstream bytelinecnt;
	bytelinecnt << lines << "L " << bytes << "B";
	std::string S; std::getline(bytelinecnt, S);
	bottomDisplay += " " + S;
}