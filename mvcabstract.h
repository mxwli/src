#ifndef MVCABSTRACT_H
#define MVCABSTRACT_H
#include <vector>

/*
This header file declares the three interfaces that make up the MVC model
1. Model
2. Viewer
3. Controller
4. Operation (also known as Action)
*/

class Model;
class View;
class Controller;

class Operation {
public:
	virtual void operator()(Model* m) = 0;
};

class View {
public:
	virtual void notify(Model* m) = 0;
};

class Model {
	std::vector<View*> views;
public:
	void addView(View* v) {
		views.push_back(v);
	}
	virtual void notify(Operation* o) = 0;
	void notifyViewers() {
		for(auto i: views) i->notify(this);
	}
};

class Controller {
	Model* model;
public:
	Controller(): model(nullptr) {}
	void setModel(Model* m) {
		model = m;
	}
	void notifyModel(Operation& o) {
		if(model != nullptr) model->notify(&o);
	}
};

#endif