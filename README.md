# xgboost_source_code_reading

./include/xgboost/文件夹下的文件均为需要继承的基类

主函数 Cli_main.cc
# 怎么传入命令行参数
xgboost的传参部分利用了dmlc的Parameter模块
dmlc::Parameter 模块的路径为./dmlc-core/include/dmlc/parameter.h

```c++
struct CLIParam:public dmlc::Parameter<CLIparam>
{
    int task;
    int eval_train;
    DMLC_DECLARE_PARAMETER(CLIParam) {
    // NOTE: declare everything except eval_data_paths.
    DMLC_DECLARE_FIELD(task).set_default(kTrain)
        .add_enum("train", kTrain)
        .add_enum("dump", kDumpModel)
        .add_enum("pred", kPredict)
        .describe("Task to be performed by the CLI program.");

    DMLC_DECLARE_FIELD(eval_train).set_default(false)
        .describe("Whether evaluate on training data during training.");
   
  }
  DMLC_REGISTER_PARAMETER(CLIParam);

}
int main(int argc,char*argv[])
{
    CLIParam param;
     std::vector<std::pair<std::string, std::string> > param_data = {
    {"task", "0"},{'eval_train','1'}
    };
    param.init(param_data)

}
```

上述代码中init()是Parameter模块中的函数，它是怎么“看到”task和eval_train这两个参数的呢？ 

知识点：
工厂模式;
C++实现反射机制;
宏定义函数;




# 读取数据类
数据存储在DMatric类中,基类路径是./include/xgboost/data.h，继承该基类的各个子类路径是./src/data/
1. 首先要load输入， DMatrix的load方法定义在 ./src/data/data.cc中


# 学习类
learner.h，所有学习器的基类，位于./include/xgboost/
learner.h继承了rabit中的Serializable类。
[rabit](https://github.com/dmlc/rabit/tree/a764d45cfb438cc9f15cf47ce586c02ff2c65d0f)是一个可以实现分布式机器学习算法的接口。

learner.cc 继承了learner.h，并实现了默认的学习器


这个代码大量使用了工厂模式和反射机制。。
learner.cc中的 LearnerImpl继承了 Learner
```c++
class LearnerImpl : public Learner
```
部分类成员为
```c++
  std::unique_ptr<ObjFunction> obj_;
  /*! \brief The gradient booster used by the model*/
  std::unique_ptr<GradientBooster> gbm_;
  /*! \brief The evaluation metrics used to evaluate the model. */
  std::vector<std::unique_ptr<Metric> > metrics_;
```
注意这里的ObjFunction 和 GradientBooster类并不是固定的，需要根据传入的参数选择相应的目标函数和booster分类器or回归器，所以这是一种典型的工厂模式。
我们去看类ObjFunction的定义：
```c++
class ObjFunction {
      /*!
   * \brief Create an objective function according to name.
   * \param name Name of the objective.
   */
  static ObjFunction* Create(const std::string& name);
}

```
而在ObjFunction.cc中实现了Create方法，这就是典型的反射机制,ObjFunction实现了将名字注册到工厂的映射中(通过DMLC_REGISTRY宏函数)，并且定义了一个返回自身对象的接口。其他具体的目标函数需要继承ObjFunction类，并实现自己的Create函数，
```c++
namespace dmlc {
DMLC_REGISTRY_ENABLE(::xgboost::ObjFunctionReg);
}  // namespace dmlc

namespace xgboost {
// implement factory functions
ObjFunction* ObjFunction::Create(const std::string& name) {
  auto *e = ::dmlc::Registry< ::xgboost::ObjFunctionReg>::Get()->Find(name);
  if (e == nullptr) {
    for (const auto& entry : ::dmlc::Registry< ::xgboost::ObjFunctionReg>::List()) {
      LOG(INFO) << "Objective candidate: " << entry->name;
    }
    LOG(FATAL) << "Unknown objective function " << name;
  }
  return (e->body)();
}
}  // namespace xgboost

namespace xgboost {
namespace obj {
// List of files that will be force linked in static links.
DMLC_REGISTRY_LINK_TAG(regression_obj);
DMLC_REGISTRY_LINK_TAG(multiclass_obj);
DMLC_REGISTRY_LINK_TAG(rank_obj);
}  // namespace obj
}  // namespace xgboost

```
使用反射机制，给定一个类名就可以直接返回一个该类的实例。