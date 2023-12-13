#ifndef TEXTOPERATION_H
#define TEXTOPERATION_H

class TextOperation;

#include <functional>
#include "mvcabstract.h"

class TextOperation : public Operation {
	std::function<void(Model*, int count)> func;
	bool change;
	bool recordable;
	int count;
public:
	//TextOperation(): func(nullptr), change(false), count(0) {}
	TextOperation(std::function<void(Model*, int cnt)> func, bool change, int c):
		func(func), change(change), count(c), recordable(true) {}
	TextOperation(std::function<void(Model*, int cnt)> func, bool change, int c, bool rec):
		func(func), change(change), count(c), recordable(rec) {}
	TextOperation(const TextOperation& other):
		func(other.func), change(other.change), count(other.count) {}
	TextOperation(TextOperation&& other):
		func(std::move(other.func)), change(other.change), count(other.count) {}
	TextOperation& operator=(TextOperation other)
		{swap(func, other.func); change = other.change; count = other.count; return *this;}

	void operator()(Model* m) override {func(m, count);}
	void setCount(int c) {count = c;}
	bool isChange() const {return change;}
	bool isRecordable() const {return recordable;}
	int getCount() const {return count;}
};


#endif