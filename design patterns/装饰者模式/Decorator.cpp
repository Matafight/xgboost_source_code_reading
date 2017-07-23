#include<iostream>

using namespace std;

class component
{
public:
	virtual void operation() = 0;
};

class sub_component : public component
{
public:
	void operation(){
		cout << "operation without decoratoration!" << endl;
	}
};

class Decorator : public component
{
public:
	Decorator(component * input) :comp_class(input){};
	void operation()
	{
		if (comp_class != NULL)
		{
			//父类指针可以指向子类
			comp_class->operation();
		}
	}
private:
	component *comp_class;
};

class concreate_Decorator_A :public Decorator
{
public:
	//调用父类的构造函数
	concreate_Decorator(component*input) :Decorator(input){};
	void operation()
	{
		add_Behavior();
		//调用父类地同名函数，使用class::fun_name的方式调用
		Decorator::operation();
	}
	void add_Behavior()
	{
		cout << "this added behavior is concreate_decorator_A" << endl;
	}

};

class concreate_Decorator_B :public Decorator
{
public:
	concreate_Decorator_B(component * input) :Decorator(input){};
	void operation()
	{
		add_Behavior();
		Decorator::operation();
	}
	void add_Behavior()
	{
		cout << "this  added behavior is concreate_decorator_B" << endl;
	}

};