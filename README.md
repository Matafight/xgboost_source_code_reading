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

上述代码中init()是Parameter模块中的函数，它是怎么“看到”task和eval_train这两个参数的呢？ C++中并没有反射机制，

知识点：
工厂模式;
C++实现反射机制;
宏定义函数;

什么是工厂模式？
工程模式分为简单工程模式，工厂方法模式和抽象工厂模式
1. 简单工厂模式就是在工厂类中做判断，根据输入的字符串返回该类对象，比如输入一个字符串，判断该字符串是a还是b，从而new一个相应的对象返回。这个模式的一个缺点是增加一个新的类就要修改工厂类。
2. 工厂方法模式
不同于简单工厂模式，工厂方法模式定义了一个基类，其他各个需要生产的商品各自继承这个基类，各自成为一个自己的工厂类，这个工厂只生产一种商品。缺点是增加一个新的类就要新继承一个工厂。
3. 抽象工厂模式
抽象工厂模式相当于结合了简单工厂模式和工厂方法模式，简单工厂是所有商品都在一个工厂中，工厂方法是每个商品有各自的工厂，抽象工厂就是同类的商品在同一个工厂。所以抽象工厂模式也是有多个工厂，每个工厂有生产同类的商品。

什么是反射？
简单理解就是只根据类名就可以获取到该类的实例，这个特性在java和c#是原生支持的，但是C++中并不原生支持，不过可以自己实现反射特性。

一个反射器应当满足以下条件：
1. 实现一个需要反射机制类的基类，基类要实现两个方法，一个是register,表示将该类注册到映射中，另一个是createobj方法，返回类对象。
2. 工厂类中需要实现一个映射，即一个字符串对应一个类new的方法。
3. 假设现在需要根据类名生成相应的对象，首先这个类要继承基类，并重写基类中的createObj方法，返回自身实例，

使用宏简化代码：

一个非常好的介绍如何在C++实现反射机制的博客：http://blog.csdn.net/y1196645376/article/details/51455273
http://shuokay.com/2017/05/20/mxnet-parameter/


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