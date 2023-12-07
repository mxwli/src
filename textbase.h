#ifndef TEXTBASE_H
#define TEXTBASE_H

#include <vector>
#include <memory>

class TextBase;

#include "textoperation.h"

class TextBase {
	using line = std::shared_ptr<std::string>;
	using version = std::vector<line>;
	std::vector<version> history;
	// history[0] is the first version, [1] is the second, etc.
public:
	TextBase() {
		newSave();
		newLine(0, 1);
	}
	void newSave() {
		if(history.size() > 0) {
			history.push_back(history.back());
		}
		else {
			history.push_back(version());
		}
	}
	void rollback(size_t cnt) {
		while(history.size() > 1 && cnt > 0) {
			cnt--;
			history.pop_back();
		}
	}
	void newLine(size_t idx, size_t cnt) {
		history.back().insert(history.back().begin()+idx, cnt, std::make_shared<std::string>("\n"));
	}
	void eraseLine(size_t idx, size_t cnt) {
		history.back().erase(history.back().begin()+idx, history.back().begin()+idx+cnt);
		if(history.back().size() == 0) {
			newLine(0, 1);
		}
	}
	void setLineModifiable(size_t idx) {
		if(history.back()[idx].use_count() > 1) {
			history.back()[idx] = std::make_shared<std::string>(*history.back()[idx]);
		}
	}
	std::string& operator[](size_t idx) {
		setLineModifiable(idx);
		return *history.back()[idx];
	}
	const std::string& operator[](size_t idx) const {
		return *history.back()[idx];
	}
	int numLines() const {
		return static_cast<int>(history.back().size());
	}
};

#endif