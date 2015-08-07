##Extract BGI
提取BGI的arc文件

#文件
main.cpp
arc.cpp/h	- 提取arc中文件索引并调用相应的dsc/cbg进行处理
cbg.cpp/h	- 解码CompressedBG___类型文件
dsc.cpp/h	- 解码DSC FORMAT 1.00类型文件
error.h
type.h

***************************************
CBG v2
原来CompressedBG___还是有Ver.2的...*(short*)(file_head_ptr + 0x2e)
原先的cbg.cpp 改为 cbg_v1.cpp
新增的则是CBG_v2.h 