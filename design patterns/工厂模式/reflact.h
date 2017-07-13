#ifndef REFLACT
#define REFLACT
#include<iostream>
#include<vector>
#include<map>

class ClassInfo;
class object
{
    public:
        object(){
        };
        virtual ~object(){};
        static bool Register(ClassInfo*ci);
        static object* CreateObject(std::string name);
    private:
        std::string name;

};
typedef object* (*ObjectConstructFn)(void);
class ClassInfo
{
    public:
        ClassInfo(const std::string className,ObjectConstructFn ctor):m_classname(className),m_objectConstructor(ctor){
            object::Register(this);
        };
        virtual ~ClassInfo(){};
        object * CreateObject()const{
        //这是对函数指针解引用吗
        return m_objectConstructor ?(*m_objectConstructor)():0;
        };
        bool IsDynamic()const
        {
            return NULL!=m_objectConstructor;
        };
        const std::string GetClassname() const 
        {
            return m_classname;
        };
        ObjectConstructFn GetConstructor()const{return m_objectConstructor;};


    public:
        std::string m_classname;
        ObjectConstructFn m_objectConstructor;
};
static std::map<std::string,ClassInfo*> * classinfomap = NULL;
 bool object::Register(ClassInfo * ci)
{
    if(!classinfomap)
    {
        classinfomap = new std::map<std::string,ClassInfo*>();
    }
    if(ci)
    {
        if(classinfomap->find(ci->m_classname)==classinfomap->end())
        {
            std::cout<<ci->m_classname<<std::endl;
            classinfomap->insert(std::map<std::string,ClassInfo*>::value_type(ci->m_classname,ci));
        }
    }
    return true;
}

  object* object::CreateObject(std::string name)
{
    std::map<std::string,ClassInfo*>::const_iterator iter = classinfomap->find(name);
    if(classinfomap->end()!=iter)
    {
        return iter->second->CreateObject();
    }
    return NULL;
}

//利用宏简化代码
#define DECLARE_CLASS(name)\
    protected:\
        static ClassInfo ms_classinfo;\
    public:\
        virtual ClassInfo *GetClassInfo() const;\
        static object* CreateObject();


#define IMPLEMENT_CLASS_COMMON(name,func)\
    ClassInfo name::ms_classinfo((#name),\
            (ObjectConstructFn) func);\
    ClassInfo *name::GetClassInfo() const\
    {return &name::ms_classinfo;}

#define IMPLEMENT_CLASS(name) \
    IMPLEMENT_CLASS_COMMON(name,name::CreateObject)\
    object*name::CreateObject()\
    {return new name;}

class C:public object
{
    //宏没有什么特别的，只要将宏替换成其相应的代码就行了，很简单
    DECLARE_CLASS(C)
    public:
        C(){std::cout<<"C constructor!"<<std::endl;};
        ~C(){std::cout<<"C destructor!"<<std::endl;};
};
IMPLEMENT_CLASS(C)
#endif




