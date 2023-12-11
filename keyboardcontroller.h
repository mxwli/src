#ifndef KEYBOARDCONT_H
#define KEYBOARDCONT_H

#define KEY_ESC 27
#define CTRLB 2
#define CTRLD 4
#define CTRLF 6
#define CTRLG 7
#define CTRLU 21

class AutomatonNode;
class KeyboardController;

#include <functional>
#include <map>
#include <iostream>
#include <queue>
#include "mvcabstract.h"
#include "textoperation.h"
#include "texteditor.h"
#include "ncurses.h"

class AutomatonNode {
	struct AutomatonEdge {
		std::function<void(KeyboardController*)> sideEffect;
		AutomatonNode* target; // not ownership
		AutomatonEdge(): target(nullptr) {}
		AutomatonEdge(std::function<void(KeyboardController*)> se, AutomatonNode* t): sideEffect(se), target(t) {}
		AutomatonEdge(const AutomatonEdge& other): sideEffect(other.sideEffect), target(other.target) {}
		AutomatonEdge(AutomatonEdge&& other):  sideEffect(std::move(other.sideEffect)), target(other.target) {}
		AutomatonEdge& operator=(AutomatonEdge other) {sideEffect = std::move(other.sideEffect); target = other.target; return *this;}
	};
	std::map<int, AutomatonEdge> transitions;
	AutomatonEdge defaultTransition;
public:
	AutomatonNode() {}
	AutomatonNode(const AutomatonNode& other): transitions(other.transitions), defaultTransition(other.defaultTransition) {}
	AutomatonNode(AutomatonNode&& other): transitions(std::move(other.transitions)), defaultTransition(std::move(other.defaultTransition)) {}
	AutomatonNode& operator=(AutomatonNode other) {std::swap(transitions, other.transitions); std::swap(defaultTransition, other.defaultTransition); return *this;}
	AutomatonNode(AutomatonEdge def): defaultTransition(def) {}
	void addTransition(int trans, std::function<void(KeyboardController*)> se, AutomatonNode* t) {transitions[trans] = AutomatonEdge(se, t);}
	void setDefault(std::function<void(KeyboardController*)> se, AutomatonNode* t) {defaultTransition = AutomatonEdge(se, t);}
	AutomatonNode* transitionTo(KeyboardController* cont, int trans) {
		if(transitions.count(trans) == 0) {
			auto c = defaultTransition.target;
			defaultTransition.sideEffect(cont);
			return c;
		}
		else {
			auto c = transitions[trans].target;
			transitions[trans].sideEffect(cont);
			return c;
		}
	}
};

// this is just included to make sure no funny business with weird special keys occurs
class ValidKeyChecker {
	bool validArr[512];
	bool validInsert[512];
	public:
	ValidKeyChecker() {
		validArr[KEY_BACKSPACE] = validArr[KEY_DC] = validArr[KEY_ESC] = 1;
		for(int i = 32; i <= 126; i++) validArr[i] = validInsert[i] = 1;
		validArr['\t'] = validArr['\n'] = 1;
		validInsert['\t'] = validInsert['\n'] = 1;
		validArr[CTRLB] = validArr[CTRLD] = validArr[CTRLF] = validArr[CTRLG] = validArr[CTRLU] = 1;
	}
	bool isValid(int c) const {return c < 512 && c >= 0 && validArr[c];}
	bool isValidInsert(int c) const {return c < 512 && c >= 0 && validInsert[c];}
};

class KeyboardController : public Controller {
	std::vector<AutomatonNode> automaton;
	AutomatonNode* current;
	std::vector<int> buffer; // buffer for characters
	size_t countBuffer; // buffer for count
	// scans the buffer for a count and return the count value
	// additionally removes the count from the buffer
	bool isRecording = false;
	ValidKeyChecker validator;
public:
	KeyboardController(bool enableEnhancement = true);
	void handleKeystroke(int keystroke) {
		if(!validator.isValid(keystroke)) return;
		buffer.push_back(keystroke);
		current = current->transitionTo(this, keystroke);
	}

private:
	
	// store cursor position prior to entering every insert
	Cursor preInsertPosition;


	// cursor movement functions
	void moveBackByWord();
	void moveFrontByWord();

	void moveUp();
	void moveDown();

	void moveLeft();
	void moveRight();

	void moveBeginNonBlank();
	void handleZero(); // zero is a special one
	void moveEnd();

	char lastJump = 0; bool isRight = false;
	void jumpLeft();
	void jumpRight();
	void repeatLastJump();

	// moves cursor to the next occurence of the special set of patterns
	void nextC();
	// basically %

	// search operations
	std::string lastSearch = ""; bool direction = 0;
	void enterNextSearch();
	void searchNextCharTyped();
	void searchNextCharErased();
	void finishNextSearch();
	void enterPrevSearch();
	void searchPrevCharTyped();
	void searchPrevCharErased();
	void finishPrevSearch();
	void repeatNextSearch();
	void repeatPrevSearch();


	// common operations
	void defaultFunction();
	void voidFunction();
	void ignoreKeyFunction();

	// operations used to modify text
	void beginAppend();
	void endAppend();

	void beginEOLAppend();
	void endEOLAppend();

	void beginInsert();
	void endInsert();

	void beginBOLInsert();
	void endBOLInsert();

	void beginNLInsert();
	void endNLInsert();

	void beginPNLInsert();
	void endPNLInsert();

	void beginDelInsert();
	void endDelInsert();

	void beginDelLineInsert();
	void endDelLineInsert();

	void eraseKeyBefore();
	void eraseKeyAfter();
	void insertKey();
	void deleteLine();

	void beginReplace();
	void endReplace();
	void replaceKey();
	void replaceKeyWithSave();

	void undo();
	void repeatLastChange();

	void qPressed();
	void exitQ();
	void beginRecording();
	void endRecording();
	void playRecording();

	// range text modification operations (eg. cc/dd/yy)
	void applyMotion(VM* v, char c, int cnt);
	// to only be used by the range modification functions
	void recognizeAlias();
	void copyMotion();
	void copyDeleteMotion();
	void copyDeleteInsertMotion();
	void endCopyDeleteInsertMotion(); // im very good at naming

	void join();

	void pasteBefore();
	void pasteAfter();

	void beginColonCommand();
	void appendColonCommand();
	void eraseColonCommand();
	void finishColonCommand();

	// window scrolling commands
	void scrollUp();
	void scrollDown();
	void scrollHalfUp();
	void scrollHalfDown();
	void getFileInfo();

	// helper functions
	void parseCount();
	struct InsertedChanges{
		std::vector<std::string> lft;
		size_t rit;
	};
	InsertedChanges parseChanges();
};

#endif
