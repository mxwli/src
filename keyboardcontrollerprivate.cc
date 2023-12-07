#include <ncurses.h>
#include <iostream>
#include "keyboardcontroller.h"
#include "vm.h"

using KC = KeyboardController;

void KC::moveBackByWord() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.jumpByWord(-cnt);
	}, false, countBuffer);
	notifyModel(op);
}
void KC::moveFrontByWord() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.jumpByWord(cnt);
	}, false, countBuffer);
	notifyModel(op);
}

void KC::moveUp() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.moveCursor(-cnt, 0);
	}, false, countBuffer);
	notifyModel(op);
}
void KC::moveDown() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.moveCursor(cnt,0 );
	}, false, countBuffer);
	notifyModel(op);
}

void KC::moveLeft() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.moveCursor(0, -cnt);
	}, false, countBuffer);
	notifyModel(op);
}
void KC::moveRight() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.moveCursor(0, cnt);
	}, false, countBuffer);
	notifyModel(op);
}

void KC::moveBeginNonBlank() {
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.moveCursor(0, -1e9);
		v->editor.jumpToNonWhitespace(false);
	}, false, 1);
	notifyModel(op);
}
void KC::handleZero() {
	if(buffer.size() > 1 && buffer[0] >= '1' && buffer[0] <= '9') {
		// in this case, it's a count input
		return voidFunction();
	}
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.moveCursor(0, -1e9);
	}, false, 1);
	notifyModel(op);
}
void KC::moveEnd() {
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.moveCursor(0, 1e9);
	}, false, 1);
	notifyModel(op);
}

void KC::jumpLeft() {
	parseCount();
	char c = buffer.back();
	buffer.clear();
	lastJump = c; isRight = false;
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		while(cnt-->0) v->editor.jumpLeft(c);
	}, false, countBuffer);
	notifyModel(op);
}
void KC::jumpRight() {
	parseCount();
	char c = buffer.back();
	buffer.clear();
	lastJump = c; isRight = true;
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		while(cnt-->0) v->editor.jumpRight(c);
	}, false, countBuffer);
	notifyModel(op);
}

void KC::repeatLastJump() {
	if(lastJump == 0) { // if there was no last jump
		buffer.clear();
		return;
	}
	parseCount();
	buffer.clear();
	TextOperation op([this](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		while(cnt-->0) {
			if(isRight) {v->editor.jumpRight(lastJump);}
			else {v->editor.jumpLeft(lastJump);}
		}
	}, false, countBuffer);
	notifyModel(op);
}

void KC::nextC() {
	parseCount();
	buffer.clear();
	TextOperation op([this](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.jumpC();
	}, false, countBuffer);
	notifyModel(op);
}

void KC::enterNextSearch() {
	automaton[16].addTransition(KEY_BACKSPACE, &KC::searchNextCharErased, &automaton[0]);
	// if we only have /, then pressing backspace returns to start
	// this is changed when we enter anything (ie. searchCharTyped is triggered)
	parseCount();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay = "/";
	}, false, countBuffer);
	notifyModel(op);
}
void KC::searchNextCharTyped() {
	if(buffer.size() == 1) {
		automaton[16].addTransition(KEY_BACKSPACE, &KC::searchNextCharErased, &automaton[16]);
		// change the transition target
	}
	char c = buffer.back();
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay.push_back(c);
	}, false, countBuffer);
	notifyModel(op);
}
void KC::searchNextCharErased() {
	buffer.pop_back(); // pop the backspace key
	if(buffer.size() == 2) {
		automaton[16].addTransition(KEY_BACKSPACE, &KC::searchNextCharErased, &automaton[0]);
		// now that only one character will be in the buffer after this erase,
		// we change the transition target back to beginning for the next erase
	}
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay.pop_back();
	}, false, countBuffer);
	notifyModel(op);
}
void KC::finishNextSearch() {
	buffer.erase(buffer.begin());
	buffer.pop_back();
	std::string str = "";
	for(auto i: buffer) str.push_back(i);
	TextOperation op([str](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay = "/" + str;
		bool flag = false;
		while(cnt-->0) {
			flag = v->editor.findNextOccurence(str);
		}
		if(!flag) v->bottomDisplay = "Pattern not found: " + str;
	}, false, countBuffer);
	notifyModel(op);
	lastSearch = str;direction = true;
	buffer.clear();
}
void KC::enterPrevSearch() {
	automaton[17].addTransition(KEY_BACKSPACE, &KC::searchPrevCharErased, &automaton[0]);
	// if we only have /, then pressing backspace returns to start
	// this is changed when we enter anything (ie. searchCharTyped is triggered)
	parseCount();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay = "?";
	}, false, countBuffer);
	notifyModel(op);
}
void KC::searchPrevCharTyped() {
	if(buffer.size() == 1) {
		automaton[17].addTransition(KEY_BACKSPACE, &KC::searchPrevCharErased, &automaton[17]);
		// change the transition target
	}
	char c = buffer.back();
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay.push_back(c);
	}, false, countBuffer);
	notifyModel(op);
}
void KC::searchPrevCharErased() {
	buffer.pop_back(); // pop the backspace key
	if(buffer.size() == 2) {
		automaton[17].addTransition(KEY_BACKSPACE, &KC::searchPrevCharErased, &automaton[0]);
		// now that only one character will be in the buffer after this erase,
		// we change the transition target back to beginning for the next erase
	}
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay.pop_back();
	}, false, countBuffer);
	notifyModel(op);
}
void KC::finishPrevSearch() {
	buffer.erase(buffer.begin());
	buffer.pop_back();
	std::string str = "";
	for(auto i: buffer) str.push_back(i);
	TextOperation op([str](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay = "?" + str;
		bool flag = false;
		while(cnt-->0) {
			flag = v->editor.findPrevOccurence(str);
		}
		if(!flag) v->bottomDisplay = "Pattern not found: " + str;
	}, false, countBuffer);
	notifyModel(op);
	lastSearch = str; direction = false;
	buffer.clear();
}


void KC::repeatNextSearch() {
	parseCount();
	TextOperation op([this](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		bool flag = false;
		if(direction) {
			v->bottomDisplay = "/" + lastSearch;
			while(cnt-->0) {
				flag = v->editor.findNextOccurence(lastSearch);
			}
		}
		else {
			v->bottomDisplay = "?" + lastSearch;
			while(cnt-->0) {
				flag = v->editor.findPrevOccurence(lastSearch);
			}
		}
		if(!flag) v->bottomDisplay = "Pattern not found: " + lastSearch;
	}, false, countBuffer);
	notifyModel(op);
	buffer.clear();
}
void KC::repeatPrevSearch() {
	parseCount();
	TextOperation op([this](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		bool flag = false;
		if(direction) {
			v->bottomDisplay = "?" + lastSearch;
			while(cnt-->0) {
				flag = v->editor.findPrevOccurence(lastSearch);
			}
		}
		else {
			v->bottomDisplay = "/" + lastSearch;
			while(cnt-->0) {
				flag = v->editor.findNextOccurence(lastSearch);
			}
		}
		if(!flag) v->bottomDisplay = "Pattern not found: " + lastSearch;
	}, false, countBuffer);
	notifyModel(op);
	buffer.clear();
}


void KC::defaultFunction() {
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->bottomDisplay = "";
	}, false, 1);
	notifyModel(op);
}

void KC::voidFunction() {}
void KC::ignoreKeyFunction() {buffer.pop_back();}

void KC::beginAppend() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->editor.moveCursor(0, 1);
		v->bottomDisplay = "-- INSERT --";
	}, false, 1);
	notifyModel(op);
}
void KC::endAppend() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		while(cnt-->0) {
			v->editor.moveCursor(0, 1);
			v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
			v->editor.moveCursor(0, -1);
		}
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}

void KC::beginEOLAppend() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->editor.moveCursor(0, 1e9);
		v->bottomDisplay = "-- INSERT --";
	}, false, 1);
	notifyModel(op);
}
void KC::endEOLAppend() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		while(cnt-->0) {
			v->editor.moveCursor(0, 1e9);
			v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
			v->editor.moveCursor(0, -1);
		}
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}

void KC::beginInsert() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->bottomDisplay = "-- INSERT --";
	}, false, 1);
	notifyModel(op);
}

void KC::endInsert() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		while(cnt-->0) {
			v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
			v->editor.moveCursor(0, -1);
		}
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}

void KC::beginBOLInsert() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->editor.moveCursor(0, -1e9);
		v->editor.jumpToNonWhitespace(false);
		v->bottomDisplay = "-- INSERT --";
	}, false, 1);
	notifyModel(op);
}
void KC::endBOLInsert() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		while(cnt-->0) {
			v->editor.moveCursor(0, -1e9);
			v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
			v->editor.moveCursor(0, -1);
		}
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}

void KC::beginNLInsert() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->editor.moveCursor(0, 1e9);
		v->editor.insert('\n');
		v->bottomDisplay = "-- INSERT --";
	}, false, 1);
	notifyModel(op);
}
void KC::endNLInsert() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		while(cnt-->0) {
			v->editor.moveCursor(0, 1e9);
			v->editor.insert('\n');
			v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
		}
		v->editor.moveCursor(0, -1);
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}

void KC::beginPNLInsert() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->editor.moveCursor(0, -1e9);
		v->editor.insert('\n');
		v->editor.moveCursor(-1, 0);
		v->bottomDisplay = "-- INSERT --";
	}, false, 1);
	notifyModel(op);
}
void KC::endPNLInsert() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		while(cnt-->0) {
			v->editor.moveCursor(0, -1e9);
			v->editor.insert('\n');
			v->editor.moveCursor(-1, 0);
			v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
		}
		v->editor.moveCursor(0, -1);
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}

void KC::beginDelInsert() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->editor.eraseInLine(cnt);
		v->bottomDisplay = "-- INSERT --";
	}, false, countBuffer);
	notifyModel(op);
}
void KC::endDelInsert() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		v->editor.eraseInLine(cnt);
			v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
		v->editor.moveCursor(0, -1);
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}

void KC::beginDelLineInsert() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->editor.eraseLine(cnt);
		v->editor.moveCursor(0, -1e9);
		v->editor.insert('\n');
		v->editor.moveCursor(-1, 0);
		v->bottomDisplay = "-- INSERT --";
	}, false, countBuffer);
	notifyModel(op);
}
void KC::endDelLineInsert() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		v->editor.eraseLine(cnt);
		v->editor.moveCursor(0, -1e9);
		v->editor.insert('\n');
		v->editor.moveCursor(-1, 0);
		v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
		v->editor.moveCursor(0, -1);
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}


void KC::eraseKeyBefore() {
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.erase(1, 0);
	}, false, 1);
	notifyModel(op);
}
void KC::eraseKeyAfter() {
	// erase the key after
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.erase(0, 1);
	}, false, 1);
	notifyModel(op);
}
void KC::insertKey() {
	int c = buffer.back();
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.insert(c);
	}, false, 1);
	notifyModel(op);
}

void KC::beginReplace() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.enterInsertMode();
		v->bottomDisplay = "-- REPLACE --";
	}, false, 1);
	notifyModel(op);
}
void KC::endReplace() {
	InsertedChanges changes = parseChanges();
	TextOperation op([changes](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		while(cnt-->0) {
			v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
		}
		v->editor.moveCursor(0, -1);
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}
void KC::replaceKey() {
	int c = buffer.back();
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.erase(0, 1);
		v->editor.insert(c);
	}, false, 1);
	notifyModel(op);
}

void KC::replaceKeyWithSave() {
	parseCount();
	int c = buffer.back();
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->base.newSave();
		while(cnt-->0) {
			v->editor.erase(0, 1);
			v->editor.insert(c);
		}
		v->editor.moveCursor(0, -1);
	}, true, countBuffer);
	notifyModel(op);
}

void KC::undo() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		while(cnt-->0)
			v->editor.undo();
	}, false, 1);
	notifyModel(op);
}

void KC::repeatLastChange() {
	if(buffer[0] < '0' || buffer[0] > '9') {
		TextOperation op([](Model* m, int cnt)->void{
			VM* v = dynamic_cast<VM*>(m);
			v->repeatLastChange(0);
		}, false, countBuffer);
		notifyModel(op);
	}
	else {
		parseCount();
		TextOperation op([](Model* m, int cnt)->void{
			VM* v = dynamic_cast<VM*>(m);
			v->repeatLastChange(cnt);
		}, false, countBuffer);
		notifyModel(op);
	}
	buffer.clear();
}


void KC::qPressed() {
	if(isRecording) {
		endRecording();
		automaton[0].addTransition('q', &KC::qPressed, &automaton[12]);
		// play god. sideeffects change the automaton itself
		// is this dangerous? probably. oh well.
	}
	else {
		buffer.clear();
		automaton[0].addTransition('q', &KC::qPressed, &automaton[0]);
	}
}
void KC::exitQ() {
	automaton[0].addTransition('q', &KC::qPressed, &automaton[12]);
}
void KC::beginRecording() {
	char c = buffer.back();
	buffer.clear();
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->beginRecording(c);
		v->bottomDisplaySuffix = " recording @";
		v->bottomDisplaySuffix.push_back(c);
	}, false, 1);
	isRecording = true;
	notifyModel(op);
}
void KC::endRecording() {
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->endRecording();
		v->bottomDisplaySuffix = "";
	}, false, 1);
	isRecording = false;
	notifyModel(op);
}
void KC::playRecording() {
	char c = buffer.back();
	parseCount();
	buffer.clear();
	TextOperation op([c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		while(cnt-->0) {
			v->playRecording(c);
		}
	}, false, countBuffer);
	notifyModel(op);
}

void KC::recognizeAlias() {
	if(buffer[0] == 'S') buffer = {'c','c'};
	if(buffer[0] == 's') buffer = {'c','l'};
	if(buffer[0] == 'X') buffer = {'d','h'};
	if(buffer[0] == 'x') buffer = {'d','l'};
}

void KC::applyMotion(VM* v, char c, int cnt) {
	switch(c) {
		case 'h':v->editor.moveCursor(0, -cnt);
		break;
		case 'j':v->editor.moveCursor(cnt, 0);
		break;
		case 'k':v->editor.moveCursor(-cnt, 0);
		break;
		case 'l':v->editor.moveCursor(0, cnt);
		break;
		case 'b':v->editor.jumpByWord(-cnt);
		break;
		case 'w':v->editor.jumpByWord(cnt);
		break;
		case '^':v->editor.moveCursor(0, -1e9); v->editor.jumpToNonWhitespace(false);
		break;
		case '$':v->editor.moveCursor(0, 1e9);
		break;
		case '0':v->editor.moveCursor(0, 1e9);
		break;
		case ';':
		while(cnt-->0) {
			if(isRight) {v->editor.jumpRight(lastJump);}
			else {v->editor.jumpLeft(lastJump);}
		}
		break;
		case 'n':
		if(direction) {
			while(cnt-->0) {
				v->editor.findNextOccurence(lastSearch);
			}
		} else {
			while(cnt-->0) {
				v->editor.findPrevOccurence(lastSearch);
			}
		}
		break;
		case 'N':
		if(!direction) {
			while(cnt-->0) {
				v->editor.findNextOccurence(lastSearch);
			}
		} else {
			while(cnt-->0) {
				v->editor.findPrevOccurence(lastSearch);
			}
		}
		break;
		case '%':
		v->editor.jumpC();
		break;
	}
}
bool motionIsLinewise(char c) {return c == 'j' || c == 'k' || c == 'c' || c == 'd' || c == 'y';}
void KC::copyMotion() {
	parseCount();
	recognizeAlias();
	char c = buffer.back();
	buffer.clear();
	TextOperation op([this, c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.reAdjustCursor(true);
		Cursor cur = v->getCursor();
		applyMotion(v, c, cnt);
		v->editor.copyTo(cur, motionIsLinewise(c));
		v->editor.setCursor(cur.line, cur.column);
	}, false, countBuffer);
	notifyModel(op);
}
void KC::copyDeleteMotion() {
	parseCount();
	recognizeAlias();
	char c = buffer.back();
	buffer.clear();
	TextOperation op([this, c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->base.newSave();
		v->editor.reAdjustCursor(true);
		Cursor cur = v->getCursor();
		applyMotion(v, c, cnt);
		v->editor.copyTo(cur, motionIsLinewise(c));
		v->editor.deleteTo(cur, motionIsLinewise(c));
	}, true, countBuffer);
	notifyModel(op);
}
void KC::copyDeleteInsertMotion() {
	parseCount();
	recognizeAlias();
	char c = buffer.back();
	TextOperation op([this, c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.reAdjustCursor(true);
		v->editor.enterInsertMode();
		v->bottomDisplay = "-- INSERT --";
		Cursor cur = v->getCursor();
		applyMotion(v, c, cnt);
		v->editor.copyTo(cur, motionIsLinewise(c));
		v->editor.deleteTo(cur, motionIsLinewise(c), true);
	}, true, countBuffer);
	notifyModel(op);
}
void KC::endCopyDeleteInsertMotion() {
	char c = buffer[1];
	buffer.erase(buffer.begin(), buffer.begin()+2);
	InsertedChanges changes = parseChanges();
	TextOperation op([this, changes, c](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->editor.exitInsertMode();
		Cursor cur = v->getCursor();
		applyMotion(v, c, cnt);
		v->editor.copyTo(cur, motionIsLinewise(c));
		v->editor.deleteTo(cur, motionIsLinewise(c), true);
		v->editor.erase(0, changes.rit);
			for(const auto& i: changes.lft) {
				if(i.size() > 0 && i[0] == (char)KEY_BACKSPACE) {
					v->editor.erase(i.size(), 0);
				}
				else {
					v->editor.insert(i);
				}
			}
		v->editor.moveCursor(0, -1);
		v->bottomDisplay = "";
	}, true, countBuffer);
	notifyModel(op);
}
void KC::join() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->base.newSave();
		while(cnt-->0) {
			v->editor.join();
		}
	}, true, countBuffer);
	notifyModel(op);
}

void KC::pasteBefore() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->base.newSave();
		while(cnt-->0) {
			v->editor.pasteBefore();
		}
	}, true, countBuffer);
	notifyModel(op);
}
void KC::pasteAfter() {
	parseCount();
	buffer.clear();
	TextOperation op([](Model* m, int cnt)->void{
		VM* v = dynamic_cast<VM*>(m);
		v->base.newSave();
		while(cnt-->0) {
			v->editor.pasteAfter();
		}
	}, true, countBuffer);
	notifyModel(op);
}



void KC::parseCount() {
	if(buffer.size() == 0 || buffer[0] < '0' || buffer[0] > '9') {
		countBuffer = 1;
		return;
	}
	size_t ret = 0;
	for(size_t i = 0; i < buffer.size(); i++) {
		int c = buffer[i];
		if(c >= '0' && c <= '9') {
			ret = 10*ret + (c - '0');
		}
		else {
			buffer.erase(buffer.begin(), buffer.begin()+i);
			countBuffer = ret;
			return;
		}
	}
	buffer.clear();
	countBuffer = ret;
}

KC::InsertedChanges KC::parseChanges() {
	InsertedChanges ret{{""}, 0};
	bool prvBackspace = false;
	for(int i: buffer) {
		switch(i){
			case KEY_BACKSPACE:
			if(prvBackspace) {
				ret.lft.back().push_back(i);
			}
			else {
				ret.lft.push_back("");
				ret.lft.back().push_back(i);
				prvBackspace = true;
			}
			case KEY_DC:
				ret.rit++;
				break;
			case KEY_ESC:
				break;
			default:
			if(prvBackspace) {
				ret.lft.push_back("");
				ret.lft.back().push_back(i);
				prvBackspace = false;
			}
			else {
				ret.lft.back().push_back(i);
			}
		}
	}
	buffer.clear();
	return ret;
}


