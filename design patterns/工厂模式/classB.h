#ifndef CLASS_B
#define CLASS_B
#include<iostream>
#include"reflact.h"
#include"reflact_classinfo.h"

class B:public object
{
    public:
        B(){
        std::cout<<"B constructor!"<<std::endl; 
        };
        ~B(){
        std::cout<<"B destorier!"<<std::endl; 
        };
        void printClassName()
        {
        std::cout<<"this is class B!"<<std::endl;
        }
        virtual ClassInfo*GetClassInfo() const 
        {
            return &ms_classinfo;
        };
        static object*CreateObject()
        {
            return new B;
        };
    protected:
        static ClassInfo ms_classinfo;
};

//每一个继承object的类都有一个静态的classinfo成员，需要在类外初始化
//这是什么语法？怎么初始化类中的static变量
ClassInfo B::ms_classinfo("B",B::CreateObject);
#endif
