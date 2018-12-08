
#include "dbg.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>


using namespace std;

typedef int(*UNCOMPRESS)(char* dst, uint32_t* dstLength, char* src, uint32_t srcLength);
UNCOMPRESS uncompress;


std::string UnicodeToGBK(const std::wstring& ustr)
{
    int n = WideCharToMultiByte(CP_ACP, NULL, ustr.c_str(), -1, NULL, NULL, NULL, NULL);
    vector<char> mcb(n + 1);
    WideCharToMultiByte(CP_ACP, NULL, ustr.c_str(), -1, mcb.data(), n, "_", NULL);
    return mcb.data();
}

int SplitFileNameAndSave(const string& curDir, const string& filename, const vector<char>& unpackData)
{
	DWORD ByteWrite;
	string buf;

    if (curDir.back() != '\\')
	    buf = curDir + "\\" + filename;
    else
        buf = curDir + filename;

	int len = buf.size();
	int i = curDir.size() + 1;
	const char* p = buf.c_str();
	const char* end = buf.c_str() + len;
	while (p <= end && i < len)
	{
		while (buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
		if (buf[i] == '/') buf[i] = '\\';
		if (i < len)
		{
			auto tmp = buf[i];
			buf[i] = '\0';

			CreateDirectory(p, 0);
			buf[i] = tmp;
			++i;
		}
	}

	wstring buf2;
	HANDLE hFile;
	int ret = 0;
	do {
		hFile = CreateFile(buf.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			ret = GetLastError();
			break;
		}

		BOOL success = WriteFile(hFile, unpackData.data(), unpackData.size(), &ByteWrite, NULL);

		if (success && ByteWrite == unpackData.size())
		{
			ret = ERROR_SUCCESS;
		}
		else
		{
			ret = GetLastError();
		}

	} while (0);

	CloseHandle(hFile);
	return ret;
}


bool StartProcess(const string& exePath, const string& exeDir, bool suspend, PROCESS_INFORMATION* ppi)
{
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_MINIMIZE;
    PROCESS_INFORMATION pi;
    char cmdLine[1024];
    strcpy_s(cmdLine, sizeof(cmdLine), exePath.c_str());

    int flag = (suspend ? CREATE_SUSPENDED : 0) | DEBUG_ONLY_THIS_PROCESS;
    BOOL Success = CreateProcess(NULL, cmdLine,
        NULL, NULL, FALSE, flag, NULL, exeDir.c_str(),
        &si, &pi);
    if (!Success)
    {
        printf("start exe failed(%#x).\n", GetLastError());
        return false;
    }

    if (!suspend)
    {
        CloseHandle(pi.hThread);
        pi.hThread = NULL;
    }
    *ppi = pi;
    return true;
}


void StopDebug(DebugInfo* di)
{
    TerminateProcess(di->hProcess, 0);
}


void ResetBreakPoint(DEBUG_EVENT* de, DebugInfo* di)
{
	if (di->hProcess == NULL)
		return;

	for (size_t i = 0; i < di->mBP.size(); ++i)
	{
		if (di->mBP[i].mNeedResetBP)
		{
			HANDLE hT = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT, FALSE, de->dwThreadId);
			assert(hT);
			CONTEXT ctx;
			ctx.ContextFlags = CONTEXT_FULL;
			BOOL b = GetThreadContext(hT, &ctx);
			assert(b);
			CloseHandle(hT);

			SIZE_T D;
			ReadProcessMemory(di->hProcess, (LPVOID)di->mBP[i].mAddr, &di->mBP[i].mOldByte, 1, &D);
			WriteProcessMemory(di->hProcess, (LPVOID)di->mBP[i].mAddr, &di->int3, 1, &D);
			di->mBP[i].mNeedResetBP = false;
		}
	}
}


void TriggerBreakPoint(DEBUG_EVENT* de, DebugInfo* di)
{
	if (di->hProcess == NULL)
		return;

	for (size_t i = 0; i < di->mBP.size(); ++i)
	{
		if (de->u.Exception.ExceptionRecord.ExceptionAddress == (LPVOID)di->mBP[i].mAddr)
		{
			di->mBP[i].mHandler(&di->mBP[i], de);
			break;
		}
	}
}


bool ModifyThreadContext(DWORD tid, std::function<bool(CONTEXT& ctx)> f)
{
    // f return true to set context back

    HANDLE hT = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT, FALSE, tid);
    if (hT == NULL)
        return false;

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(hT, &ctx))
    {
        CloseHandle(hT);
        return false;
    }

    bool result = f(ctx);

    if (result)
    {
        if (!SetThreadContext(hT, &ctx))
        {
            CloseHandle(hT);
            return false;
        }
    }

    CloseHandle(hT);
    return true;
}

// 只按 4 字节对齐搜索
int SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, uint32_t value, uint32_t* result, uint32_t arrayCount)
{
	assert((from & 3) == 0);
	assert((to & 3) == 0);
	assert(arrayCount > 0);
	BYTE remoteMemory[4096];

	uint32_t index = 0;
	while (from < to && index < arrayCount)
	{
		SIZE_T D;
		DWORD length = ((from + 0x1000) & ~0xfff) - from;
		ReadProcessMemory(hProcess, (LPVOID)from, remoteMemory, length, &D);
		if (D == 0)
		{
			from = (from + 0x1000) & ~0xfff;
			continue;
		}

		for (DWORD p = 0; p < length; p += 4)
		{
			if (value == *(PDWORD)(remoteMemory+p))
			{
				result[index++] = from + p;
				assert(index < arrayCount);
			}
		}

		from += length;
	}
	return index;
}

int SearchMemory(HANDLE hProcess, uint32_t from, uint32_t to, const char* str, uint32_t strLength, uint32_t* result, uint32_t arrayCount)
{
    assert(strLength > 0);
	assert(arrayCount > 0);
	BYTE remoteMemory[4096];

    uint32_t index = 0;
    while (from < to && index < arrayCount)
    {
		SIZE_T D;
		DWORD length = ((from + 0x1000) & ~0xfff) - from;
		ReadProcessMemory(hProcess, (LPVOID)from, remoteMemory, length, &D);
		if (D == 0)
		{
			from = (from + 0x1000) & ~0xfff;
			continue;
		}

		for (DWORD p = 0; p < length; ++p)
		{
			if (!memcmp(&remoteMemory[p], str, strLength))
			{
				result[index++] = from + p;
				assert(index < arrayCount);
				p += strLength - 1;		// ++p
			}
		}

		from += length;
    }
	return index;
}


bool FindVtbl(HANDLE hProcess, uint32_t classNameAddr, uint32_t* pResult)
{
	uint32_t addr = 0;
	SIZE_T D;
	int num = 0;
	uint32_t result[32] = { 0 };
	uint32_t temp[16];
	
	uint32_t searchRangeLow = 0;
	uint32_t searchRangeHigh = 0x70000000;

	num = SearchMemory(hProcess, searchRangeLow, searchRangeHigh, (classNameAddr & ~0x3) - 8, result, _countof(result));
	cout << (num > 0 ? "Cxdec1 found!\n" : "Cxdec1 not found.\n");
	if (num == 0)
		return false;

	int validCount = 0;
	for (int i = 0; i < num; ++i)
	{
		ReadProcessMemory(hProcess, (PVOID)(result[i] - 8), temp, 8, &D);
		assert(D == 8);
		if (temp[0] == 0)
			result[validCount++] = result[i];
	}

	vector<uint32_t> result1;
	for (int i = 0; i < validCount; ++i)
	{
		uint32_t temp[32];
		num = SearchMemory(hProcess, searchRangeLow, searchRangeHigh, result[i] - 12, temp, _countof(temp));
		for (int i = 0; i < num; ++i) result1.push_back(temp[i]);
	}

	vector<uint32_t> result2;
	for (int i = 0; i < result1.size(); ++i)
	{
        // 自身所在地址为只读
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQueryEx(hProcess, (LPVOID)(result1[i] + 4), &mbi, sizeof(mbi));
        if (mbi.Protect != PAGE_READONLY)
            continue;
        
        uint32_t temp[32];
		num = SearchMemory(hProcess, searchRangeLow, searchRangeHigh, result1[i] + 4, temp, _countof(temp));
        if (num > 0)
        {
            // 表中指向地址为可执行，目前有两个虚函数（虚析构以及解密函数）
            SIZE_T D;
            uint32_t vfn[2] = { 0 };
            ReadProcessMemory(hProcess, (LPVOID)(result1[i] + 4), vfn, sizeof(vfn), &D);
            assert(D == sizeof(vfn));
            MEMORY_BASIC_INFORMATION mbi, mbi2;
            VirtualQueryEx(hProcess, (LPVOID)vfn[0], &mbi, sizeof(mbi));
            VirtualQueryEx(hProcess, (LPVOID)vfn[1], &mbi2, sizeof(mbi2));
            if (mbi.Protect == PAGE_EXECUTE_READ && mbi.Protect == PAGE_EXECUTE_READ)
            {
                result2.push_back(result1[i] + 4);  // 推断为目标虚表
            }
        }
	}

    assert(result2.empty() || result2.size() == 1);
    if (!result2.empty())
        *pResult = result2[0];
	//num = SearchMemory(hProcess, searchRangeLow, searchRangeHigh, result[0] + 4, result, _countof(result));
	//cout << (num > 0 ? "Cxdec3 found!\n" : "Cxdec3 not found.\n");
	//if (num > 0)
	//{
	//	assert(num == 1);
	//	*pResult = result[0];
	//	return true;
	//}
	//else
	//{
	//	return false;
	//}
	return result2.size() == 1;
}


__declspec(naked) void Extracter()
{
	// esp + 1c: constructor
    // esp + 18: buffer
    // esp + 14: buffer_length
    // esp + 10: 0
    // esp +  c: part_length
    // esp +  8: checksum
    // esp +  4: vptr
    // esp +  0: esp + 4
    __asm {
        sub esp, 0x20
Restart:
		int 3
		mov eax, [esp+0x1c]
        mov ecx, esp
		call eax
		mov ecx, eax
        mov edx, [ecx]
        mov edx, [edx+4]
        push [esp+0x14]
        push [esp+0x1c]
        xor eax, eax
        push eax
        push eax
        call edx
        jmp Restart
    }
}

char code[] = {
	'\x83', '\xEC', '\x20', '\xCC', '\x8B', '\x44', '\x24', '\x1c', '\x8B', '\xCC', '\xFF', '\xD0',
	'\x8B', '\xC8', '\x8B', '\x11', '\x8B', '\x52', '\x04', '\xFF', '\x74', '\x24', '\x14', '\xFF',
	'\x74', '\x24', '\x1c', '\x33', '\xC0', '\x50', '\x50', '\xFF', '\xD2', '\xEB', '\xE0'
};

struct BP1UserData
{
	uint32_t	 fileIndex{ 0 };
	vector<char> packData;
	vector<char> plainData;
};

void InitBreakPoints(DEBUG_EVENT* de, DebugInfo* di)
{
	SIZE_T D;
	HMODULE hDll = LoadLibrary("kernelbase.dll");
	assert(hDll);
	FARPROC fnReadFile = GetProcAddress(hDll, "ReadFile");
	assert(fnReadFile);
	di->mBP[0].mAddr = (DWORD)fnReadFile;
	FreeLibrary(hDll);
	ReadProcessMemory(di->hProcess, (LPVOID)di->mBP[0].mAddr, &di->mBP[0].mOldByte, 1, &D);
	WriteProcessMemory(di->hProcess, (LPVOID)di->mBP[0].mAddr, &DebugInfo::int3, 1, &D);

	di->mBP[0].mHandler = [di](BreakPointInfo* self, DEBUG_EVENT* de) {
		SIZE_T D;
		WriteProcessMemory(di->hProcess, (LPVOID)self->mAddr, &self->mOldByte, 1, &D);
		bool b = ModifyThreadContext(de->dwThreadId, [self, di](CONTEXT& ctx) {
			--ctx.Eip;
			ctx.EFlags |= 0x100;        // 设单步以便重新设置断点
			DWORD remoteSize = 0;
			DWORD D;
			ReadProcessMemory(di->hProcess, (LPVOID)(ctx.Esp + 0xc), &remoteSize, 4, &D);
			if (remoteSize == di->mStartupFile.GetTotlePackedSize())
				self->mBpCondition = true;
			return true;
		});
		assert(b);
		self->mNeedResetBP = true;

		if (self->mBpCondition)
		{
			cout << "start search memory...\n";
			uint32_t result[32];
			int num = SearchMemory(di->hProcess, 0, 0x70000000, "AVCxdecFilterCore", 17, result, _countof(result));
			cout << (num > 0 ? "Cxdec found!\n" : "Cxdec not found.\n");
			if (num > 0)
			{
				stringstream ss;
				ss << "Cxdec ID address: ";
				for (int i = 0; i < num; ++i) ss << hex << result[i] << " ";
				cout << ss.str() << "\n";

				uint32_t table[32] = { 0 };
				for (int i = 0; i < num; ++i)
				{
					bool b1 = FindVtbl(di->hProcess, result[i], &table[i]);
					if (b1)
					{
						ss.str("");
						ss << table[i] << ", ";
						cout << "table address: " << ss.str() << "\n";
						assert(!di->mCxdecVtbl);
						di->mCxdecVtbl = table[i];

						// 查找整数
						uint32_t temp[32];
						int num = SearchMemory(di->hProcess, 0, 0x70000000, di->mCxdecVtbl, temp, _countof(temp));
						assert(num > 0);
						for (int i = 0; i < num; ++i)
						{
							MEMORY_BASIC_INFORMATION mbi;
							VirtualQueryEx(di->hProcess, (LPVOID)temp[i], &mbi, sizeof(mbi));
							if (mbi.Protect == PAGE_EXECUTE_READ)
							{
								assert(di->mKeyPair.first == 0 && di->mKeyPair.second == 0);
								vector<uint8_t> codebytes(128);
								SIZE_T W;
								ReadProcessMemory(di->hProcess, (LPVOID)(temp[i] - codebytes.size()), codebytes.data(), codebytes.size(), &W);
								assert(W == codebytes.size());
								int k = 0;
								for (k = codebytes.size() - 1; k >= 0; --k)
									if (codebytes[k] == 0xcc)
										break;
								assert(k > 0);
								di->mCxdecCon = temp[i] - (128 - k) + 1;
								ss.str("");
								ss << "constructor: " << di->mCxdecCon;
								cout << ss.str() << "\n";
							}
						}
					}
				}
				
				assert(di->mCxdecVtbl);
				self->mRemoteMemory = VirtualAllocEx(di->hProcess, NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                assert(self->mRemoteMemory);
				SIZE_T W;
				WriteProcessMemory(di->hProcess, self->mRemoteMemory, code, sizeof(code), &W);
				assert(W == sizeof(code));
				bool b = ModifyThreadContext(de->dwThreadId, [self, di](CONTEXT& ctx) {
					ctx.Eip = (DWORD)self->mRemoteMemory;
					return true;
				});
				assert(b);
				di->mBP[1].mAddr = (DWORD)self->mRemoteMemory + 3;
			}
		}
	};

	di->mBP[1].mAddr = 0;
	di->mBP[1].mOldByte = DebugInfo::int3;
	//di->mBP[1].mNeedResetBP = true;
	di->mBP[1].mUserData = new BP1UserData;
	((BP1UserData*)di->mBP[1].mUserData)->fileIndex = -1;
	di->mBP[1].mHandler = [di](BreakPointInfo* self, DEBUG_EVENT* de) {

		// 收取上一次的
		uint32_t* pFileIndex = &((BP1UserData*)self->mUserData)->fileIndex;
		if (self->mRemoteMemory)
		{
			const file_entry& curFile = di->mFileList[*pFileIndex];
			vector<char> clearData(curFile.GetTotleOriginalSize());
			SIZE_T W;
			ReadProcessMemory(di->hProcess, self->mRemoteMemory, clearData.data(), clearData.size(), &W);
			assert(curFile.GetTotleOriginalSize() == W);
			int ok = !SplitFileNameAndSave(di->mSaveDir, UnicodeToGBK(curFile.file_name), clearData);
			cout << "save [" << UnicodeToGBK(curFile.file_name) << "] " << (ok ? "success.\n" : "failed.\n");
			// 并释放内存
			VirtualFreeEx(di->hProcess, self->mRemoteMemory, 0, MEM_RELEASE);
		}

		++(*pFileIndex);

		if (*pFileIndex >= di->mFileList.size())
		{
			cout << "extract finished.\n";
			StopDebug(di);
			return;
		}

		if (!di->mFileList[*pFileIndex].IsEncrypted())
			cout << UnicodeToGBK(di->mFileList[*pFileIndex].file_name) << " [not encrypted].\n";

        ifstream in(di->mXp3Path, ios::binary);
		assert(in.is_open());
		in.seekg(0, ios::end);
		vector<char> fileData(di->mFileList[*pFileIndex].GetTotlePackedSize());
		vector<char> plainData(di->mFileList[*pFileIndex].GetTotleOriginalSize());
        di->mFileList[*pFileIndex].ReadFileData(in, fileData);
		in.close();
		if (di->mFileList[*pFileIndex].IsCompressed())
		{
			uint32_t plainLength = plainData.size();
			int ok = uncompress(plainData.data(), &plainLength, fileData.data(), fileData.size());
			assert(ok == 0);
		}
		else
			plainData = fileData;

		self->mRemoteMemory = VirtualAllocEx(di->hProcess, NULL, plainData.size(), MEM_COMMIT, PAGE_READWRITE);
		SIZE_T W;
		WriteProcessMemory(di->hProcess, self->mRemoteMemory, plainData.data(), plainData.size(), &W);
		assert(W == plainData.size());

		bool b = ModifyThreadContext(de->dwThreadId, [pFileIndex, self, di](CONTEXT& ctx) {
			SIZE_T W;
			DWORD writeBuffer;
			//writeBuffer = di->mCxdecVtbl;
			//WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp +  0), &writeBuffer, 4, &W); assert(W == 4);
			//writeBuffer = di->mFileList[*pFileIndex].mChecksum;
			//WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp +  4), &writeBuffer, 4, &W); assert(W == 4);
			//writeBuffer = 0x51;
			//WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp +  8), &writeBuffer, 4, &W); assert(W == 4);
			//writeBuffer = 0;
			//WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp + 12), &writeBuffer, 4, &W); assert(W == 4);
            writeBuffer = ctx.Esp + 4;
            WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp +  0), &writeBuffer, 4, &W); assert(W == 4);
            writeBuffer = di->mFileList[*pFileIndex].checksum;
			WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp +  4), &writeBuffer, 4, &W); assert(W == 4);
			writeBuffer = di->mFileList[*pFileIndex].GetTotleOriginalSize();
			WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp + 20), &writeBuffer, 4, &W); assert(W == 4);
			writeBuffer = (DWORD)self->mRemoteMemory;
			WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp + 24), &writeBuffer, 4, &W); assert(W == 4);
            writeBuffer = di->mCxdecCon;
            WriteProcessMemory(di->hProcess, (LPVOID)(ctx.Esp + 28), &writeBuffer, 4, &W); assert(W == 4);
			return true;
		});
		assert(b);
	};

	HMODULE hZlib = LoadLibrary("zlib.dll");
	assert(hZlib);
	uncompress = (UNCOMPRESS)GetProcAddress(hZlib, "uncompress");
	assert(uncompress);
}


bool DebugIt(const string& exePath, const string& exeDir, const std::string& xp3Path, const string& saveDir, vector<file_entry>& fileList, file_entry& startuptjs)
{
    PROCESS_INFORMATION pi;
    if (!StartProcess(exePath, exeDir, false, &pi))
        return false;

    DebugInfo di(pi.hProcess, fileList, startuptjs);
    di.mSaveDir = saveDir;
    di.mXp3Path = xp3Path;
	di.mExePath = exePath;
	di.mExeDir = exeDir;

	HMODULE hKernelBase = LoadLibrary("kernelbase.dll");
	assert(hKernelBase);
	PVOID fnReadFile = GetProcAddress(hKernelBase, "ReadFile");
	assert(fnReadFile);

    DEBUG_EVENT de;
    while (1)
    {
        if (di.mStopDebugger)
        {
            DWORD r = WaitForSingleObject(di.hProcess, INFINITE);
            assert(r == WAIT_OBJECT_0);
            CloseHandle(di.hProcess);
            di.hProcess = NULL;
            return true;
        }

        BOOL b1 = WaitForDebugEvent(&de, 5000);
        assert(b1);

        if (de.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT)
        {
            // 加载地址 0x10000000
//             if (de.u.LoadDll.lpBaseOfDll == (LPVOID)DebugInfo::PalDllBase[static_cast<int>(ver)])
//             {
//                 HANDLE hPalDll = de.u.LoadDll.hFile;
//                 CloseHandle(hPalDll);
//                 di.mDllBaseAddr = de.u.LoadDll.lpBaseOfDll;
// 
//                 // 调用 dll 的入口处，位于 exe 中
//                 di.mExeImageBase = GetProcessImageBaseAddr(de.dwProcessId);
//                 assert(di.mExeImageBase);
// 
//                 InitBreakPoints(&de, &di);
//             }
			if (de.u.LoadDll.lpBaseOfDll == hKernelBase)
			{
				InitBreakPoints(&de, &di);
			}
			//InitBreakPoints(&de, &di);
        }
        else if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
        {
            if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP)
            {
                ResetBreakPoint(&de, &di);
            }
            else if (de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT)
            {
                TriggerBreakPoint(&de, &di);
            }
        }
        else if (de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
        {
            di.mStopDebugger = true;
        }


        b1 = ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
        assert(b1);
    }

    return false;
}