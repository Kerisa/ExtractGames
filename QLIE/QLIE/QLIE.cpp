
#include <cassert>
#include <Windows.h>
#include <vector>
#include <strsafe.h>
#include "QLIE.h"

using std::vector;


static int SplitFileNameAndSave(const wchar_t *cur_dir, const wchar_t *file_name, void* unpack, unsigned long file_length)
{
	DWORD ByteWrite;
	wchar_t buf[MAX_PATH] = {0}, buf2[MAX_PATH];

	StringCchCopy(buf, MAX_PATH, cur_dir);
	StringCchCat (buf, MAX_PATH, L"\\");
	StringCchCat (buf, MAX_PATH, file_name);

	int len = wcslen(buf);
	int i = wcslen(cur_dir) + 1;
	wchar_t *p = buf, *end = buf + len;
	while (p <= end && i < len)
	{
		while(buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
		if (buf[i] == '/') buf[i] = '\\';
		if (i<len)
		{
			wchar_t tmp = buf[i];
			buf[i] = '\0';

			CreateDirectoryW(p, 0);
			buf[i] = tmp;
			++i;
		}
	}

	HANDLE hFile;
	int ret = 0;
	do{
		hFile = CreateFile(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			StringCchPrintf(buf2, MAX_PATH, L"[文件创建错误]%s\r\n", file_name);
			ret = -1;
			break;
		}

		WriteFile(hFile, unpack, file_length, &ByteWrite, NULL);

		if (ByteWrite != file_length)
		{
			StringCchPrintf(buf2, MAX_PATH, L"[文件写入错误]%s\r\n", file_name);
			ret = -2;
			break;
		}
		
		int t = GetLastError();
		if (!t || t == ERROR_ALREADY_EXISTS)
			StringCchPrintf(buf2, MAX_PATH, L"[已保存]%s\r\n", file_name);
		else
		{
			StringCchPrintf(buf2, MAX_PATH, L"[无法保存]%s\r\n", file_name);
			ret = -3;
		}
	}while(0);

	AppendMsg(buf2);
	CloseHandle(hFile);
	return ret;
}


DWORD GenerateKey(PVOID Buf, DWORD Len)
{
	// paddw
	PWORD wBuf = (PWORD) Buf;
	WORD  mm0[4], mm2[4];

	for (DWORD i=0; i<4; ++i)
		mm0[i] = mm2[i] = 0;

	for (DWORD k=0; k<Len/8; ++k)
	{
		for (DWORD i=0; i<4; ++i)
		{
			mm2[i] += 0x307;
			mm0[i] += wBuf[i] ^ mm2[i];
		}
		wBuf += 4;
	}

	return (mm0[3] ^ mm0[1]) << 16 | mm0[0] ^ mm0[2];
}


DWORD GenerateKey2(const PVOID Buf, DWORD Len)
{
	uint64_t mm0 = 0ul;
	uint64_t mm2 = 0ul;
	const uint64_t mm3 = 0xA35793A7A35793A7ul;

	// paddw
    const uint64_t* buf = (const uint64_t*)Buf;
	for (int i = 0; i < Len / 8; ++i)
	{
		uint64_t mm1 = *buf++;
		for (int k = 0; k < 4; ++k)
			((uint16_t*)&mm2)[k] += ((uint16_t*)&mm3)[k];
		mm1 ^= mm2;
		for (int k = 0; k < 4; ++k)
			((uint16_t*)&mm0)[k] += ((uint16_t*)&mm1)[k];
		uint32_t u1 = mm0 >> 32;
		uint32_t u2 = mm0 & 0xfffffffful;
		mm0 = ((uint64_t)((u1 >> 29) | (u1 << 3)) << 32) | (uint64_t)((u2 >> 29) | (u2 << 3));
	}

	uint64_t mm1 = mm0 >> 32;
	int32_t tmp1 = (int32_t)((int16_t*)&mm0)[1] * ((int16_t*)&mm1)[1] + ((int16_t*)&mm0)[0] * ((int16_t*)&mm1)[0];
	int32_t tmp2 = (int32_t)((uint16_t*)&mm0)[3] * ((uint16_t*)&mm1)[3] + ((uint16_t*)&mm0)[2] * ((uint16_t*)&mm1)[2];
	mm0 = ((uint64_t)tmp2 << 32) | (uint32_t)tmp1;
	return static_cast<DWORD>(mm0);
/*
004E259C  |.  0FEFC0        pxor    mm0, mm0
004E259F  |.  0FEFD2        pxor    mm2, mm2
004E25A2  |.  B8 A79357A3   mov     eax, A35793A7
004E25A7  |.  0F6ED8        movd    mm3, eax
004E25AA  |.  0F62DB        punpckldq mm3, mm3
004E25AD  |>  0F6F0E        /movq    mm1, qword ptr [esi]
004E25B0  |.  0FFDD3        |paddw   mm2, mm3
004E25B3  |.  0FEFCA        |pxor    mm1, mm2
004E25B6  |.  0FFDC1        |paddw   mm0, mm1                        ;  +
004E25B9  |.  0F6FC8        |movq    mm1, mm0
004E25BC  |.  0F72F0 03     |pslld   mm0, 3
004E25C0  |.  0F72D1 1D     |psrld   mm1, 1D                         ;  循环左移3位
004E25C4  |.  0FEBC1        |por     mm0, mm1                        ;  保存在 mm0
004E25C7  |.  83C6 08       |add     esi, 8
004E25CA  |.  49            |dec     ecx
004E25CB  |.^ 75 E0         \jnz     short 004E25AD
004E25CD  |.  0F6FC8        movq    mm1, mm0
004E25D0  |.  0F73D1 20     psrlq   mm1, 20                          ;  q右移32位
004E25D4  |.  0FF5C1        pmaddwd mm0, mm1
004E25D7  |.  0F7E45 F4     movd    dword ptr [ebp-C], mm0           ;  mm0 = f17b4f4c
004E25DB  |.  0F77          emms
*/
}


// ver cl == 1
void Decode(PVOID Buf, DWORD Len, DWORD Key)
{
	PDWORD dBuf = (PDWORD) Buf;
	DWORD  mm5[2], mm6[2], mm7[2];

	for (DWORD i=0; i<2; ++i)
	{
		mm6[i] = 0xce24f523;
		mm7[i] = 0xa73c5f9d;
		mm5[i] = (Key + Len) ^ 0xfec9753e;
	}

	for (DWORD i=0; i<Len/8; ++i)
	{
		for (DWORD j=0; j<2; ++j)
		{
			mm7[j] = (mm7[j] + mm6[j]) ^ mm5[j];
			dBuf[j] ^= mm7[j];
			mm5[j] = dBuf[j];
		}
		dBuf += 2;
	}
	return;
}


int Decode_Sub1(DWORD dwSeed, PDWORD arrDecode, PDWORD pSubFlag)
{
    constexpr uint32_t magic = 0x8df21431;
	// 类似于初始化
    uint32_t val = dwSeed;
    for (int i = 0; i < 0x40; ++i)
    {
        val ^= magic;
        uint64_t mul = (uint64_t)val * magic;
        val = arrDecode[i] = (mul & 0xffffffff) + (mul >> 32);
    }


	*pSubFlag = 1;

	return 0;
}


void Decode_Sub2(PVOID Buf, DWORD Len, PDWORD arrDecode)
{	
	// 处理exe文件中的Key时长度是0x423e，
	// 但好像无所谓反正只用前面256字节

	PDWORD pdwBuf = (PDWORD) Buf;
	DWORD  Len1 = Len / 4;

	if (Len1 > 64)
		Len1 = 64;

	for (DWORD i=0; i<Len1; ++i)
	{
		arrDecode[i] ^= pdwBuf[i];
	}

	return;
}


DWORD Decode_Sub3(PDWORD arrDecode, PDWORD pSubFlag)
{
	static const DWORD A = 64, B = 39;
	DWORD i, eax;
	

	if (0 == --(*pSubFlag))
	{
		*pSubFlag = A;

		for (i=0; i<A-B; ++i)
		{
			eax = (arrDecode[i] & 0x80000000) | ((arrDecode[i+1] & 0x7fffffff) >> 1);
			if (arrDecode[i+1] & 1)
				eax ^= 0x9908b0df;
			arrDecode[i] = eax ^ arrDecode[i+B];
		}

		for (; i<A-1; ++i)
		{
			eax = (arrDecode[i] & 0x80000000) | ((arrDecode[i+1] & 0x7fffffff) >> 1);
			if (arrDecode[i+1] & 1)
				eax ^= 0x9908b0df;
			arrDecode[i] = eax ^ arrDecode[i-(A-B)];
		}

		eax = (arrDecode[A-1] & 0x80000000) | ((arrDecode[0] & 0x7fffffff) >> 1);
		if (arrDecode[A] & 1)			//	!
			eax ^= 0x9908b0df;
		arrDecode[A-1] = eax ^ arrDecode[B-1];
	}
	
	eax = arrDecode[A - *pSubFlag];		//	!

	eax ^= eax >> 11;
	eax ^= (eax << 7) & 0x9c4f88e3;
	eax ^= (eax << 15) & 0xe7f70000;
	eax ^= eax >> 18;

	return eax;
}


int DecodeKeyFileData(PVOID PackData, const PACKIDX& Idx, PBYTE pExeKey, DWORD dwExeKeyLen, PBYTE pKeyFile, DWORD dwKeyFileLen)
{
    DWORD dwSum = 0x85f532;
    DWORD dwSeed = 0x33f641;
    DWORD SubFlag;
    DWORD arrDecode[256];	// "8qH"


    assert(Idx.IsEncrypted == 1);

    const uint32_t dwFileNameLen = wcslen((const wchar_t*)Idx.Name);
    for (DWORD i = 0; i < dwFileNameLen; ++i)
    {
        dwSum += ((uint32_t)((wchar_t*)Idx.Name)[i] << (i & 7));
        dwSeed ^= dwSum;
    }

    dwSeed += ((Idx.CompressLen ^ 0x8f32dc ^ dwSum) + dwSum + Idx.CompressLen + 7 * (Idx.CompressLen & 0xffffff)) ^ Idx.Key;
    dwSeed = (dwSeed & 0xffffff) * 9;
    //dwSeed ^= 0x453a;


    Decode_Sub1(dwSeed, arrDecode, &SubFlag);

    typedef union {
        ULONGLONG	Qword;
        DWORD		Dword[2];
        WORD		Word[4];
        BYTE		Byte[8];
    } UQWORD;
    UQWORD Tmp;
    Tmp.Dword[1] = arrDecode[7];
    Tmp.Dword[0] = arrDecode[6];

    ULONGLONG *p = (ULONGLONG*)PackData;
    ULONGLONG *pEnd = (ULONGLONG*)PackData + Idx.CompressLen / 8;
    uint8_t* parr1 = (uint8_t*)arrDecode;

    DWORD arr1Idx = (arrDecode[0xd] & 0xf) * 8;
    while (p < pEnd)
    {
        // pxor mm7, mm6
        Tmp.Qword ^= *(uint64_t*)(parr1 + arr1Idx);

        // paddd mm7, mm6
        Tmp.Dword[0] += *(uint32_t*)(parr1 + arr1Idx);
        Tmp.Dword[1] += *(uint32_t*)(parr1 + arr1Idx + 4);

        // movq mm0, qword ptr[edi]
        // pxor mm0, mm7
        // movq mm1, mm0
        // mov1 qword ptr [edi], mm0
        *p ^= Tmp.Qword;

        // paddb mm7, mm1   (*ppd)
        for (DWORD i = 0; i < 8; ++i)
            Tmp.Byte[i] += *((PBYTE)p + i);

        // pxor mm7, mm1
        Tmp.Qword ^= *p;

        // pslld mm7, 1
        Tmp.Dword[0] <<= 1;
        Tmp.Dword[1] <<= 1;

        // paddw mm7, mm1
        for (DWORD i = 0; i < 4; ++i)
            Tmp.Word[i] += *((PWORD)p + i);

        ++p;
        arr1Idx = (arr1Idx + 8) & 0x7f;
    }
    return 0;
}


int Decode_Sub12(DWORD dwSeed, PDWORD arrDecode, PDWORD pSubFlag)
{
    // 类似于初始化
    constexpr uint32_t magic = 0x8a77f473;
    uint32_t val = dwSeed;
    for (int i = 0; i < 0x40; ++i)
    {
        val ^= magic;
        uint64_t mul = (uint64_t)val * magic;
        val = arrDecode[i] = (mul & 0xffffffff) + (mul >> 32);
    }


    *pSubFlag = 1;

    return 0;
}

int DecodeKeyFileData2(PVOID PackData, const PACKIDX& Idx, PBYTE pExeKey, DWORD dwExeKeyLen, PBYTE pKeyFile, DWORD dwKeyFileLen)
{
    DWORD dwSum = 0x86f7e2;
    DWORD dwSeed = 0x4437f1;
    DWORD SubFlag;
    DWORD arrDecode[256];	// "8qH"


    assert(Idx.IsEncrypted >= 2);

    const uint32_t dwFileNameLen = wcslen((const wchar_t*)Idx.Name);
    for (DWORD i = 0; i < dwFileNameLen; ++i)
    {
        dwSum += ((uint32_t)((wchar_t*)Idx.Name)[i] << (i & 7));
        dwSeed ^= dwSum;
    }

    dwSeed += ((Idx.CompressLen ^ 0x56e213 ^ dwSum) + dwSum + Idx.CompressLen + 0xd * (Idx.CompressLen & 0xffffff)) ^ Idx.Key;
    dwSeed = (dwSeed & 0xffffff) * 0xd;
    //dwSeed ^= 0x453a;


    Decode_Sub12(dwSeed, arrDecode, &SubFlag);

    DWORD anotherArrDecode[256] = {
        0x20ECBA48, 0x96BB6008, 0x8E88126E, 0x352F5BD7, 0xD7DA5F02, 0xB79F9A11, 0xCA7D6C03, 0x073FFEE3, 0x6C46DE6C, 0x6F81DB03,
0x2DED61E0, 0x9D6059E0, 0xB644A8D6, 0xE3C19CE1, 0x0D7125B6, 0xB4E439E7, 0x14ACDD13, 0x7769A599, 0xA79E759E, 0xA80DCD4D,
0xB9CC47AE, 0x0CD89750, 0xE1F76F8A, 0xD9830589, 0xC65C722E, 0x4A76B0EF, 0xE3D336A1, 0x8A6F136F, 0x3990CFC8, 0x3E81AF73,
0x3911747C, 0xC2A957E0, 0xECD27046, 0xEEF388F7, 0x2F12CC36, 0x7DE71CAB, 0xBB2003C9, 0x6E62F353, 0x03BE9CC2, 0x591FC763,
0xA6FB34C4, 0xC95473DE, 0xF9E46924, 0x20A8B4F5, 0xB59C3202, 0xB74E2D65, 0x7E3ACF9B, 0xEF5FFC5D, 0xA7C247F6, 0xEE652C6F,
0xEBBF63C6, 0x1CEDB2E0, 0x9969C9E8, 0xB159AC55, 0x4AEADD0E, 0x1C26C493, 0x14A39FAF, 0xAB24FEC3, 0xBABFC940, 0x1FB74051,
0x0DBB7F4C, 0x7BB7798A, 0x12523F0E, 0xA4495F55, 0x1D21C0E6, 0x71A4DC47, 0xAE8ADDF5, 0x4BC48003, 0x212286B6, 0x314BFE2B,
0xE213168E, 0x07F34E94, 0xD1B3BE70, 0x2B44E737, 0x65D2C6AA, 0x09387D95, 0x667AC551, 0x0467A31B, 0x3311BF1E, 0xE42647FB,
0xF2192FBC, 0x20E3B49C, 0x8F635C64, 0xBA053EED, 0x90C21690, 0x691BCA2F, 0x20783A1B, 0xC6DA9BCD, 0x26B5A944, 0x90398AC1,
0x0069D7B0, 0x8B90A904, 0xDF9ABE31, 0xC08E2C90, 0x0E84FAF5, 0x38AA9ED1, 0xAE42DAF1, 0x372D8AF8, 0xC71DC61C, 0x23E9B3DB,
0xCE2FE84B, 0x83ABE724, 0x1A6E1793, 0x25E6C650, 0x3A07F3BE, 0x69E96486, 0x07CD5601, 0xCBD73CA6, 0x7C1D1311, 0xE6DE8FE8,
0xD40F07CE, 0x50C2DAA8, 0xEB6F8282, 0xF74861CB, 0xCBEE1B52, 0x9A34CE4E, 0xDDC69006, 0x5AE8FEF6, 0x9F80A57C, 0xCCA4F043,
0xF74CBED4, 0x2B83F6DB, 0x677ED7D2, 0xF6BDBCE8, 0xC2DD4038, 0xEE4AA452, 0x1C046092, 0xF3FC3B4F, 0xE385951B, 0x97384739,
0x1A4BD617, 0x21D85406, 0x1492424A, 0x8BBDA984, 0xFE893FD7, 0xEB3174C4, 0x7FE0A3B8, 0xBD8314CD, 0x22EA3F5A, 0xE896A145,
0x3C64BC8F, 0x0C933C42, 0xEF66BE9A, 0x4FABE80F, 0x4F4ECCED, 0xCEACB557, 0xE3D1947E, 0x186FB647, 0xD382D8CD, 0xD23FA3DC,
0x43AE7EA9, 0xB507D6C7, 0xD187F7D6, 0x04A4A654, 0x92179493, 0x329C35AB, 0x187B7583, 0x13E1BF56, 0xA12DC1E5, 0x1F435CE4,
0x0843F786, 0xDF5A9D56, 0xF89B0035, 0xC7A8D016, 0x5617AF0B, 0x8B3BF28D, 0xED009028, 0xEFF269AA, 0x30A30CA6, 0xA052F8AD,
0x8661EC44, 0x5C968742, 0x91130287, 0xC1A2D86B, 0xF5FE5344, 0x1EE296F0, 0xCFEFC1AF, 0x742AEEC0, 0xAB2BE334, 0x5E5DED9E,
0x376E8A9A, 0xE7783207, 0x3B8187B0, 0x40CABA61, 0xE8DF9010, 0x8CFF5D65, 0x7053E118, 0x4258C004, 0xD0860991, 0x0A360306,
0xFAFBFEC5, 0x04F4E493, 0xD155E946, 0xA8A6AD1E, 0x6B9FADA5, 0x902CB8C9, 0x2ED84CE9, 0x35D41D05, 0x11DFE7C5, 0x954DFC1C,
0xC7B5C928, 0x5315AA6E, 0x01BECC77, 0x657C7823, 0xAF2689CF, 0x28875C0B, 0x5A187B93, 0xA2117BD4, 0xE7820AB4, 0xFA359E0F,
0x2710D1D3, 0xD62AA2D2, 0x61303AC1, 0xA2078E95, 0xA217A056, 0x1C1DC8B2, 0xCCE7B65E, 0x8084E97B, 0xD14AED20, 0x47DE3AC5,
0x8753A653, 0x10689FFD, 0xA6CDE78E, 0x7460F50E, 0x1581C04D, 0xA420C41A, 0xF39ECE0A, 0x8F50E1FD, 0x31F37CB7, 0xE3BB023A,
0xAFF7AF34, 0xB1F9200E, 0xA2FAEB1C, 0x0D7ECD3C, 0x8ABAC141, 0x7701EDB1, 0xCAE4C399, 0x3F3FB038, 0xE8569746, 0xCDFD8A4A,
0xA8745604, 0x42C2DB14, 0x3EC84F17, 0xB98F5435, 0x5115FE06, 0xBA33CF0A, 0x3AF73C5F, 0x1C064AFE, 0xFA92F734, 0x7A3C7BF7,
0x3FF51525, 0x5BBEEB51, 0x8410F480, 0x7D49ECC5, 0xD988E730, 0xD9F97518
    };


    typedef union {
        ULONGLONG	Qword;
        DWORD		Dword[2];
        WORD		Word[4];
        BYTE		Byte[8];
    } UQWORD;
    UQWORD Tmp;
    Tmp.Dword[1] = arrDecode[7];
    Tmp.Dword[0] = arrDecode[6];

    ULONGLONG *p = (ULONGLONG*)PackData;
    ULONGLONG *pEnd = (ULONGLONG*)PackData + Idx.CompressLen / 8;
    uint8_t* parr1 = (uint8_t*)arrDecode;

    DWORD arr1Idx = (arrDecode[0x8] & 0xd) * 8;
    while (p < pEnd)
    {
        uint64_t mm6 = *(uint64_t*)(parr1 + (arr1Idx & 0xf) * 8);

        uint64_t mm5 = *(uint64_t*)((uint8_t*)anotherArrDecode + (arr1Idx & 0x7f) * 8);

        mm6 ^= mm5;

        // pxor mm7, mm6
        Tmp.Qword ^= mm6;

        // paddd mm7, mm6
        Tmp.Dword[0] += mm6 & 0xffffffff;
        Tmp.Dword[1] += mm6 >> 32;

        // movq mm0, qword ptr[edi]
        // pxor mm0, mm7
        // movq mm1, mm0
        // mov1 qword ptr [edi], mm0
        *p ^= Tmp.Qword;

        // paddb mm7, mm1   (*ppd)
        for (DWORD i = 0; i < 8; ++i)
            Tmp.Byte[i] += *((PBYTE)p + i);

        // pxor mm7, mm1
        Tmp.Qword ^= *p;

        // pslld mm7, 1
        Tmp.Dword[0] <<= 1;
        Tmp.Dword[1] <<= 1;

        // paddw mm7, mm1
        for (DWORD i = 0; i < 4; ++i)
            Tmp.Word[i] += *((PWORD)p + i);

        ++p;
        arr1Idx = (arr1Idx + 1) & 0x7f;
    }
    return 0;
}

int DecodeFileData(PVOID PackData, const PACKIDX& Idx, PBYTE pExeKey, DWORD dwExeKeyLen, PBYTE pKeyFile, DWORD dwKeyFileLen)
{
    DWORD dwSum = 0x85f532;
    DWORD dwSeed = 0x33f641;    
	DWORD arr1Idx, SubFlag;
	DWORD arrDecode[256];	// "8qH"
	
	const uint32_t dwFileNameLen = wcslen((const wchar_t*)Idx.Name);
	for (DWORD i=0; i<dwFileNameLen; ++i)
	{
        dwSum += ((uint32_t)((wchar_t*)Idx.Name)[i] << (i & 7));
		dwSeed ^= dwSum;
	}

	dwSeed += ((Idx.CompressLen ^ 0x8f32dc ^ dwSum) + dwSum + Idx.CompressLen + 7 * (Idx.CompressLen & 0xffffff)) ^ Idx.Key;
	dwSeed =  (dwSeed & 0xffffff) * 9;
	//dwSeed ^= 0x453a;


	Decode_Sub1(dwSeed, arrDecode, &SubFlag);
	
	Decode_Sub2(pKeyFile, dwKeyFileLen, arrDecode);
	Decode_Sub2(pExeKey, dwExeKeyLen, arrDecode);

    DWORD arrDecode_1[256] = { 0 };
	for (DWORD i=0; i<0x29; ++i)
		arrDecode_1[i] = Decode_Sub3(arrDecode, &SubFlag);

	arrDecode_1[0x29] = 0;
	

	typedef union {
		ULONGLONG	Qword;
		DWORD		Dword[2];
		WORD		Word[4];
		BYTE		Byte[8];
	} UQWORD;
	UQWORD Tmp;
	Tmp.Dword[0] = Decode_Sub3(arrDecode, &SubFlag);
	Tmp.Dword[1] = Decode_Sub3(arrDecode, &SubFlag);

	ULONGLONG *p     = (ULONGLONG*) PackData;
	ULONGLONG *pEnd  = (ULONGLONG*) PackData + Idx.CompressLen / 8;
	ULONGLONG *parr1 = (ULONGLONG*) arrDecode_1;

	arr1Idx = Decode_Sub3(arrDecode, &SubFlag) & 0xf;
	while (p < pEnd)
	{
		// pxor mm7, mm6
		Tmp.Qword ^= *(parr1 + arr1Idx);	
		
		// paddd mm7, mm6
		Tmp.Dword[0] += * (PDWORD)(parr1 + arr1Idx)     ;
		Tmp.Dword[1] += *((PDWORD)(parr1 + arr1Idx) + 1);

		// movq mm0, qword ptr[edi]
		// pxor mm0, mm7
		// movq mm1, mm0
		// mov1 qword ptr [edi], mm0
		*p ^= Tmp.Qword;

		// paddb mm7, mm1   (*ppd)
		for (DWORD i=0; i<8; ++i)
			Tmp.Byte[i] += *((PBYTE)p + i);

		// pxor mm7, mm1
		Tmp.Qword ^= *p;

		// pslld mm7, 1
		Tmp.Dword[0] <<= 1;
		Tmp.Dword[1] <<= 1;

		// paddw mm7, mm1
		for (DWORD i=0; i<4; ++i)
			Tmp.Word[i] += *((PWORD)p + i);

		++p;
		arr1Idx = ++arr1Idx & 0xf;
	}



	return 0;
}


int Decompress(PVOID HashData, DWORD HashDataLen, PBYTE *ppunCompressData, PDWORD punCompressDataLen)
{
	PBYTE pHash, punCompre;
	DWORD CompreLen, unCompreLen;
	BYTE Node[2][256], Child[256];

	*ppunCompressData   = 0;
	*punCompressDataLen = 0;


	if (((COMPRESSHEADER*)HashData)->Magic != 0xFF435031)
	{
		AppendMsg(L"Hash数据不匹配\r\n");
		return -3;
	}

	unCompreLen = ((COMPRESSHEADER*)HashData)->unCompressLen;
	if (NULL == (punCompre = (PBYTE)VirtualAlloc(NULL, unCompreLen, MEM_COMMIT, PAGE_READWRITE)))
	{
		AppendMsg(L"内存不足\r\n");
		return -2;
	}

	CompreLen = HashDataLen - sizeof(COMPRESSHEADER);
	pHash     = (PBYTE)HashData + sizeof(COMPRESSHEADER);


	DWORD curHash = 0, curUncom = 0, cnt;
	while (curHash < CompreLen)
	{
		for (DWORD i=0; i<256; ++i)
			Node[0][i] = (BYTE)i;

		for (DWORD i=0; i<256;)
		{
			cnt = pHash[curHash++];
		
			if (cnt > 127)
			{
				i += cnt - 127;
				cnt = 0;
			}

			if (i > 255) 
				break;

			++cnt;

			for (DWORD j=0; j<cnt; ++j, ++i)
			{
				Node[0][i] = pHash[curHash++];
			
				if (Node[0][i] != i)
					Node[1][i] = pHash[curHash++];
			}
		}

		if (((COMPRESSHEADER*)HashData)->Flag & 1)
		{
			cnt = *(PWORD)(pHash + curHash);
			curHash += 2;
		}
		else
		{
			cnt = *(PDWORD)(pHash + curHash);
			curHash += 4;
		}

		DWORD j = 0, idx;
		while (1)
		{
			if (j > 0)
				idx = Child[--j];
			else
			{
				if (!cnt)
					break;

				--cnt;
				idx = pHash[curHash++];
			}

			if (Node[0][idx] == idx)
				punCompre[curUncom++] = (BYTE)idx;
			else
			{///////////////////////////////////////////////////////////
				Child[j++] = Node[1][idx];
				Child[j++] = Node[0][idx];
			}///////////////////////////////////////////////////////////
		}
	}

	if (curUncom != ((COMPRESSHEADER*)HashData)->unCompressLen)
	{
		VirtualFree(punCompre, 0, MEM_RELEASE);
		AppendMsg(L"Hash数据解压错误\r\n");
		return -4;
	}

	*ppunCompressData   = punCompre;
	*punCompressDataLen = curUncom;
	return 0;
}


int GetPackageDirectory(const HANDLE hPack, const PACKHEADER *ph, vector<PACKIDX>& Idx)
{
	DWORD	Key, HashSize, RawIndexLen = 0, R;
	int		ret = 0;
	PBYTE	HashData, RawIndex = 0;
	BYTE	Buf[0x440];


	SetFilePointer(hPack, -(int)sizeof(Buf), 0, FILE_END);
	ReadFile(hPack, Buf, sizeof(Buf), &R, 0);

	Key = 0xfffffff & GenerateKey2(Buf + 0x24, 256);
	Decode(Buf, 32, Key);


    // 验证前 32 个字符为 "8hr48uky,8ugi8ewra4g8d5vbf5hb5s6"


	HashSize   = *(PDWORD)(Buf + 0x20);
	HashData = new BYTE[HashSize];
	if (!HashData)
	{
		AppendMsg(L"内存不足\r\n");
		return -2;
	}

	SetFilePointer(hPack, -(int)(sizeof(Buf) + HashSize), 0, FILE_END);
	ReadFile(hPack, HashData, HashSize, &R, 0);



	if (!strcmp((char*)HashData, "HashVer1.4"))
	{
		Decode(HashData + sizeof(HASHHEADER14), ((HASHHEADER13*)HashData)->DataSize, 0x428);
		ret = Decompress(HashData + sizeof(HASHHEADER14), ((HASHHEADER13*)HashData)->DataSize,
				&RawIndex, &RawIndexLen);
	}
	else if (!strcmp((char*)HashData, "HashVer1.3"))
	{
		Decode(HashData + sizeof(HASHHEADER13), ((HASHHEADER13*)HashData)->DataSize, 0x428);
		ret = Decompress(HashData + sizeof(HASHHEADER13), ((HASHHEADER13*)HashData)->DataSize,
				&RawIndex, &RawIndexLen);
	}
	else
	{
		AppendMsg(L"Hash数据不匹配\r\n");
		return -3;
	}

	
	
	if (ret)
	{
		delete[] HashData;
		return ret;
	}

	DWORD OffsetHigh = (DWORD)(ph->IndexOffset >> 32);
	SetFilePointer(hPack, (ULONG)ph->IndexOffset, (PLONG)&OffsetHigh, FILE_BEGIN);
	for (DWORD i=0; i<ph->FileNum; ++i)
	{
		WORD NameLen, j;
		PACKIDX pi;
		
		ReadFile(hPack, &NameLen, 2, &R, 0);
		assert(NameLen * 2 < _countof(PACKIDX::Name));
		ReadFile(hPack, pi.Name, NameLen * 2, &R, 0);

        DWORD tmpKey = (NameLen ^ 0x3e13 ^ (Key ^ ((Key >> 16) & 0xffff)) ^ (NameLen * NameLen)) & 0xffff;
        DWORD tmpKey2 = tmpKey;
        for (j = 0; j < NameLen; ++j)
        {
            tmpKey2 = ((tmpKey2 << 3) + j + tmpKey) & 0xffff;
            *(PWORD)(pi.Name + j * 2) ^= tmpKey2;
        }
		//NameKey = (BYTE)(NameLen + (Key ^ 0x3e));
		//for (j=0; j<NameLen; ++j)
		//	pi.Name[j] ^= ((j+1) ^ NameKey) + (j+1);
		pi.Name[NameLen * 2] = 0;
        pi.Name[NameLen * 2 + 1] = 0;

		ReadFile(hPack, &pi.Offset,        8, &R, 0);
		ReadFile(hPack, &pi.CompressLen,   4, &R, 0);
		ReadFile(hPack, &pi.unCompressLen, 4, &R, 0);
		ReadFile(hPack, &pi.IsCompressed,  4, &R, 0);
		ReadFile(hPack, &pi.IsEncrypted,   4, &R, 0);
		ReadFile(hPack, &pi.Hash,          4, &R, 0);

		pi.Key = Key;

		Idx.push_back(pi);
	}

	if (Idx.size() != ph->FileNum)
	{
		AppendMsg(L"封包索引提取失败\r\n");
		return -4;
	}
	return 0;
}


int GetKeyBuf(PBYTE *ppExeKeyBuf, PDWORD pdwExeKeyLen, PBYTE *ppKeyFile, PDWORD pdwKeyFileLen)
{
	HANDLE hExe, hKey;
	DWORD  R;
	int    ret = 0;
	PBYTE  retKey = 0, retExe = 0, pExeBuf = 0, p;


	hExe = CreateFile(g_ExePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	hKey = CreateFile(g_KeyPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hExe == INVALID_HANDLE_VALUE || hKey == INVALID_HANDLE_VALUE)
	{
		AppendMsg(L"exe或KeyFile打开失败\r\n");
		return -1;
	}


	// 处理exe
	DWORD dwExeLen = GetFileSize(hExe, 0);
	pExeBuf = (PBYTE)VirtualAlloc(NULL, dwExeLen, MEM_COMMIT, PAGE_READWRITE);
	if (!pExeBuf)
	{
		AppendMsg(L"内存分配失败\r\n");
		ret = -2;
		goto RELEASE;
	}

	ReadFile(hExe, pExeBuf, dwExeLen, &R, 0);

	for (p = pExeBuf+dwExeLen-6; p>=pExeBuf; --p)
		if (!memcmp(p, "\x05TIcon", 6))
			break;
	if (p < pExeBuf)
	{
		AppendMsg(L"无法在指定exe中找到Key信息\r\n");
		ret = -7;
		goto RELEASE;
	}

	retExe = (PBYTE)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_READWRITE);
	if (!pExeBuf)
	{
		AppendMsg(L"内存分配失败\r\n");
		ret = -2;
		goto RELEASE;
	}

	memcpy(retExe, p+6, 256);


	// 处理KeyFile
	DWORD dwKeyFileLen = GetFileSize(hKey, 0);
	retKey = (PBYTE) VirtualAlloc(NULL, dwKeyFileLen, MEM_COMMIT, PAGE_READWRITE);
	if (!pExeBuf)
	{
		AppendMsg(L"内存分配失败\r\n");
		ret = -2;
		goto RELEASE;
	}

	ReadFile(hKey, retKey, dwKeyFileLen, &R, 0);

	*ppExeKeyBuf   = retExe;
	*ppKeyFile     = retKey;
	*pdwExeKeyLen  = 256;
	*pdwKeyFileLen = dwKeyFileLen;

RELEASE:
	CloseHandle(hExe);
	CloseHandle(hKey);

	if (pExeBuf) VirtualFree(pExeBuf, 0, MEM_RELEASE);

	if (ret)
	{
		if (retExe) VirtualFree(retExe, 0, MEM_RELEASE);
		if (retKey) VirtualFree(retKey, 0, MEM_RELEASE);
		*ppExeKeyBuf = 0;
		*ppKeyFile   = 0;
		*pdwExeKeyLen = *pdwKeyFileLen = 0;
	}

	return ret;
}


int HandleDPNG(PBYTE Data, DWORD Len, const wchar_t *CurDir, const wchar_t *UniName)
{
	if (Len < 4 || *(PDWORD)Data != 0x474e5044) // "DPNG"
		return 0;

	wchar_t Format[MAX_PATH], NewName[MAX_PATH];
	int     ret = 0;

	StringCchCopy(Format, MAX_PATH, UniName);
	int k = wcslen(Format);
	while (k>0 && Format[k-1] != '.') --k;
	Format[k-1] = 0;
	StringCchCat(Format, MAX_PATH, L"+%02d+x%dy%d.png");


	DPNGHEADER *pdh = (DPNGHEADER*) Data;
	Data += sizeof(DPNGHEADER);

	for (DWORD i = 0; i < pdh->IdxNum; ++i) 
	{
		DPNGIDX* idx = (DPNGIDX*) Data;
		Data += sizeof(DPNGIDX);

		if (idx->Length) 
		{
			StringCchPrintf(NewName, MAX_PATH, Format, i, idx->Offsetx, idx->Offsety);
			if (!SplitFileNameAndSave(CurDir, NewName, Data, idx->Length))
				++ret;
		}

		Data += idx->Length;
	}

	return ret;
}


int ExtractResource(const HANDLE hPack, const vector<PACKIDX>& Idx, const wchar_t *CurDir, const wchar_t *PackName)
{
	DWORD dwunCompreLen, dwExeKeyLen, dwKeyFileLen, R;
	DWORD dwFileProcesses = 0;
	PBYTE unCompre, pExeKeyBuf, pKeyFile;
	int   ret = 0;
	wchar_t	UniName[MAX_PATH], MagBuf[MAX_PATH];


	ret = GetKeyBuf(&pExeKeyBuf, &dwExeKeyLen, &pKeyFile, &dwKeyFileLen);
	if (ret)
	{
		AppendMsg(L"Key获取失败\r\n");
		return ret;
	}

	
	for (DWORD i=0; i<Idx.size(); ++i)
	{
		PBYTE PackData = (PBYTE)VirtualAlloc(NULL, Idx[i].CompressLen * 2, MEM_COMMIT, PAGE_READWRITE);
		if (!PackData)
		{
			AppendMsg(L"内存不足\r\n");
			return -2;
		}


		//MultiByteToWideChar(932, 0, Idx[i].Name, MAX_PATH, UniName, MAX_PATH);
        wcscpy_s(UniName, _countof(UniName), (wchar_t*)Idx[i].Name);


		int err = 0;
		LONG OffsetHigh = (LONG)(Idx[i].Offset >> 32);

		err = SetFilePointer(hPack, (DWORD)Idx[i].Offset, &OffsetHigh, FILE_BEGIN);
		if (err == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
		{
			StringCchPrintf(MagBuf, MAX_PATH, L"设置文件指针失败, 跳过 %s\r\n", UniName);
			AppendMsg(MagBuf);
			goto CONTINUE;
		}

		err = ReadFile(hPack, PackData, Idx[i].CompressLen, &R, 0);
		if (!err || R != Idx[i].CompressLen)
		{
			StringCchPrintf(MagBuf, MAX_PATH, L"读取文件出错, 跳过 %s\r\n", UniName);
			AppendMsg(MagBuf);
			goto CONTINUE;
		}
		

		if (GenerateKey2(PackData, Idx[i].CompressLen) != Idx[i].Hash)
		{
			StringCchPrintf(MagBuf, MAX_PATH, L"文件Hash不匹配, 跳过 %s\r\n", UniName);
			AppendMsg(MagBuf);
			goto CONTINUE;
		}
		
		if (Idx[i].IsEncrypted == 1)
			ret = DecodeKeyFileData(PackData, Idx[i], pExeKeyBuf, dwExeKeyLen, pKeyFile, dwKeyFileLen);
        else if (Idx[i].IsEncrypted >= 2)
            ret = DecodeKeyFileData2(PackData, Idx[i], pExeKeyBuf, dwExeKeyLen, pKeyFile, dwKeyFileLen);
			
		if (Idx[i].IsCompressed)
		{
			Decompress(PackData, Idx[i].CompressLen, &unCompre, &dwunCompreLen);
			if (ret)				
				goto CONTINUE;

			VirtualFree(PackData, 0, MEM_RELEASE);
			PackData = 0;
		}
		else
		{
			unCompre		= PackData;
			dwunCompreLen	= Idx[i].CompressLen;
		}


		if (HandleDPNG(unCompre, dwunCompreLen, CurDir, UniName))
		{
			++dwFileProcesses;
			goto CONTINUE;
		}
		

		if (!SplitFileNameAndSave(CurDir, UniName, unCompre, dwunCompreLen))
			++dwFileProcesses;

CONTINUE:
		if (PackData) VirtualFree(PackData, 0, MEM_RELEASE);
		VirtualFree(unCompre, 0, MEM_RELEASE);
	}



	return dwFileProcesses;
}


int Entrance(const wchar_t *CurDir, const wchar_t *PackName)
{
	HANDLE			hPack;
	DWORD			dwFileProcesses, R;
	int				ret;
	PACKHEADER		ph;
	vector<PACKIDX>	Idx;
	wchar_t			MsgBuf[MAX_PATH];


	if (INVALID_HANDLE_VALUE == 
			(hPack = CreateFile(PackName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)))
	{
		AppendMsg(L"无法打开文件\r\n");
		return -1;
	}

	SetFilePointer(hPack, -(int)sizeof(PACKHEADER), 0, FILE_END);
	ReadFile(hPack, &ph, sizeof(ph), &R, 0);
	if (strcmp(ph.Maigc, "FilePackVer3.1"))
	{
		AppendMsg(L"封包类型不匹配\r\n");
		return -3;
	}

	// 提取索引目录
	if (ret = GetPackageDirectory(hPack, &ph, Idx))
		return ret;


	// 提取资源
	dwFileProcesses = ExtractResource(hPack, Idx, CurDir, PackName);

	if (dwFileProcesses == ph.FileNum)
	{
		StringCchPrintf(MsgBuf, MAX_PATH, L"[提取完成(%d)] %s\r\n", ph.FileNum, PackName);
		AppendMsg(MsgBuf);
	}
	else
	{
		StringCchPrintf(MsgBuf, MAX_PATH, L"[提取完成(%d/%d)] %s\r\n有%d个文件提取失败",
			dwFileProcesses, ph.FileNum, PackName, ph.FileNum - dwFileProcesses);
		AppendMsg(MsgBuf);
		MessageBox(0, MsgBuf, L"结果", MB_ICONWARNING);
	}
	return 0;
}