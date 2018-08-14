#pragma once

#include <fstream>
#include <string>
#include <vector>

class DDSystem
{
public:
	struct Header
	{
		enum {
			MAGIC_2 = 0x32504444,	// DDP2
			MAGIC_3 = 0x33504444,	// DDP3
		};
		uint32_t Magic;
		uint32_t EntryCount;
		uint32_t DataOffset;
		uint8_t  Revsered[20];
	};

	struct Entry2
	{
		uint32_t Offset;
		uint32_t PlainSize;
		uint32_t PackedSize;
		uint32_t Revsered;
	};

	struct Entry3_L1
	{
		uint32_t BlockSize;
		uint32_t BlockOffset;
	};

#pragma pack(push, 1)
	struct Entry3_L2_Header
	{
		uint8_t  Size;
		uint32_t Offset;
		uint32_t PlainSize;
		uint32_t PackedSize;
		uint8_t  Revsered[4];
		// follow   char* NameWithoutSuffix;
	};
#pragma pack(pop)

    struct ScriptCipherHeader
    {
        char Magic[8];      // "DDSxHXB"
        uint8_t BigEnddingSize[3];
        uint8_t Flag;
        uint8_t Revsered[4];
    };

	struct NormalizedEntry
	{
		uint32_t Offset;
		uint32_t PlainSize;
		uint32_t PackedSize;
		std::string NameWithoutSuffix;
	};

public:
	bool Open(const std::string& path);
	void Close();
	bool ExtractEntries();
	bool ExtractResource(const std::string& saveDir);

private:
	bool        ExtractDDP2();
	bool        ExtractDDP3();
	int         Decompress(uint8_t* dst, int dstLen, uint8_t* src, int srcLen);
	std::string ReadNullTerminateString(std::ifstream& stream);
	std::string DetermineSuffix(std::vector<uint8_t>& plainData);
    bool        ProcessScriptFile(std::vector<uint8_t>& plainData);

public:
	std::string                     mPackagePath;
	std::ifstream                   mStream;
	Header                          mHeader;
	std::vector<NormalizedEntry>    mEntries;

private:
	int mDefaultName{ 1 };
};