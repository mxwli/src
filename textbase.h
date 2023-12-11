#ifndef TEXTBASE_H
#define TEXTBASE_H

#include <vector>
#include <memory>
#include <iostream>

class TextBase;

#include "textoperation.h"

class TextBase {
	using line = std::shared_ptr<std::string>;
	using version = std::vector<line>;
	std::vector<version> history;
	// history[0] is the first version, [1] is the second, etc.
	void setLineModifiable(size_t idx);
public:
	TextBase();
	void newSave();
	void rollback(size_t cnt);
	void newLine(size_t idx, size_t cnt);
	void eraseLine(size_t idx, size_t cnt);
	std::string& operator[](size_t idx);
	const std::string& operator[](size_t idx) const;
	int numLines() const;
};

#endif