#include "keyboardcontroller.h"
#include "textoperation.h"
#include "vm.h"
#include <ncurses.h>

using KC = KeyboardController;

KC::KeyboardController() {
	/*
	the plan:
	nearly every state that transitions back to start clears the buffer.
	there are a few exceptions.
	the way counts are considered is that they occupy no special state.
	the input is simply not cleared from the buffer
	All commands that are exclusively a single input do not have their own states
	*/


	// beginning state
	automaton.resize(23);
	AutomatonNode& start = automaton[0];
	current = &start;
	// everybody's default goes right back to the beginning
	for(auto& i: automaton) i.setDefault(&KC::defaultFunction, &start);

	// add count tracking transitions
	for(char i = '1'; i <= '9'; i++) {
		start.addTransition(i, &KC::voidFunction, &start);
	}

	start.addTransition('b', &KC::moveBackByWord, &start);
	start.addTransition('w', &KC::moveFrontByWord, &start);
	
	start.addTransition('h', &KC::moveLeft, &start);
	start.addTransition('j', &KC::moveDown, &start);
	start.addTransition('k', &KC::moveUp, &start);
	start.addTransition('l', &KC::moveRight, &start);

	start.addTransition('0', &KC::handleZero, &start);
	start.addTransition('$', &KC::moveEnd, &start);
	start.addTransition('^', &KC::moveBeginNonBlank, &start);

	start.addTransition('%', &KC::nextC, &start);

	start.addTransition('u', &KC::undo, &start);
	start.addTransition('.', &KC::repeatLastChange, &start);

	start.addTransition('a', &KC::beginAppend, &automaton[2]);
	automaton[2].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[2]);
	automaton[2].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[2]);
	automaton[2].setDefault(&KC::insertKey, &automaton[2]);
	automaton[2].addTransition(KEY_ESC, &KC::endAppend, &start);

	start.addTransition('A', &KC::beginEOLAppend, &automaton[3]);
	automaton[3].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[3]);
	automaton[3].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[3]);
	automaton[3].setDefault(&KC::insertKey, &automaton[3]);
	automaton[3].addTransition(KEY_ESC, &KC::endEOLAppend, &start);

	start.addTransition('i', &KC::beginInsert, &automaton[4]);
	automaton[4].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[4]);
	automaton[4].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[4]);
	automaton[4].setDefault(&KC::insertKey, &automaton[4]);
	automaton[4].addTransition(KEY_ESC, &KC::endInsert, &start);

	start.addTransition('I', &KC::beginBOLInsert, &automaton[5]);
	automaton[5].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[5]);
	automaton[5].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[5]);
	automaton[5].setDefault(&KC::insertKey, &automaton[5]);
	automaton[5].addTransition(KEY_ESC, &KC::endBOLInsert, &start);

	start.addTransition('o', &KC::beginNLInsert, &automaton[6]);
	automaton[6].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[6]);
	automaton[6].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[6]);
	automaton[6].setDefault(&KC::insertKey, &automaton[6]);
	automaton[6].addTransition(KEY_ESC, &KC::endNLInsert, &start);

	start.addTransition('O', &KC::beginPNLInsert, &automaton[7]);
	automaton[7].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[7]);
	automaton[7].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[7]);
	automaton[7].setDefault(&KC::insertKey, &automaton[7]);
	automaton[7].addTransition(KEY_ESC, &KC::endPNLInsert, &start);

	start.addTransition('s', &KC::beginDelInsert, &automaton[8]);
	automaton[8].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[8]);
	automaton[8].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[8]);
	automaton[8].setDefault(&KC::insertKey, &automaton[8]);
	automaton[8].addTransition(KEY_ESC, &KC::endDelInsert, &start);

	start.addTransition('S', &KC::beginDelLineInsert, &automaton[9]);
	automaton[9].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[9]);
	automaton[9].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[9]);
	automaton[9].setDefault(&KC::insertKey, &automaton[9]);
	automaton[9].addTransition(KEY_ESC, &KC::endDelLineInsert, &start);

	start.addTransition('R', &KC::beginReplace, &automaton[10]);
	automaton[10].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[10]);
	automaton[10].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[10]);
	automaton[10].setDefault(&KC::replaceKey, &automaton[10]);
	automaton[10].addTransition(KEY_ESC, &KC::endReplace, &start);

	start.addTransition('r', &KC::voidFunction, &automaton[11]);
	automaton[11].setDefault(&KC::replaceKeyWithSave, &start);
	automaton[11].addTransition(KEY_BACKSPACE, &KC::defaultFunction, &start);
	automaton[11].addTransition(KEY_DC, &KC::defaultFunction, &start);
	automaton[11].addTransition(KEY_ESC, &KC::defaultFunction, &start);

	start.addTransition('q', &KC::qPressed, &automaton[12]);
	for(char c = 'a'; c <= 'z'; c++)
		automaton[12].addTransition(c, &KC::beginRecording, &start);
	for(char c = 'A'; c <= 'Z'; c++)
		automaton[12].addTransition(c, &KC::beginRecording, &start);
	for(char c = '0'; c <= '9'; c++)
		automaton[12].addTransition(c, &KC::beginRecording, &start);
	automaton[12].addTransition('\"', &KC::beginRecording, &start);
	automaton[12].setDefault(&KC::exitQ, &start);

	start.addTransition('@', &KC::voidFunction, &automaton[13]);
	for(char c = 'a'; c <= 'z'; c++)
		automaton[13].addTransition(c, &KC::playRecording, &start);
	for(char c = 'A'; c <= 'Z'; c++)
		automaton[13].addTransition(c, &KC::playRecording, &start);
	for(char c = '0'; c <= '9'; c++)
		automaton[13].addTransition(c, &KC::playRecording, &start);
	automaton[13].addTransition('\"', &KC::playRecording, &start);

	start.addTransition('f', &KC::voidFunction, &automaton[14]);
	automaton[14].setDefault(&KC::jumpRight, &start);
	automaton[14].addTransition(KEY_BACKSPACE, &KC::defaultFunction, &start);
	automaton[14].addTransition(KEY_DC, &KC::defaultFunction, &start);
	automaton[14].addTransition(KEY_ESC, &KC::defaultFunction, &start);

	start.addTransition('F', &KC::voidFunction, &automaton[15]);
	automaton[15].setDefault(&KC::jumpLeft, &start);
	automaton[15].addTransition(KEY_BACKSPACE, &KC::defaultFunction, &start);
	automaton[15].addTransition(KEY_DC, &KC::defaultFunction, &start);
	automaton[15].addTransition(KEY_ESC, &KC::defaultFunction, &start);

	start.addTransition(';', &KC::repeatLastJump, &start);

	start.addTransition('/', &KC::enterNextSearch, &automaton[16]);
	automaton[16].setDefault(&KC::searchNextCharTyped, &automaton[16]);
	automaton[16].addTransition(KEY_ESC, &KC::defaultFunction, &start);
	automaton[16].addTransition(KEY_BACKSPACE, &KC::searchPrevCharErased, &start);
	automaton[16].addTransition(KEY_DC, &KC::ignoreKeyFunction, &start);
	automaton[16].addTransition('\n', &KC::finishNextSearch, &start);

	start.addTransition('?', &KC::enterPrevSearch, &automaton[17]);
	automaton[17].setDefault(&KC::searchPrevCharTyped, &automaton[17]);
	automaton[17].addTransition(KEY_ESC, &KC::defaultFunction, &start);
	automaton[17].addTransition(KEY_BACKSPACE, &KC::searchPrevCharErased, &start);
	automaton[17].addTransition(KEY_DC, &KC::ignoreKeyFunction, &start);
	automaton[17].addTransition('\n', &KC::finishPrevSearch, &start);

	start.addTransition('n', &KC::repeatNextSearch, &start);
	start.addTransition('N', &KC::repeatPrevSearch, &start);
	
	std::string motionChars = "hjklbw^$0;nN%";

	start.addTransition('y', &KC::voidFunction, &automaton[18]);
	automaton[18].addTransition('y', &KC::copyMotion, &start);
	for(auto i: motionChars)
		automaton[18].addTransition(i, &KC::copyMotion, &start);
	
	start.addTransition('p', &KC::pasteAfter, &start);
	start.addTransition('P', &KC::pasteBefore, &start);

	start.addTransition('d', &KC::voidFunction, &automaton[19]);
	automaton[19].addTransition('d', &KC::copyDeleteMotion, &start);
	for(auto i: motionChars)
		automaton[19].addTransition(i, &KC::copyDeleteMotion, &start);

	start.addTransition('c', &KC::voidFunction, &automaton[20]);
	automaton[20].addTransition('c', &KC::copyDeleteInsertMotion, &automaton[21]);
	for(auto i: motionChars)
		automaton[20].addTransition(i, &KC::copyDeleteInsertMotion, &automaton[21]);
	
	automaton[21].setDefault(&KC::insertKey, &automaton[21]);
	automaton[21].addTransition(KEY_BACKSPACE, &KC::eraseKeyBefore, &automaton[21]);
	automaton[21].addTransition(KEY_DC, &KC::eraseKeyAfter, &automaton[21]);
	automaton[21].addTransition(KEY_ESC, &KC::endCopyDeleteInsertMotion, &start);
	
	// just aliases. replace then with what theyre aliasing in KC::recognizeAlias
	start.addTransition('s', &KC::copyDeleteInsertMotion, &automaton[21]);
	start.addTransition('S', &KC::copyDeleteInsertMotion, &automaton[21]);
	start.addTransition('x', &KC::copyDeleteMotion, &start);
	start.addTransition('X', &KC::copyDeleteMotion, &start);

	start.addTransition('J', &KC::join, &start);

	start.addTransition(2, [](KC*)->void{ // ctrl+B
		
	}, &start);

	start.addTransition(4, [](KC*)->void{ // ctrl+D
		
	}, &start);

	start.addTransition(6, [](KC*)->void{ // ctrl+F
		
	}, &start);

	start.addTransition(7, [](KC*)->void{ // ctrl+G
		
	}, &start);

	start.addTransition(21, [](KC*)->void{ // ctrl+G
		
	}, &start);
}

