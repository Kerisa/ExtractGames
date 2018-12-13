
#include "xp3.h"
#include <iterator>
#include <iostream>
#include <sstream>
#include "utility/utility.h"


using namespace std;


UNCOMPRESS unCom;



DWORD HaveExtraEntryChunk(const char *game)
{
    for (int i=_countof(XP3EntryExtraChunk)-1; i>=0; --i)
        if (!strcmp(game, XP3EntryExtraChunk[i].name))
        {
            return XP3EntryExtraChunk[i].chunk;
        }

    return 0;
}

int XP3ArcPraseEntryStage1 (
        PVOID _idx,
        DWORD _len,
        std::vector<file_entry>& Entry,
        DWORD chunk
        )
{
    /////////////////////////////////////////
    // Format:
    // <magic> <length>  <adlr>   <filename>
    // 4Bytes + 8Bytes + 4Bytes + 2Bytes+wcharstring
    /////////////////////////////////////////
    if (!chunk)
        return 1;

    PBYTE pEnd = (PBYTE)_idx + _len, p = (PBYTE)_idx;
    int walk = -1;
    while (p < pEnd && *(PDWORD)p != chunk) ++p;
    for (DWORD i=0; i<Entry.size(); ++i)
        if (*(PDWORD)(p+0xc) == Entry[i].checksum)
        {
            walk = i + 1;
            wchar_t temp[128] = { 0 };
            wcscpy_s(temp, _countof(temp), (wchar_t*)(p + 0x12));
            Entry[i].file_name = temp;
            p += 0x12 + wcslen((wchar_t*)(p + 0x12));
        }

    assert(walk != -1);

    while (p < pEnd)
    {
        if (*(PDWORD)p == chunk)
        {
            wchar_t temp[128] = { 0 };
            wcscpy_s(temp, _countof(temp), (wchar_t*)(p + 0x12));
            Entry[walk++].file_name = temp;
            p += 0x12 + wcslen((wchar_t*)(p + 0x12));
        }
        else
            ++p;
    }

    return 0;
}


UNCOMPRESS EncryptedXP3::unCom;

EncryptedXP3::EncryptedXP3()
{
    if (!unCom)
    {
        HMODULE hZlib = LoadLibrary(__T("zlib.dll"));
        assert(hZlib);
        unCom = (UNCOMPRESS)GetProcAddress(hZlib, "uncompress");
    }
}

EncryptedXP3::~EncryptedXP3()
{
    Close();
}

bool EncryptedXP3::Open(const std::wstring & path)
{
    assert(!mStream.is_open());
    mStream.open(path, ios::binary);
    if (!mStream.is_open())
    {
        assert("open failed xp3 packet" && 0);
        return false;
    }

    if (!IsValid())
    {
        Close();
        assert("not valid xp3 packet" && 0);
        return false;
    }

    mPath = path;
    return true;
}

void EncryptedXP3::Close()
{
    if (mStream.is_open())
        mStream.close();
    mPath.clear();
}

bool EncryptedXP3::IsValid()
{
    vector<char> magic(sizeof(mHeader.magic));
    assert(mStream.is_open());
    mStream.seekg(0, ios::beg);
    mStream.read(magic.data(), magic.size());

    return !memcmp(magic.data(), "XP3\r\n \n\x1A\x8B\x67\x01", magic.size());
}

std::vector<char> EncryptedXP3::GetPlainIndexBytes()
{
    mStream.seekg(0, ios::beg);
    mStream.read((char*)&mHeader.magic, sizeof(mHeader.magic));
    assert(!memcmp(mHeader.magic, "XP3\r\n \n\x1A\x8B\x67\x01", sizeof(mHeader.magic)));

    mStream.read((char*)&mHeader.offset, 8);

    if (mHeader.offset != 0x17)
    {
        mStream.seekg(mHeader.offset, ios::beg);
    }
    else
    {
        mStream.read((char*)&mHeader.minor_version, 4);
        mStream.read((char*)&mHeader.flag, 1);
        mStream.read((char*)&mHeader.index_size, 8);
        mStream.read((char*)&mHeader.index_offset, 8);
        mStream.seekg(mHeader.index_offset, ios::beg);
    }

    BYTE  idx_flag;
    uint64_t idx_size;
    uint64_t idx_uncom;

    mStream.read((char*)&idx_flag, 1);
    mStream.read((char*)&idx_size, 8);
    if (idx_flag)
    {
        mStream.read((char*)&idx_uncom, 8);
    }
    else
    {
        idx_uncom = idx_size;
    }

    vector<char> idx(idx_size);
    vector<char> idx_raw(idx_uncom);

    mStream.read(idx.data(), idx.size());
    if (idx_flag)
    {
        uint32_t detLen = (uint32_t)idx_uncom;
        unCom(idx_raw.data(), &detLen, idx.data(), idx_size);
    }
    else
    {
        idx_raw = idx;
    }

    return idx_raw;
}

std::vector<file_entry> EncryptedXP3::XP3ArcPraseEntryStage0(uint32_t extraMagic, const std::vector<char>& plainBytes)
{
	static const uint32_t
		flag_file  = 0x1,
		flag_adlr  = 0x2,
		flag_segm  = 0x4,
		flag_info  = 0x8,
		flag_extra = 0x10,
		flag_all   = 0x1f,
		flag_time  = 0x80;

	std::vector<file_entry> Entry;

    uint8_t* p = (uint8_t*)plainBytes.data();
    uint8_t* pEnd = p + plainBytes.size();

    while (p < pEnd)
    {
        //////////////////////////////////////
        // 31<--4----3----2----1--->0
        //    time  info segm adlr file
        //////////////////////////////////////
        int flag = 0;
        file_entry fe;
		assert(*(uint32_t*)p == FileSection::MAGIC || *(uint32_t*)p == extraMagic);
		uint32_t dummy, entrySize;
        uint8_t* SingleEnd = ParseFileSection(p, &dummy, &entrySize) ? p + entrySize : pEnd;

        while (p < SingleEnd && flag != flag_all)
        {
            switch (*(uint32_t*)p)
            {
            default:
				if (*(uint32_t*)p == extraMagic)
				{
					assert(!(flag & flag_extra));
					uint32_t size;
					if (!ParseExtraSection(p, extraMagic, fe, &size))
					{
						assert("parse extra error" && 0);
					}
					p += size;
					flag |= flag_extra;
				}
				else
				{
					assert("error parse entry" && 0);
				}
                break;
				
			case TimeSection::MAGIC: {
				assert(!(flag & flag_time));
				uint32_t size;
				if (!ParseTimeSection(p, fe, &size))
				{
					assert("parse file error" && 0);
				}
				p += size;
				flag |= flag_time;
				break;
			}
			case FileSection::MAGIC: {
				assert(!(flag & flag_file));
				uint32_t size, entrySize;
				if (!ParseFileSection(p, &size, &entrySize))
				{
					assert("parse file error" && 0);
				}
				SingleEnd = p + entrySize;
				p += size;
				flag |= flag_file;
				break;
			}

			case AdlrSection::MAGIC: {
				assert(!(flag & flag_adlr));
				uint32_t size;
				if (!ParseAdlrSection(p, fe, &size))
				{
					assert("parse adlr error" && 0);
				}
				p += size;
				flag |= flag_adlr;
				break;
			}

            case SegmSection::MAGIC: {
				assert(!(flag & flag_segm));
				uint32_t size;
				if (!ParseSegmSection(p, fe, &size))
				{
					assert("parse segm error" && 0);
				}
				p += size;
                flag |= flag_segm;
                break;
            }

            case InfoSection::MAGIC: {
                assert(!(flag & flag_info));
				uint32_t size;
				if (!ParseInfoSection(p, fe, &size))
				{
					assert("parse info error" && 0);
				}
				p += size;
                flag |= flag_info;
                break;
            }
            }
        }   // end while (p < pEnd && flag != flag_all)

		const wstring STARTUP{ L"startup.tjs" };
		const wstring PROTECT{ L"$$$ This is a protected archive. $$$" };

		if (extraMagic != 0)
		{
			if (fe.internal_name != STARTUP && fe.internal_name.find(PROTECT) == wstring::npos)
			{
				assert(fe.mExtra.Checksum == fe.checksum);
				assert((flag & flag_all) == flag_all);
			}
			else
			{
				assert((flag & flag_all) == (flag_all & ~flag_extra));
			}
		}
		else
		{
			assert((flag & flag_all) == (flag_all & ~flag_extra));
		}

		if (fe.file_name.empty())
			fe.file_name = fe.internal_name;

		if (fe.file_name.find(PROTECT) == wstring::npos)
			Entry.push_back(fe);
    }

    return Entry;
}

bool EncryptedXP3::ParseFileSection(const uint8_t * ptr, uint32_t * secSize, uint32_t* entrySize)
{
	FileSection* file = (FileSection*)ptr;
	if (file->Magic != FileSection::MAGIC)
		return false;

	*secSize = sizeof(FileSection);
	*entrySize = sizeof(FileSection) + file->SizeOfData;
	return true;
}

bool EncryptedXP3::ParseSegmSection(const uint8_t * ptr, file_entry& fe, uint32_t * secSize)
{
	const uint8_t* old = ptr;
	const SegmSection* segm = (const SegmSection*)ptr;
	if (segm->Magic != SegmSection::MAGIC)
		return false;

	assert(segm->SizeOfData % 0x1c == 0);
	int part = segm->SizeOfData / 0x1c;
	fe.mInfo.push_back(*segm);
	ptr += sizeof(SegmSection);
	if (part > 1)
	{
		SegmSection ss;
		ss.Magic = SegmSection::MAGIC;
		ss.SizeOfData = 0x1c;
		ss.IsCompressed = *(uint32_t*)ptr;
		ptr += 4;
		ss.Offset = *(uint64_t*)ptr;
		ptr += 8;
		ss.OriginalSize = *(uint64_t*)ptr;
		ptr += 8;
		ss.PackedSize = *(uint64_t*)ptr;
		ptr += 8;
		fe.mInfo.push_back(ss);
	}
	*secSize = ptr - old;
	return true;
}

bool EncryptedXP3::ParseInfoSection(const uint8_t * ptr, file_entry& fe, uint32_t * secSize)
{
	const InfoSection* info = (const InfoSection*)ptr;
	if (info->Magic != InfoSection::MAGIC)
		return false;

	fe.encryption_flag = info->EncryptFlag;
	fe.internal_name.assign(info->NamePtr, info->NamePtr + info->NameInWords);
	*secSize = info->SizeOfData + 0xc;
	return true;
}

bool EncryptedXP3::ParseAdlrSection(const uint8_t * ptr, file_entry& fe, uint32_t * secSize)
{
	const AdlrSection* adlr = (const AdlrSection*)ptr;
	if (adlr->Magic != AdlrSection::MAGIC)
		return false;

	fe.checksum = adlr->Checksum;
	*secSize = sizeof(AdlrSection);
	return true;
}

bool EncryptedXP3::ParseTimeSection(const uint8_t * ptr, file_entry & fe, uint32_t * secSize)
{
	const TimeSection* time = (const TimeSection*)ptr;
	if (time->Magic != TimeSection::MAGIC)
		return false;

	fe.mFileTime = time->Time;
	*secSize = sizeof(TimeSection);
	return true;
}

bool EncryptedXP3::ParseExtraSection(const uint8_t * ptr, uint32_t extraMagic, file_entry & fe, uint32_t * secSize)
{
	const ExtraSection* ex = (const ExtraSection*)ptr;
	if (ex->Magic != extraMagic)
		return false;

	*secSize = ex->SizeOfData + 0xc;
	fe.mExtra = *ex;
	fe.file_name = ex->NamePtr;
	assert(fe.file_name.size() == ex->NameInWords);
	return true;
}

bool EncryptedXP3::ParseProtectWarning(const uint8_t * ptr, uint32_t * secSize)
{
	if (*(uint32_t*)ptr == FileSection::MAGIC && *(uint64_t*)(ptr + 4) == 0x324)
	{
		*secSize = 0x324 + 0xc;
		return true;
	}

	return false;
}

bool EncryptedXP3::HasExtraSection(const std::vector<char>& plainBytes, uint32_t * magic)
{
	uint32_t size;
	const uint8_t* ptr = (const uint8_t*)plainBytes.data();

	if (ParseProtectWarning(ptr, &size))
		ptr += size;

	if (*(uint32_t*)ptr == FileSection::MAGIC)
	{
		bool succcess = true;
		int skipCount = 4;		// 类似 startup.tjs 没有额外节，随便跳过几个
		uint32_t entrySize;
		while (skipCount--)
		{
			uint32_t dummy;
			if (ParseFileSection(ptr, &dummy, &entrySize))
			{
				ptr += entrySize;
				continue;
			}
			else
			{
				const uint8_t* pEnd = (const uint8_t*)plainBytes.data() + plainBytes.size();
				while (ptr < pEnd)
				{
					switch (*(uint32_t*)ptr)
					{
					case AdlrSection::MAGIC: ParseAdlrSection(ptr, file_entry(), &size); ptr += size; break;
					case TimeSection::MAGIC: ParseTimeSection(ptr, file_entry(), &size); ptr += size; break;
					case SegmSection::MAGIC: ParseSegmSection(ptr, file_entry(), &size); ptr += size; break;
					case InfoSection::MAGIC: ParseInfoSection(ptr, file_entry(), &size); ptr += size; break;
					case FileSection::MAGIC: ParseFileSection(ptr, &size, &entrySize);   pEnd = ptr + entrySize; ptr += size; break;
					default: {
						const ExtraSection* ex = (const ExtraSection*)ptr;
						if (wcslen(ex->NamePtr) != ex->NameInWords)
						{
							assert("???" && 0);
						}
						*magic = ex->Magic;
						return true;
					}
					}
				}
			}
		}
		return false;
	}
	else
	{
		const ExtraSection* ex = (const ExtraSection*)ptr;
		if (wcslen(ex->NamePtr) != ex->NameInWords)
		{
			assert("???" && 0);
		}
		*magic = ex->Magic;
		return true;
	}
}

std::vector<file_entry> EncryptedXP3::ParsePalette_9nine(const std::vector<char>& plainBytes)
{
	std::vector<file_entry> nameList;

	uint8_t* p = (uint8_t*)plainBytes.data();
	uint8_t* pEnd = p + plainBytes.size();

	assert(*(uint32_t*)p == MagicHnfn);
	p += *(uint32_t*)(p + 4) + 0xc;  // skip protection warning

	while (*(uint32_t*)p == MagicHnfn)
	{
		uint64_t length = *(uint64_t*)(p + 4);
		uint32_t checksum = *(uint32_t*)(p + 12);
		uint32_t nameLen = *(uint16_t*)(p + 16);
		std::wstring name = (wchar_t*)(p + 18);
		p += length + 12;

		file_entry fe;
		fe.checksum = checksum;
		fe.file_name = name;
		nameList.push_back(fe);
	}

	std::vector<file_entry> Entry = XP3ArcPraseEntryStage0(0, std::vector<char>((char*)p, (char*)pEnd));
	assert(Entry.size() >= nameList.size());
	size_t n = 0;
	for (size_t i = 0; i < Entry.size(); ++i)
	{
		if (Entry[i].checksum != nameList[n].checksum)
			continue;
		Entry[i].file_name = nameList[n].file_name;
		++n;
	}

	assert(n == nameList.size());
	return Entry;
}

void EncryptedXP3::DumpEntriesToFile(const std::vector<file_entry>& entries, const std::wstring & path)
{
    ofstream out(path, ios::binary);
    if (!out.is_open())
    {
        wcout << L"DumpEntries: create file [" << path << L"] failed.\n";
        return;
    }

    for (size_t i = 0; i < entries.size(); ++i)
    {
        string str;
        str += "file name: ";

        int n = WideCharToMultiByte(CP_UTF8, NULL, entries[i].file_name.c_str(), -1, NULL, NULL, NULL, NULL);
        vector<char> mcb(n + 1);
        WideCharToMultiByte(CP_UTF8, NULL, entries[i].file_name.c_str(), -1, mcb.data(), n, NULL, NULL);
        str += mcb.data();

        str += "\ninternal name: ";
        n = WideCharToMultiByte(CP_UTF8, NULL, entries[i].internal_name.c_str(), -1, NULL, NULL, NULL, NULL);
        mcb.resize(n + 1);
        WideCharToMultiByte(CP_UTF8, NULL, entries[i].internal_name.c_str(), -1, mcb.data(), n, NULL, NULL);
        str += mcb.data();


        stringstream s0;
        s0 << "\nchecksum: " << std::hex << entries[i].checksum << "\nencryption_flag: " << std::hex << entries[i].encryption_flag << "\n";
        str += s0.str();

        for (size_t k = 0; k < entries[i].mInfo.size(); ++k)
        {
            stringstream ss;
            ss << "parts " << k + 1 << ":\n    ";
            ss << "offset: " << std::hex << entries[i].mInfo[k].Offset << "\n    ";
            ss << "orignal: " << std::hex << entries[i].mInfo[k].OriginalSize << "\n    ";
            ss << "packet: " << std::hex << entries[i].mInfo[k].PackedSize << "\n    ";
            ss << "compress_flag: " << std::hex << entries[i].mInfo[k].IsCompressed << "\n";
            str += ss.str();
        }

        str += "---------------------------------------------\n\n";
        out.write(str.c_str(), str.size());
    }

    out.close();
}

std::vector<file_entry> EncryptedXP3::ExtractEntries(const std::vector<char>& plainBytes)
{
	uint32_t magic;
	if (HasExtraSection(plainBytes, &magic))
		mExtraSectionMagic = magic;

	switch (*(uint32_t*)plainBytes.data())
	{
	case MagicHnfn:
		return ParsePalette_9nine(plainBytes);
	default:
		return XP3ArcPraseEntryStage0(mExtraSectionMagic, plainBytes);
	}
}

bool EncryptedXP3::ReadEntryDataOfAllParts(const file_entry & fe, vector<char>& packedData, uint32_t* pOriginalLength)
{
    fe.ReadFileData(mStream, packedData);
    *pOriginalLength = fe.GetTotleOriginalSize();
    return true;
}

int EncryptedXP3::ExtractData(const std::vector<file_entry>& Entry, const std::wstring& saveDir, wostream& output)
{
    uint32_t cnt_savefile = 0;

    for (const file_entry& fe : Entry)
    {
        vector<char> cipher;
        uint32_t file_org_len = 0;
        ReadEntryDataOfAllParts(fe, cipher, &file_org_len);

        vector<char> unpack(file_org_len);
        if (fe.mInfo[0].IsCompressed)
        {
            uint32_t unpack_len = (uint32_t)file_org_len;
            unCom(unpack.data(), &unpack_len, cipher.data(), cipher.size());
        }
        else
        {
            assert(cipher.size() == file_org_len);
            unpack = cipher;
        }


        if (DoExtractData(fe, unpack))
        {
            if (!SplitFileNameAndSave(saveDir.c_str(), fe.file_name, unpack))
            {
                ++cnt_savefile;
                output << wstring(wstring(L"[已保存]") + fe.file_name + L"\r\n");
            }
            else
            {
                output << wstring(wstring(L"[无法保存]") + fe.file_name + L"\r\n");
            }
        }
    }

    return cnt_savefile;
}

bool EncryptedXP3::DoExtractData(const file_entry& fe, std::vector<char>& unpackData)
{
    // 默认
    return true;
}

int EncryptedXP3::SplitFileNameAndSave(const wstring& cur_dir, const wstring& file_name, const vector<char>& unpackData)
{
    DWORD ByteWrite;
    wstring buf;

    buf = cur_dir + L"\\" + file_name;

    int len = buf.size();
    int i = cur_dir.size() + 1;
    const wchar_t* p = buf.c_str();
    const wchar_t* end = buf.c_str() + len;
    while (p <= end && i < len)
    {
        while (buf[i] != '\\' && buf[i] != '/' && buf[i] != '\0') ++i;
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

    wstring buf2;
    HANDLE hFile;
    int ret = 0;
    do {
        hFile = CreateFileW(buf.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
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

std::wstring EncryptedXP3::FormatFileNameForIStream(const file_entry & fe) const
{
    wstring file, ext;
    Utility::SplitPath(mPath, wstring(), wstring(), file, ext);
	if (mExtraSectionMagic != 0)
		return L"archive://" + file + ext + L"/" + fe.file_name;

    return mPath + L">" + fe.file_name;
}





std::vector<file_entry> palette_9_nine::XP3ArcPraseEntryStage0(const std::vector<char>& plainBytes)
{
    std::vector<file_entry> Entry;
    std::vector<file_entry> nameList;


    static const DWORD _file = 0x656C6946, _adlr = 0x726c6461,
        _segm = 0x6d676573, _info = 0x6f666e69, _hnfn = 0x6e666e68,
        flag_file = 0x1, flag_adlr = 0x2,
        flag_segm = 0x4, flag_info = 0x8,
        flag_all = 0xf;
	assert(0);
    PBYTE p = (PBYTE)plainBytes.data();
    PBYTE pEnd = p + plainBytes.size();

    assert(*(PDWORD)p == _hnfn);
    p += *(PDWORD)(p + 4) + 0xc;  // skip protection warning

    while (*(PDWORD)p == _hnfn)
    {
        uint64_t length = *(uint64_t*)(p + 4);
        uint32_t checksum = *(uint32_t*)(p + 12);
        uint32_t nameLen = *(uint16_t*)(p + 16);
        std::wstring name = (wchar_t*)(p + 18);
        p += length + 12;

        file_entry fe;
        fe.checksum = checksum;
        fe.file_name = name;
        nameList.push_back(fe);
    }

    while (p < pEnd)
    {
        //////////////////////////////////////
        // 31<-------3----2----1--->0
        //          info segm adlr file
        //////////////////////////////////////
        int flag = 0;
        file_entry fe;
        memset(&fe, 0, sizeof(fe));
        PBYTE SingleEnd = pEnd;

        while (p < SingleEnd && flag != flag_all)
        {
            switch (*(PDWORD)p)
            {
            default:
                ++p;
                break;

            case _file:
                assert(!(flag & flag_file));
                SingleEnd = p + *(PDWORD)(p + 4) + 0xc;
                p += 0xc;
                flag |= flag_file;
                break;

            case _adlr:
                assert(!(flag & flag_adlr));
                p += 0xc;
                fe.checksum = *((PDWORD)p);
                p += 4;
                flag |= flag_adlr;
                break;

            case _segm: {
                SegmSection* segm = (SegmSection*)p;
                assert(!(flag & flag_segm));
                assert(segm->SizeOfData % 0x1c == 0);
                int part = segm->SizeOfData / 0x1c;
                fe.mInfo.push_back(*segm);
                p += sizeof(SegmSection);
                if (part > 1)
                {
                    SegmSection ss;
                    ss.Magic = SegmSection::MAGIC;
                    ss.SizeOfData = 0x1c;
                    ss.IsCompressed = *(uint32_t*)p;
                    p += 4;
                    ss.Offset = *(uint64_t*)p;
                    p += 8;
                    ss.OriginalSize = *(uint64_t*)p;
                    p += 8;
                    ss.PackedSize = *(uint64_t*)p;
                    p += 8;
                    fe.mInfo.push_back(ss);
                }

                flag |= flag_segm;
                break;
            }
            case _info: {
                assert(!(flag & flag_info));
                PBYTE info_sec_end = p + 0xc + *((PDWORD)(p + 0x4));
                p += 0xc;
                fe.encryption_flag = *((PDWORD)p);    // 好像这个标志也没啥用
                p += 0x14;  // 跳过info中的长度信息

                            // 剩下的是文件名长度和文件名
                int buf_size = (int)*((PWORD)p);
                p += 0x2;
                fe.internal_name.assign((wchar_t*)p, (wchar_t*)p + buf_size);

                p = info_sec_end;

                flag |= flag_info;
                break;
            }
            }
        }   // end while (p < pEnd && flag != flag_all)

        assert(flag == flag_all);
        if (fe.file_name.empty())
            fe.file_name = fe.internal_name;
        Entry.push_back(fe);
    }

    assert(Entry.size() >= nameList.size());
    size_t n = 0;
    for (size_t i = 0; i < Entry.size(); ++i)
    {
        if (Entry[i].checksum != nameList[n].checksum)
            continue;
        Entry[i].file_name = nameList[n].file_name;
        ++n;
    }

    assert(n == nameList.size());
    return Entry;
}

uint32_t file_entry::GetTotlePackedSize() const
{
    uint32_t file_pkg_len = 0;
    for (size_t i = 0; i < mInfo.size(); ++i)
    {
        file_pkg_len += (uint32_t)mInfo[i].PackedSize;
    }
    return file_pkg_len;
}

uint32_t file_entry::GetTotleOriginalSize() const
{
    uint32_t file_org_len = 0;
    for (size_t i = 0; i < mInfo.size(); ++i)
    {
        file_org_len += (uint32_t)mInfo[i].OriginalSize;
    }
    return file_org_len;
}

bool file_entry::IsCompressed() const
{
    assert(!mInfo.empty());
    return !!mInfo[0].IsCompressed;
}

bool file_entry::IsEncrypted() const
{
    return !!encryption_flag;
}

bool file_entry::ReadFileData(std::ifstream & file, std::vector<char>& packedData) const
{
    uint32_t file_pkg_len = GetTotlePackedSize();
    uint32_t file_read = 0;
    packedData.resize(file_pkg_len);

    for (size_t i = 0; i < mInfo.size(); ++i)
    {
        file.seekg(mInfo[i].Offset, ios::beg);
        file.read(packedData.data() + file_read, (uint32_t)mInfo[i].PackedSize);
        file_read += (uint32_t)mInfo[i].PackedSize;
    }

    return true;
}