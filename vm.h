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

	TextOperation lastChange;
	std::vector<TextOperation> recordings[512];
	bool isRecording;
	char recordingInto;

	friend class KeyboardController; // we want textoperation to have access to the editor

	bool quitSignal;

public:
	VM();
	void notify(Operation* o);
	void readFromFile(std::string fileName);

	std::string getBottomDisplay() const {return bottomDisplay+bottomDisplaySuffix;}
	const TextBase& getBase() const {return base;}
	const TextEditor& getEditor() const {return editor;}
	Cursor& getCursor() {return editor.cursor;}
	
	void setLastChange(TextOperation t) {lastChange = std::move(t);}
	void repeatLastChange(size_t cnt) {
		if(cnt > 0) lastChange.setCount(cnt);
		lastChange(this);
		notifyViewers();
	}

	void beginRecording(char c) {
		isRecording = true;
		recordingInto = c;
	}
	void endRecording() {
		isRecording = false;
	}
	void playRecording(char c) {
		for(auto& i: recordings[c]) notify(&i);
	}

	bool hasQuitSignal() const {return quitSignal;}
};

#endif
