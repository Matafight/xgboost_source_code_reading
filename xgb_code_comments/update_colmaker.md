
```c++

/*
参数： gpair 存储的是各个样本的grad和hess对
p_fmat 训练样本
p_tree 一棵树*/
virtual void Update(const std::vector<bst_gpair>&gpair,DMatrix*p_fmat,RegTree* p_tree)
{
    //初始化数据，包括降采样，包括初始化 变量qexpand_，这个变量里保存的是即将需要分裂的节点的所有样本，各个样本都对应着一个序号，存储的是样本的序号。保存的是节点
 this->InitData(gpair,*p_fmat,*ptree);

 //计算队列qexpand_中所有候选节点的损失和权重,qexpand_存储的是节点，每个节点都对应着一些样本
 //包括了计算统计量，一阶和二阶梯度
 this->InitNewData(qexpend_,gpair,fmat,tree);

 for (int depth = 0; depth < param.max_depth; ++depth) {
     /*
     FindSplit函数的具体介绍见下
     */
        this->FindSplit(depth, qexpand_, gpair, p_fmat, p_tree);
        //更新样本到树节点的映射关系
        this->ResetPosition(qexpand_, p_fmat, *p_tree);
        //更新qexpand_，替换成新分割的节点
        this->UpdateQueueExpand(*p_tree, &qexpand_);
        //计算qexpand_中节点的权重和损失增益,并计算统计量
        this->InitNewNode(qexpand_, gpair, *p_fmat, *p_tree);
        // if nothing left to be expand, break
        if (qexpand_.size() == 0) break;
      }
}
```

FindSplit函数：
```c++
 // find splits at current level, do split per level
    inline void FindSplit(int depth,
                          const std::vector<int> &qexpand,
                          const std::vector<bst_gpair> &gpair,
                          DMatrix *p_fmat,
                          RegTree *p_tree) {
      std::vector<bst_uint> feat_set = feat_index;
      if (param.colsample_bylevel != 1.0f) {
        std::shuffle(feat_set.begin(), feat_set.end(), common::GlobalRandom());
        unsigned n = std::max(static_cast<unsigned>(1),
                              static_cast<unsigned>(param.colsample_bylevel * feat_index.size()));
        CHECK_GT(param.colsample_bylevel, 0U)
            << "colsample_bylevel cannot be zero.";
        feat_set.resize(n);
      }
      dmlc::DataIter<ColBatch>* iter = p_fmat->ColIterator(feat_set);
      while (iter->Next()) {
          /*这个循环是遍历所有feature，UpdateSolution是为了查找当前feature的最优分裂点值，具体实现分为串行和并行两种方式，不过我有个疑问就是这个寻找特征最优分裂点不是应该每个子节点都要单独寻找吗？还是因为本来就是这样做的不过这里没体现出来？因为代表待分裂的子节点的队列是qexpand_,不过没有用到？
        */
        this->UpdateSolution(iter->Value(), gpair, *p_fmat);
      }
      // after this each thread's stemp will get the best candidates, aggregate results
      
      /*进行多线程同步，对于A节点，第一个线程寻找feature b的最优分裂点，第二个线程寻找feature c的最优分裂点，那么这个函数就是将feature b 和c综合起来考虑找到节点A的最优分裂特征及其分裂点*/
      this->SyncBestSolution(qexpand);
      // get the best result, we can synchronize the solution
      /*下面是为需要分割的节点创建左右孩子节点*/
      for (size_t i = 0; i < qexpand.size(); ++i) {
        const int nid = qexpand[i];
        NodeEntry &e = snode[nid];
        // now we know the solution in snode[nid], set split
        if (e.best.loss_chg > rt_eps) {
          p_tree->AddChilds(nid);
          (*p_tree)[nid].set_split(e.best.split_index(), e.best.split_value, e.best.default_left());
          // mark right child as 0, to indicate fresh leaf
          (*p_tree)[(*p_tree)[nid].cleft()].set_leaf(0.0f, 0);
          (*p_tree)[(*p_tree)[nid].cright()].set_leaf(0.0f, 0);
        } else {
          (*p_tree)[nid].set_leaf(e.weight * param.learning_rate);
        }
      }
    }

```


参考：
https://www.zhihu.com/question/41354392
http://mlnote.com/2016/10/29/xgboost-code-review-with-paper/
http://matafight.github.io/2017/03/14/XGBoost-%E7%AE%80%E4%BB%8B/