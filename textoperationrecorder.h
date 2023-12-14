#ifndef INPUTRECORDER_H
#define INPUTRECORDER_H

#include "textoperation.h"
#include "mvcabstract.h"

class TextOperationRecorder : public Controller {
	// lastChange & recording handling
	TextOperation lastChange;
	std::vector<TextOperation> recordings[512];
	bool recording;
	char recordingInto;
public:
	TextOperationRecorder(Model* parent): lastChange([](Model*, int cnt)->void{}, false, 1) {
		recording = false;
		recordingInto = 0;
		setModel(parent);
	}
	// lastChange manipulation
	void setLastChange(TextOperation t) {lastChange = std::move(t);}
	void repeatLastChange(size_t cnt) {
		if(cnt > 0) lastChange.setCount(cnt);
		notifyModel(lastChange);
	}

	// recording manipulation
	void beginRecording(char c) {
		recording = true;
		recordingInto = c;
		recordings[c].clear();
	}
	void recordTextOperation(const TextOperation& to) {
		recordings[recordingInto].push_back(to);
	}
	void endRecording() {
		recording = false;
	}
	void playRecording(char c) {
		for(auto& i: recordings[c]) notifyModel(i);
	}
	bool isRecording() {return recording;}
};

#endif
