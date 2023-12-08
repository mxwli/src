#ifndef VM_H
#define VM_H

class VM;

#include <string>
#include "mvcabstract.h"
#include "texteditor.h"
#include "textbase.h"
#include "keyboardcontroller.h"

class VM : public Model {
	std::string bottomDisplay, bottomDisplaySuffix;
	TextEditor editor;
	TextBase base;

	std::string fileName;
	bool lastChangeIsSaved = true;

	TextOperation lastChange;
	std::vector<TextOperation> recordings[512];
	bool isRecording;
	char recordingInto;

	friend class KeyboardController; // we want textoperation to have access to the editor

	bool quitSignal = false;

public:
	VM();
	void notify(Operation* o);

	void attemptQuit();
	void forceQuit();

	// file i/o
	void readFromFile(std::string fileName);
	void writeToFile(std::string str);
	void writeToFile(); // uses last read/write
	void insertFromFile(std::string fileName);

	// editing & bottomDisplay
	std::string getBottomDisplay() const {return bottomDisplay+bottomDisplaySuffix;}
	const TextBase& getBase() const {return base;}
	const TextEditor& getEditor() const {return editor;}
	Cursor& getCursor() {return editor.cursor;}

	// lastChange manipulation
	void setLastChange(TextOperation t) {lastChange = std::move(t);}
	void repeatLastChange(size_t cnt) {
		if(cnt > 0) lastChange.setCount(cnt);
		lastChange(this);
		notifyViewers();
	}

	// recording manipulation
	void beginRecording(char c) {
		isRecording = true;
		recordingInto = c;
		recordings[c].clear();
	}
	void endRecording() {
		isRecording = false;
	}
	void playRecording(char c) {
		for(auto& i: recordings[c]) notify(&i);
	}

	// to be used by main()
	bool hasQuitSignal() const {return quitSignal;}
};

#endif
