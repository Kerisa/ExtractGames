
#include "xp3.h"
#include <array>
#include <iterator>
#include <iostream>
#include <sstream>
#include "utility/utility.h"


using namespace std;

namespace YuzuKeys
{
    struct Keys
    {
        uint32_t dword_0;
        uint32_t dword_4;
        std::array<uint8_t, 64> arr_8;
        std::array<uint8_t, 32> arr_0;
    };

    constexpr uint32_t riddle_joker_dword_1001F5E0 = 0xbdd72518;
    constexpr uint32_t riddle_joker_dword_1001F5E4 = 0xd541d24c;
    constexpr std::array<uint8_t, 64> riddle_joker_arr_1001F5E8 = {
        0xBE, 0xED, 0x7E, 0x41, 0x44, 0x4C, 0xE2, 0x6C, 0x2E, 0x97, 0x83, 0x0C, 0xCF, 0x01, 0xF4, 0x26,
        0xE9, 0xCF, 0x1D, 0x66, 0x51, 0xE4, 0xE8, 0xDA, 0x4E, 0xA1, 0xEF, 0x0B, 0xBE, 0x0D, 0x70, 0xDB,
        0xDF, 0xBB, 0xC4, 0x5B, 0x9F, 0xCB, 0xFE, 0xE2, 0xCE, 0xD6, 0xCC, 0xC6, 0xC0, 0x00, 0x45, 0x9B,
        0x1A, 0x3E, 0x27, 0xCE, 0xC1, 0x94, 0x96, 0x51, 0xCD, 0x55, 0x08, 0x02, 0xE1, 0x01, 0xD5, 0xCC
    };
    constexpr std::array<uint8_t, 32> riddle_joker_arr_10018780 = {
        0x9A, 0x87, 0x8F, 0x9E, 0x91, 0x9B, 0xDF, 0xCC, 0xCD, 0xD2, 0x9D, 0x86, 0x8B, 0x9A, 0xDF, 0x94,
        0xC0, 0xFC, 0x00, 0x10, 0x30, 0x3A, 0x73, 0x0C, 0x1C, 0x2A, 0xCE, 0x11, 0xAD, 0xE5, 0x00, 0xAA
    };


    constexpr uint32_t stella_dword_1001F5E0 = 0x27b9123c;
    constexpr uint32_t stella_dword_1001F5E4 = 0x80724e1c;
    constexpr std::array<uint8_t, 64> stella_arr_1001F5E8 = {
        0xFA, 0xF1, 0x6D, 0xF5, 0x4B, 0x96, 0x51, 0x2E, 0x67, 0x2E, 0xE2, 0xB3, 0xE4, 0x58, 0x18, 0x8C,
        0xD6, 0x30, 0x54, 0x29, 0xF5, 0xDC, 0x9F, 0xA3, 0x45, 0xF1, 0xBD, 0xFA, 0x25, 0x6B, 0x7B, 0xB4,
        0xD5, 0x9E, 0xDC, 0x97, 0x7A, 0x8F, 0xD9, 0xAE, 0x81, 0x6C, 0xC4, 0x90, 0xE8, 0xF3, 0x5D, 0xFA,
        0x6E, 0xB6, 0xE5, 0x1C, 0xDE, 0xB2, 0xBC, 0x3C, 0x05, 0x4A, 0xD7, 0x73, 0xA2, 0x88, 0x2A, 0x8A
    };
    constexpr std::array<uint8_t, 32> stella_arr_10018780 = {
        0x9A, 0x87, 0x8F, 0x9E, 0x91, 0x9B, 0xDF, 0xCC, 0xCD, 0xD2, 0x9D, 0x86, 0x8B, 0x9A, 0xDF, 0x94,
        0xE0, 0xFC, 0x00, 0x10, 0x30, 0x3A, 0x73, 0x0C, 0x1C, 0x2A, 0xCE, 0x11, 0xAD, 0xE5, 0x00, 0xAA
    };

    std::array<Keys, 2> AllSupportKeys = {
        Keys{ riddle_joker_dword_1001F5E0, riddle_joker_dword_1001F5E4, riddle_joker_arr_1001F5E8, riddle_joker_arr_10018780 },     // RIDDLE JOKER
        Keys{ stella_dword_1001F5E0,       stella_dword_1001F5E4,       stella_arr_1001F5E8,       stella_arr_10018780       },     // 喫茶ステラと死神の蝶
    };

    DWORD ROL(DWORD x, int n)
    {
        return (x << n) | (x >> (32 - n));
    }

    int sub_1000F8D0(int a1, int a2, int a3)
    {
        int v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
        int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, v48, v50, v52, v54, v55, v56, v57;
        char *v51, *v53;

        int v58[0x50];
        int* const v59 = &v58[0x3];
        int* const v60 = &v58[0x4];
        int* const v61 = &v58[0x5];
        int* const v62 = &v58[0x6];
        int* const v63 = &v58[0x7];
        int* const v64 = &v58[0x8];
        int* const v65 = &v58[0x9];
        int* const v66 = &v58[0xa];
        int* const v67 = &v58[0xb];
        int* const v68 = &v58[0xc];
        int* const v69 = &v58[0xd];
        int* const v70 = &v58[0xe];
        int* const v71 = &v58[0xf];
        int* const v72 = &v58[0x10];
        int* const v73 = &v58[0x11];
        int* const v74 = &v58[0x12];
        unsigned int v75;

        v3 = a2;
        v4 = 0;
        do
        {
            *(v59 + v4) = ~*(int *)((char *)v59 + 4 * v4 + a2 - (DWORD)v59);
            ++v4;
        } while (v4 < 16);
        if (a3 > 0)
        {
            v5 = *v71;
            v6 = *v67;
            v7 = *v63;
            v8 = *v59;
            v75 = ((unsigned int)(a3 - 1) >> 1) + 1;
            do
            {
                v9 = v7 + v8;
                v10 = ROL(v9 ^ v5, 16);
                v11 = v10 + v6;
                v12 = ROL(v7 ^ v11, 12);
                v13 = v12 + v9;
                v14 = ROL(v13 ^ v10, 8);
                v15 = v14 + v11;
                *v71 = v14;
                *v63 = ROL(v12 ^ v15, 7);
                v16 = ROL((*v64 + *v60) ^ *v72, 16);
                v17 = v16 + *v68;
                v18 = ROL(*v64 ^ (v16 + *v68), 12);
                v19 = v18 + *v64 + *v60;
                v20 = ROL(v19 ^ v16, 8);
                *v72 = v20;
                *v68 = v20 + v17;
                *v64 = ROL(v18 ^ (v20 + v17), 7);
                v21 = ROL((*v65 + *v61) ^ *v73, 16);
                v22 = v21 + *v69;
                *v69 = v22;
                v23 = ROL(*v65 ^ v22, 12);
                v24 = v23 + *v65 + *v61;
                *v73 = ROL(v24 ^ v21, 8);
                *v65 = ROL(v23 ^ (*v73 + *v69), 7);
                *v69 += *v73;
                v25 = (*v66 + *v62) ^ *v74;
                *v62 += *v66;
                v25 = ROL(v25, 16);
                v26 = v25 + *v70;
                *v70 = v26;
                v27 = ROL(*v66 ^ v26, 12);
                v28 = (v27 + *v62) ^ v25;
                *v62 += v27;
                v28 = ROL(v28, 8);
                v29 = v28 + *v70;
                *v70 = v29;
                v30 = ROL(v27 ^ v29, 7);
                v31 = *v64 + v13;
                v32 = *v65 + v19;
                v33 = ROL(v31 ^ v28, 16);
                v34 = v33 + *v69;
                *v69 = v34;
                v35 = ROL(*v64 ^ v34, 12);
                v8 = v35 + v31;
                *v64 = v35;
                v36 = ROL(v8 ^ v33, 8);
                v37 = v36 + *v69;
                *v74 = v36;
                *v69 = v37;
                v38 = v30 + v24;
                *v71 = ROL(*v71 ^ v32, 16);
                *v64 = ROL(*v64 ^ v37, 7);
                v39 = *v65 ^ (*v71 + *v70);
                *v70 += *v71;
                v39 = ROL(v39, 12);
                *v60 = v39 + v32;
                v5 = ROL(*v71 ^ (v39 + v32), 8);
                *v70 += v5;
                *v65 = ROL(v39 ^ *v70, 7);
                v40 = ROL(*v72 ^ v38, 16);
                v41 = v40 + v15;
                v42 = ROL(v41 ^ v30, 12);
                v43 = v42 + v38;
                *v61 = v43;
                v6 = ROL(v40 ^ v43, 8) + v41;
                *v72 = ROL(v40 ^ v43, 8);
                v44 = ROL(*v73 ^ (*v63 + *v62), 16);
                *v66 = ROL(v6 ^ v42, 7);
                *v71 = v5;
                v45 = ROL(*v63 ^ (v44 + *v68), 12);
                v46 = v45 + *v63 + *v62;
                *v62 = v46;
                v47 = ROL(v44 ^ v46, 8);
                v48 = v47 + v44 + *v68;
                *v68 = v48;
                v7 = ROL(v45 ^ v48, 7);
                *v73 = v47;
                *v63 = v7;
                --v75;
            } while (v75);
            v3 = a2;
            *v67 = v6;
            *v59 = v8;
        }
        int result = a1;
        v50 = 0;
        v51 = (char *)v59 - v3;
        v52 = v3 + 8;
        v53 = (char *)v60 - v3;
        do
        {
            v54 = *(v59 + v50) + ~*(uint32_t *)(v52 - 8);
            v50 += 4;
            *(uint16_t *)result = v54;
            *(uint8_t *)(result + 2) = (uint8_t)(((uint32_t)v54) >> 16);// BYTE2(v54);
            *(uint8_t *)(result + 3) = (uint8_t)(((uint32_t)v54) >> 24);// HIBYTE(v54);
            v55 = v58[v50] + ~*(uint32_t *)(v52 - 4);
            v52 += 16;
            *(uint16_t *)(result + 4) = v55;
            *(uint8_t *)(result + 6) = (uint8_t)(((uint32_t)v55) >> 16);// BYTE2(v55);
            *(uint8_t *)(result + 7) = (uint8_t)(((uint32_t)v55) >> 24);// HIBYTE(v55);
            v56 = *(uint32_t *)&v51[v52 - 16] + ~*(uint32_t *)(v52 - 16);
            result += 16;
            *(uint16_t *)(result - 8) = *(uint16_t *)&v51[v52 - 16] + ~*(uint16_t *)(v52 - 16);
            *(uint8_t *)(result - 6) = (uint8_t)(((uint32_t)v56) >> 16);// BYTE2(v56);
            *(uint8_t *)(result - 5) = (uint8_t)(((uint32_t)v56) >> 24);// HIBYTE(v56);
            v57 = *(uint32_t *)&v53[v52 - 16] + ~*(uint32_t *)(v52 - 12);
            *(uint16_t *)(result - 4) = *(uint16_t *)&v53[v52 - 16] + ~*(uint16_t *)(v52 - 12);
            *(uint8_t *)(result - 2) = (uint8_t)(((uint32_t)v57) >> 16);// BYTE2(v57);
            *(uint8_t *)(result - 1) = (uint8_t)(((uint32_t)v57) >> 24);// HIBYTE(v57);
        } while (v50 < 16);
        return result;
    }

    void sub_1000F6F0(int *a1, uint8_t *a2, uint8_t *a3, int a4, int a5, int a6, int a7)
    {
        a1[0]  = a3[0]    | ((a3[1]  | (*((uint16_t *)a3 +  1) << 8)) << 8);
        a1[1]  = a3[4]    | ((a3[5]  | (*((uint16_t *)a3 +  3) << 8)) << 8);
        a1[2]  = a3[8]    | ((a3[9]  | (*((uint16_t *)a3 +  5) << 8)) << 8);
        a1[3]  = a3[12]   | ((a3[13] | (*((uint16_t *)a3 +  7) << 8)) << 8);

        a1[4]  = ~(a2[0]  | ((a2[1]  | (*((uint16_t *)a2 +  1) << 8)) << 8));
        a1[5]  = ~(a2[4]  | ((a2[5]  | (*((uint16_t *)a2 +  3) << 8)) << 8));
        a1[6]  = ~(a2[8]  | ((a2[9]  | (*((uint16_t *)a2 +  5) << 8)) << 8));
        a1[7]  = ~(a2[12] | ((a2[13] | (*((uint16_t *)a2 +  7) << 8)) << 8));
        a1[8]  = ~(a2[16] | ((a2[17] | (*((uint16_t *)a2 +  9) << 8)) << 8));
        a1[9]  = ~(a2[20] | ((a2[21] | (*((uint16_t *)a2 + 11) << 8)) << 8));
        a1[10] = ~(a2[24] | ((a2[25] | (*((uint16_t *)a2 + 13) << 8)) << 8));
        a1[11] = ~(a2[28] | ((a2[29] | (*((uint16_t *)a2 + 15) << 8)) << 8));

        a1[12] = ~a6;
        a1[13] = ~a7;
        a1[14] = ~a4;
        a1[15] = ~a5;
    }

    int Entrance(uint8_t *a1, uint8_t *a2, unsigned int a3, const Keys& key)
    {
        uint8_t *v3;
        uint32_t v4;
        uint8_t *v5;
        int64_t v6;
        uint32_t v7;
        int64_t v15;
        uint32_t v16;
        char v17[0x40];
        int64_t* const v18 = (int64_t*)&v17[0x30];
        char v20[0x80];
        char* const v21 = &v20[0x3d];
        char* const v22 = &v20[0x3e];
        char* const v23 = &v20[0x3f];
        char* const v24 = &v20[0x40];

        v3 = a2;
        sub_1000F6F0((int*)&v20, (uint8_t*)key.arr_8.data(), (uint8_t*)key.arr_0.data(), key.dword_0, key.dword_4, 0, 0);
        v4 = a3;
        v5 = a1;
        v6 = 0;
        v15 = 0;
        if (a3 >= 0x40)
        {
            v16 = a3 >> 6;
            do
            {
                memcpy(&v17, &v20, 0x40u);
                *v18 = ~v6;
                sub_1000F8D0((int)v24, (int)&v17[0], 8);
                v7 = 0;
                do
                {
                    v3[0] = v5[0] ^ v24[v7];
                    v7 += 4;
                    v3[1] = v5[1] ^ v21[v7];
                    v3[2] = v5[2] ^ v22[v7];
                    v3[3] = v5[3] ^ v23[v7];
                    v5 += 4;
                    v3 += 4;
                } while (v7 < 0x40);
                a3 -= 64;
                v6 = ++v15;
            } while (v16-- != 1);
            v4 = a3;
        }
        if (v4)
        {
            memcpy(&v17, &v20, 0x40u);
            *v18 = ~v6;
            sub_1000F8D0((int)v24, (int)&v17[0], 8);
            v6 &= 0xffffffff00000000ull;        // LODWORD(v6) = 0;
            if (a3)
            {
                do
                {
                    v6 &= (0xffffffff00000000ull & (v6 + 1)); // LODWORD(v6) = v6 + 1;
                    *v3++ = *v5 ^ v24[v6];
                    ++v5;
                } while ((unsigned int)v6 < a3);
            }
        }
        return v6;
    }
}


UNCOMPRESS unCom;


const wstring STARTUP{ L"startup.tjs" };
const wstring PROTECT{ L"$$$ This is a protected archive. $$$" };



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
    static const uint32_t InvalidExtraMagic = 0;
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
                if (*(uint32_t*)p == extraMagic && extraMagic != InvalidExtraMagic)
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
                    return Entry;
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


        if (extraMagic != InvalidExtraMagic)
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
    for (int i = 0; i < part - 1; ++i)
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
        int skipCount = 4;        // 类似 startup.tjs 没有额外节，随便跳过几个
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

std::vector<char> EncryptedXP3::ExtractYuzuFileTable(const std::vector<char>& packedFileTable, size_t plainSize)
{
    vector<char> plain(plainSize);
    for (size_t i = 0; i < YuzuKeys::AllSupportKeys.size(); ++i)
    {
        auto copy = packedFileTable;
        YuzuKeys::Entrance((uint8_t*)copy.data(), (uint8_t*)copy.data(), 0x100, YuzuKeys::AllSupportKeys[i]);
        int result = unCom(plain.data(), &plainSize, (char*)copy.data(), copy.size());
        if (result == 0)
        {
            return plain;
        }
    }
    return vector<char>();
}

std::vector<file_entry> EncryptedXP3::ParsePalette_9nine(const std::vector<char>& plainBytes)
{
    std::vector<file_entry> nameList;

    uint8_t* p = (uint8_t*)plainBytes.data();
    uint8_t* pEnd = p + plainBytes.size();

    ExtraSection* es = (ExtraSection*)p;
    assert(es->Magic == MagicHnfn);
    while (es->Magic == MagicHnfn)
    {
        file_entry fe;
        fe.checksum = es->Checksum;
        fe.file_name.assign(es->NamePtr, es->NamePtr + es->NameInWords);
        nameList.push_back(fe);

        p += es->SizeOfData + 0xc;
        es = (ExtraSection*)p;
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

std::vector<file_entry> EncryptedXP3::ParsePalette_NekoparaEx(const std::vector<char>& plainBytes)
{
    std::vector<file_entry> nameList;

    uint8_t* p = (uint8_t*)plainBytes.data();
    uint8_t* pEnd = p + plainBytes.size();

    ExtraSection* es = (ExtraSection*)p;
    assert(es->Magic == MagicNeko);
    while (es->Magic == MagicNeko)
    {
        file_entry fe;
        fe.checksum = es->Checksum;
        fe.file_name.assign(es->NamePtr, es->NamePtr + es->NameInWords);
        nameList.push_back(fe);

        p += es->SizeOfData + 0xc;
        es = (ExtraSection*)p;
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

std::vector<file_entry> EncryptedXP3::ParseYuzu_HnfnThunk(const std::vector<char>& plainBytes)
{
    std::vector<file_entry> ve;
    const char* ptr = plainBytes.data();
    while (ptr < plainBytes.data() + plainBytes.size())
    {
        if (*(uint32_t*)ptr != MagicHnfn)
            break;

        file_entry e;

        ptr += 4;
        uint64_t size = *(uint64_t*)ptr;
        ptr += 8;
        e.checksum = *(uint32_t*)ptr;
        ptr += 4;
        uint16_t lengthInWord = *(uint16_t*)ptr;
        ptr += 2;
        e.file_name = wstring((wchar_t*)ptr, (wchar_t*)ptr + lengthInWord);
        ve.push_back(e);
        ptr += (lengthInWord + 1) * 2;
        assert((lengthInWord + 1) * 2 + 2 + 4 == size);
    }
    return ve;
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

std::vector<file_entry> EncryptedXP3::ExtractEntries(const std::vector<char>& _plainBytes)
{
    std::vector<char> plainBytes = _plainBytes;

    uint32_t magic;
    if (HasExtraSection(plainBytes, &magic))
        mExtraSectionMagic = magic;

    switch (*(uint32_t*)plainBytes.data())
    {
    case MagicYuzu: {
        auto header = reinterpret_cast<const YuzuRiddleJokerFileNameHeader*>(plainBytes.data());
        mStream.seekg(header->mFileTableOffset);
        vector<char> fileTable(header->mPackSize);
        mStream.read(fileTable.data(), fileTable.size());
        vector<char> fileTablePlain = ExtractYuzuFileTable(fileTable, header->mPlainSize);

        if (fileTablePlain.size() < sizeof(uint32_t) || *(uint32_t*)fileTablePlain.data() != MagicHnfn)
            break;
        vector<file_entry> fileNames = ParseYuzu_HnfnThunk(fileTablePlain);
        vector<file_entry> entries   = XP3ArcPraseEntryStage0(0, vector<char>(plainBytes.begin() + sizeof(YuzuRiddleJokerFileNameHeader), plainBytes.end()));

        for (auto& fn : fileNames)
        {
            auto it = find_if(entries.begin(), entries.end(), [&fn](const file_entry& e) { return fn.checksum == e.checksum; });
            if (it != entries.end())
                it->file_name = fn.file_name;
        }

        return entries;
    }
    case MagicHnfn:
        return ParsePalette_9nine(plainBytes);
    case MagicNeko:
        return ParsePalette_NekoparaEx(plainBytes);
    default:
        return XP3ArcPraseEntryStage0(mExtraSectionMagic, plainBytes);
    }

    return std::vector<file_entry>();
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