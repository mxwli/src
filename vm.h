#ifndef VM_H
#define VM_H

class VM;

#include <string>
#include "mvcabstract.h"
#include "texteditor.h"
#include "textbase.h"
#include "keyboardcontroller.h"
#include "textoperationrecorder.h"

class VM : public Model {
	// bottom text
	std::string bottomDisplay, bottomDisplaySuffix;

	// core components of model
	TextEditor editor;
	TextBase base;
	TextOperationRecorder record;

	// file handling
	std::string fileName;
	bool readonly = false;
	bool permissionDenied = false;
	bool noeol = false;
	bool newFile = true;
	bool lastChangeIsSaved = true;

	friend class KeyboardController; // we want textoperation to have access to the editor

	bool quitSignal = false;

public:
	VM();
	void notify(Operation* o);

	void attemptQuit();
	void forceQuit();

	// file i/o //TODO: move to different class
	void readFromFile(std::string fileName);
	void writeToFile(std::string str);
	void writeToFile(); // uses last read/write
	void insertFromFile(std::string fileName);
	void getFileInfo();

	// to be used by Viewer objects
	std::string getBottomDisplay() const {return bottomDisplay+bottomDisplaySuffix;}
	const TextBase& getBase() const {return base;}
	const TextEditor& getEditor() const {return editor;}
	Cursor& getCursor() {return editor.cursor;}

	// to be used by main()
	bool hasQuitSignal() const {return quitSignal;}
};

#endif
