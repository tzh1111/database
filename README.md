# database
temp is unexpectedly rewrite in cha making cnt cannot reach 9(still in question -?-)

写后继地址导致AND出现内存覆盖，因此重开一个reload_abc给AND及其同伙使用——那就没有后继地址了哦，乱码就完事儿了==我有什么办法，我只是个孩子啊T^T还整了我一天，愁人！

外排好像有点小问题，问题不是很大，有一些零碎的没排到，不知道怎么了的，先放着，不查到这些数据问题就不大。

基于外排的二分查找可能要重构了，一个一个内存块拎进去不太现实，我感觉要开个结构体数组，少装点，大概来个一两块，还是可以忍受的。

其他的函数经过本次AND的叛乱之后，也没有检查哦，看了线性查找目前是对的，另外把temp_count为了给AND用又加到了全局变量，在其他函数里声明的局部变量应该会覆盖，出了问题就找它了，哼哼。
