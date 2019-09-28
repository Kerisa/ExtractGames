// leaf.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <cassert>
#include <iostream>
#include "leaf.h"

int main(int argc, char** argv)
{
    assert(argc == 2);

    Leaf leaf;
    leaf.Open(argv[1]);
    leaf.ExtractEntries();
    leaf.ExtractResource();
    std::cout << "Hello World!\n"; 
    return 0;
}
