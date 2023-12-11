#include "textbase.h"

using TB = TextBase;

TB::TextBase() {
	newSave();
	newLine(0, 1);
}

void TB::newSave() {
	if(history.size() > 0) {
		history.push_back(history.back());
	}
	else {
		history.push_back(version());
	}
}

void TB::rollback(size_t cnt) {
	while(history.size() > 1 && cnt > 0) {
		cnt--;
		history.pop_back();
	}
}

void TB::newLine(size_t idx, size_t cnt) {
	history.back().insert(history.back().begin()+idx, cnt,
		std::make_shared<std::string>("\n"));
}

void TB::eraseLine(size_t idx, size_t cnt) {
	history.back().erase(history.back().begin()+idx, history.back().begin()+idx+cnt);
	if(history.back().size() == 0) {
		newLine(0, 1);
	}
}

void TB::setLineModifiable(size_t idx) {
	if(history.back()[idx].use_count() > 1) {
		history.back()[idx] = std::make_shared<std::string>(*history.back()[idx]);
	}
}

std::string& TB::operator[](size_t idx) {
	setLineModifiable(idx);
	return *history.back()[idx];
}

const std::string& TB::operator[](size_t idx) const {
	return *history.back()[idx];
}

int TB::numLines() const {
	return static_cast<int>(history.back().size());
}
