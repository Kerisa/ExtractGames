
#include "merge2.h"

#ifdef _DEBUG
#pragma comment (lib, "zlibd.lib")
#pragma comment (lib, "libpng16d.lib")
#else
#pragma comment (lib, "zlib.lib")
#pragma comment (lib, "libpng16.lib")
#endif

Merge merge;

int wmain(int argc, wchar_t **argv)
{
	//
	// argv[1] - TxtFilename
	// argv[2] - groupNum
	// argv[3] - group0MaxId
	// argv[4] - group1MaxId
	// ...
	//
	if (argc < 5)		// 至少有两个分组
	{
		printf("Usage:\n    Merge.exe <TXTFilename> "
            "<groupNum> <group0MaxId> <group1MaxId> [...]\n");
		return 0;
	}

	std::vector<int> g;
	for (int i = 3; i < argc; ++i)
		g.push_back(_wtoi(argv[i]));

    merge.Initialize(argv[1], _wtoi(argv[2]), g);
	merge.Process();

	//system("pause");
    return 0;
}