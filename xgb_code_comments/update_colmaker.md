
```c++

/*
参数： gpair 存储的是各个样本的grad和hess对
p_fmat 训练样本
p_tree 一棵树*/
virtual void Update(const std::vector<bst_gpair>&gpair,DMatrix*p_fmat,RegTree* p_tree)
{
    //初始化数据，包括降采样，包括初始化 变量qexpand_，这个变量里保存的是即将需要分裂的节点的所有样本，各个样本都对应着一个序号，存储的是样本的序号。
 this->InitData(gpair,*p_fmat,*ptree);

 //计算队列qexpand_中所有候选节点的损失函数和权重
 this->InitNewData(qexpend_,gpair,fmat,tree);

}
```