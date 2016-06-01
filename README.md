# IEH

by 付聪 11521012 of ZJU CS

这是翻译论文“Fast and Accurate Hashing Via Iterative Nearest Neighbors Expansion”
以及相关的实现工程
1.
其中文件夹paper_translate下包含文献翻译的latex文档相关材料，和生成的pdf。文件csmath.tex是latex文档，csmath.pdf是对应的翻译文稿。
该latex文件可以直接被编译

2.
其余的文档包括IEH的实现，由于IEH是一个算法框架，因此我实现了其中的一种：IEH－LSH，是一种最近邻检索算法

3.
matlab文件是用来进行论文中所说的哈希函数训练和训练数据转换。

其中LSH_learn.m是LSH训练函数，输入是对应格式的训练数据点集合，遗迹所需哈希编码长度，输出是训练数据的二进制码和哈希函数
LSH_compress.m是LSH的编码函数，可以将测试数据转换为二进制码
LSHrun.m是主程序，可以直接运行。但首先要调整脚本中的参数，包括：输入数据文件的路径，和对应输出文件的路径。
运行结束后，会产生特定格式的LSH函数组和训练数据二进制码的txt文件。

4.上述程序的结果会作为在线测试部分的输入。
在线测试程序工程是用c++写的代码，包括distance.h和IEHtest.cpp。
Makefile可以用来编译这个工程，在Linux或mac系统下执行make命令即可。

该工程运行需要几个参数：
1）上一部生成的哈希函数的文件路径
2）上一步生成的训练数据二进制码文件路径
3）训练数据集文件路径
4）测试数据集文件路径
5）根据训练数据集生成的精确k－NN最近邻图文件路径
6）扩展点数目（对应论文中参数k，做10近邻检索使用50-1000范围内的数值即可，越大精度越高，时间越慢）
7）迭代次数上限
8）想要获得的测试点的最近邻个数
9）测试点最近邻查询结果保存路径（输出）

运行举例：
./IEHtest LSHfunction.txt LSHtrainCode.txt sift/sift_base.fvecs sift/sift_query.fvecs sift/sift_bf.index 100 10 10 LSH.res

数据集：
工程在SIFT－1M和GIST－1M公开数据集上进行了测试，由于数据集太大，在这里提供下载链接：
http://corpus-texmex.irisa.fr/

其中fvecs_read.m和ivecs_read.m可以在matlab中对该数据集进行读写，其也源自上述网站，包含在我的工程文件里。
另外，由于离线构建最近邻图十分耗时，我也提供了我蛮力构建sift和gist训练集的10-NN近邻图，存放于百度云：
http://pan.baidu.com/s/1dEALTTn 提取码：xd2x

IEH_sift.eps, IEH_gist.eps 分别是我在两个数据集上进行测试的时间对召回率的曲线矢量图。分别在1NN，10NN，50NN，100NN对算法进行了测试
