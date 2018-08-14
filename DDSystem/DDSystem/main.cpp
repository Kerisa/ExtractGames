

#include <iostream>
#include <string>
#include "DDSystem.h"

using namespace std;


int main(int argc, char** argv)
{
	if (argc != 3)
	{
		cout << "usage: DDSystem.exe <package file> <output directory>.\n";
		return 0;
	}

    DDSystem dds;
    if (!dds.Open(argv[1]))
    {
        cout << "open [" << argv[1] << "] failed.\n";
        return 0;
    }

    if (!dds.ExtractEntries())
    {
        cout << "extract entries failed.\n";
        return 0;
    }

    string dir(argv[2]);
    if (dir.back() != '\\')
        dir += '\\';

    
	if (!dds.ExtractResource(dir))
	{
		cout << "some resource save failed.\n";
		return 0;
	}

	dds.Close();
	
    return 0;
}