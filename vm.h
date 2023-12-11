#ifndef VM_H
#define VM_H

class VMInternal;
class VMVisual;
class VM;

#include <string>
#include "mvcabstract.h"
#include "texteditor.h"
#include "textbase.h"
#include "keyboardcontroller.h"
#include "textoperationrecorder.h"

class VMVisual { // interface for viewer
public:
	virtual ~VMVisual() = default;
	virtual std::string getBottomDisplay() const = 0;
	virtual const TextBase& getBase() const = 0;
	virtual Cursor& getCursor() = 0;
};

class VMInternal: public Model { // interface for controller
protected:
	VMInternal(): editor(&base), record(this) {}
	// bottom text
	std::string bottomDisplay, bottomDisplaySuffix;

	// core components of model
	TextEditor editor;
	TextBase base;
	TextOperationRecorder record;

	virtual void readFromFile(std::string fileName) = 0;
	virtual void writeToFile(std::string str) = 0;
	virtual void writeToFile() = 0; // uses last read/write
	virtual void insertFromFile(std::string fileName) = 0;
	virtual void getFileInfo() = 0;

	virtual void attemptQuit() = 0;
	virtual void forceQuit() = 0;

	friend class KeyboardController; // we want textoperation to have access to these things

	public:
	virtual ~VMInternal() = default;
};

class VM : public VMVisual, public VMInternal {
	bool quitSignal = false;

	// file handling
	std::string fileName;
	bool readonly = false;
	bool permissionDenied = false;
	bool noeol = false;
	bool newFile = true;
	bool lastChangeIsSaved = true;

	// file io
	void readFromFile(std::string fileName) override;
	void writeToFile(std::string str) override;
	void writeToFile() override; // uses last read/write
	void insertFromFile(std::string fileName) override;
	void getFileInfo() override;

	void attemptQuit() override;
	void forceQuit() override;
public:
	VM(std::string fileName);
	void notify(Operation* o);

	// to be used by Viewer objects
	std::string getBottomDisplay() const override {return bottomDisplay+bottomDisplaySuffix;}
	const TextBase& getBase() const override {return base;}
	Cursor& getCursor() override {return editor.getCursor();}

	// to be used by main()
	bool hasQuitSignal() const {return quitSignal;}
};

#endif
