#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdint.h>
#include <string_view>
#include <stdexcept>
#include <vector>
#include <unordered_map>
//#include <chrono>
#include <time.h>

extern std::unordered_map<int, const char*> itemNamesById;
extern std::unordered_map<int, const char*> prefixNamesById;

using namespace std;

typedef uint64_t u64;
typedef int64_t i64;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint8_t u8;
typedef int8_t i8;


class BinaryReader
{
private:
	char* buf;
	uint64_t size;
	uint64_t readPos = 0;
public:
	BinaryReader(char* p_buffer, unsigned long p_size)
		: buf{p_buffer}, size{p_size} {}
	template <typename T> T read()
	{
		if (readPos >= size) { throw std::out_of_range("Reading past end of file"); }
		T ret = *(T*)&(buf[readPos]);
		readPos += sizeof(T);
		return ret;
	}
	template <typename T> T peek()
	{
		if (readPos >= size) { throw std::out_of_range("Reading past end of file"); }
		return *(T*)&(buf[readPos]);
	}
	std::string_view readStr()
	{
		if (readPos >= size) { throw std::out_of_range("Reading past end of file"); }
		u8 len = read<u8>();
		string_view ret(buf + readPos, len);
		readPos += len;
		return ret;
	}
	std::string_view readChars(u64 len)
	{
		if (readPos >= size) { throw std::out_of_range("Reading past end of file"); }
		string_view ret{&buf[readPos], len};
		readPos += len;
		return ret;
	}
	void seekg(u64 pos)
	{
		if (pos < size)
		{
			readPos = pos;
		}
		else
		{
			throw std::out_of_range("Seeking past end of file");
		}
	}
	u64 tellg() { return readPos; }
};


void showItem(int abs_x, int abs_y, int gps_x, int gps_y, int iChest, std::string_view chestName, int iStack, int stackSize, int itemid, int prefix, u32 chestFileOfs, u32 itemFileOfs)
{
	const char* itemName = "unknown_item";
	const char* prefixName = "";
	if (itemNamesById.contains(itemid)) { itemName = itemNamesById[itemid]; }
	if (prefixNamesById.contains(prefix)) { prefixName = prefixNamesById[prefix]; }
	//std::cout << "\t" << std::hex << "0x" << itemFileOfs << std::dec << " (" << x << ", " << y << ") [" << iChest << "] " << "\"" << chestName << "\" {" << stackSize << ", " << itemid << ", " << (int)prefix << "} "
	//	<< stackSize << " x " << prefixName << " " << itemName << "\n";

	std::cout << "0x" << std::hex << setw(8) << setfill('0') << chestFileOfs
			  << std::dec << std::setw(0) << std::setfill(' ') << " tile(" << setw(5) << abs_x << setw(0) << ", " << setw(5) << abs_y << ") "
			  << std::dec << std::setw(0) << std::setfill(' ') << " gps(" << setw(5) << gps_x << setw(0) << ", " << setw(5) << gps_y << ") "
			  << "Chest #" << setw(4) << iChest << setw(0) << ": "
			  << setw(20) << setfill('-') << chestName << setw(0) << setfill(' ');

	std::cout << " ";
	std::cout //<< "\t"
		<< std::hex << "0x" << setw(8) << setfill('0') << itemFileOfs << std::dec << setfill(' ')
		<< setw(0) << " {" << setw(4) << stackSize << setw(0) << ", "
		<< setw(5) << itemid << setw(0) << ", " << setw(2) << (int)prefix << setw(0) << "} "
		<< setw(4) << stackSize << setw(0) << " x " << prefixName << (prefix == 0 ? "" : " ") << itemName << "\n";

}

void showChest(int abs_x, int abs_y, int gps_x, int gps_y, int iChest, std::string_view chestName, u32 chestFileOfs)
{
	//std::cout << std::hex << "0x" << chestFileOfs << std::dec << " (" << x << ", " << y << ") " << "Chest #" << iChest << ": " << chestName << "\n";
	//std::cout << "0x" << std::hex << setw(8) << setfill('0') << chestFileOfs << std::dec << std::setw(0) << std::setfill(' ') << " (" << setw(5) << x << setw(0) << ", " << setw(5) << y << ") " << "Chest #" << setw(4) << iChest << setw(0) << ": " << chestName;

	std::cout << "0x" << std::hex << setw(8) << setfill('0') << chestFileOfs
			  << std::dec << std::setw(0) << std::setfill(' ') << " tile(" << setw(5) << abs_x << setw(0) << ", " << setw(5) << abs_y << ") "
			  << std::dec << std::setw(0) << std::setfill(' ') << " gps(" << setw(5) << gps_x << setw(0) << ", " << setw(5) << gps_y << ") "
			  << "Chest #" << setw(4) << iChest << setw(0) << ": "
			  << setw(20) << setfill('-') << chestName << setw(0) << setfill(' ');

	std::cout << "\n";
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "%s", "pass the name of a world file as an argument\n");
		return 1;
	}
	char* worldFileName = argv[1];

	FILE* f = fopen(worldFileName, "rb");
	if (!f)
	{
		fprintf(stderr, "Can not open file\n");
		return -4;
	}
	fseek(f, 0, SEEK_END);
	auto flen = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* worldData = new char[flen];
	fread(worldData, 1, flen, f);
	fclose(f);

	BinaryReader w(worldData, flen);

	auto version = w.read<i32>();
	bool isAndroid = false;
	u32 fileRevision = 0;
	u32 fileFlags = 0;
	if (version >= 140)
	{
		//fprintf(stderr, "%s", "World version is too old.\n");
		//return 2;
		isAndroid = (w.peek<u8>() == 'x');
		auto headerFormat = w.readChars(7);
		auto fileType = w.read<char>();
		if (fileType != 2) // 2 == world (as opposed to character I guess)
		{
			fprintf(stderr, "%s", "File type is not a world.\n");
			return 3;
		}
		fileRevision = w.read<u32>();
		fileFlags = w.read<u64>();
		// w.IsFavorite = ((flags & 1uL) == 1uL);
	}

	auto sectionCount = w.read<u16>();
	std::vector<u32> sectionOfs(sectionCount);
	for (u32 i = 0; i < sectionCount; i++)
	{
		sectionOfs[i] = w.read<u32>();
	}

	w.seekg(sectionOfs[0]); // world information
	std::string seed;
	auto worldTitle = w.readStr();
	if (version >= 179)
	{
		if (version == 179)
			seed = std::to_string(w.read<i32>());
		else
			seed = w.readStr();
		auto WorldGenVersion = w.read<u64>();
	}
	else
	{
		seed = "";
	}
	string_view worldGuid;
	if (version >= 181)
	{
		worldGuid = w.readChars(16);
	}
	else
	{
		worldGuid = "1234567890123456";
	}
	auto WorldId = w.read<i32>();
	auto LeftWorld = w.read<i32>();
	auto RightWorld = w.read<i32>();
	auto TopWorld = w.read<i32>();
	auto BottomWorld = w.read<i32>();
	auto TilesHigh = w.read<i32>();
	auto TilesWide = w.read<i32>();

	i32 GameMode = 0;
	bool DrunkWorld = false, GoodWorld = false, TenthAnniversaryWorld = false,
			DontStarveWorld = false, NotTheBeesWorld = false, RemixWorld = false,
			NoTrapsWorld = false, ZenithWorld = false;
	if (version >= 209)
	{
		GameMode = w.read<i32>();

		if (version >= 222) { auto DrunkWorld = w.read<u8>(); }
		if (version >= 227) { auto GoodWorld = w.read<u8>(); }
		if (version >= 238) { auto TenthAnniversaryWorld = w.read<u8>(); }
		if (version >= 239) { auto DontStarveWorld = w.read<u8>(); }
		if (version >= 241) { auto NotTheBeesWorld = w.read<u8>(); }
		if (version >= 249) { auto RemixWorld = w.read<u8>(); }
		if (version >= 266) { auto NoTrapsWorld = w.read<u8>(); }
		ZenithWorld = (version < 267) ? RemixWorld && DrunkWorld : w.read<u8>();
	}
	else if (version == 208)
	{
		GameMode = w.read<u8>() ? 2 : 0;
	}
	else if (version >= 112)
	{
		GameMode = w.read<u8>() ? 1 : 0;
	}
	else
	{
		GameMode = 0;
	}

	u64 CreationTime = version >= 141 ? w.read<u64>() : (u64)time(nullptr);

	auto MoonType = w.read<u8>();// r.ReadByte();
	auto TreeX0 = w.read<i32>();//r.ReadInt32();
	auto TreeX1 = w.read<i32>();//r.ReadInt32();
	auto TreeX2 = w.read<i32>();//r.ReadInt32();
	//auto TreeX2 = w.TreeX[2];
	//auto TreeX1 = w.TreeX[1];
	//auto TreeX0 = w.TreeX[0];
	auto TreeStyle0 = w.read<i32>();//r.ReadInt32();
	auto TreeStyle1 = w.read<i32>();//r.ReadInt32();
	auto TreeStyle2 = w.read<i32>();//r.ReadInt32();
	auto TreeStyle3 = w.read<i32>();//r.ReadInt32();
	auto CaveBackX0 = w.read<i32>();//r.ReadInt32();
	auto CaveBackX1 = w.read<i32>();//r.ReadInt32();
	auto CaveBackX2 = w.read<i32>();//r.ReadInt32();
	//auto CaveBackX2 = w.CaveBackX[2];
	//auto CaveBackX1 = w.CaveBackX[1];
	//auto CaveBackX0 = w.CaveBackX[0];
	auto CaveBackStyle0 = w.read<i32>();//r.ReadInt32();
	auto CaveBackStyle1 = w.read<i32>();//r.ReadInt32();
	auto CaveBackStyle2 = w.read<i32>();//r.ReadInt32();
	auto CaveBackStyle3 = w.read<i32>();//r.ReadInt32();
	auto IceBackStyle = w.read<i32>();//r.ReadInt32();
	auto JungleBackStyle = w.read<i32>();//r.ReadInt32();
	auto HellBackStyle = w.read<i32>();//r.ReadInt32();

	auto SpawnX = w.read<i32>();//r.ReadInt32();
	auto SpawnY = w.read<i32>();//r.ReadInt32();
	auto GroundLevel = w.read<double>();// r.ReadDouble();
	auto RockLevel = w.read<double>();// r.ReadDouble();












	w.seekg(sectionOfs[2]); // chests

	auto totalChests = w.read<u16>();
	auto maxItems = w.read<u16>();

	for (u16 iChest = 0; iChest < totalChests; iChest++)
	{
		u32 chestFileOfs = (u32)w.tellg();
		auto x = w.read<i32>();
		auto y = w.read<i32>();
		auto name = w.readStr();

		int displayX = (x - SpawnX) * 2;
		//int displayX = x - LeftWorld;
		// TODO: y coordinate system?
		int displayY = (GroundLevel - y) * 2;
		//int displayY = y - GroundLevel;
		//int displayY = RockLevel - y;


		//displayX = x;
		//displayY = y;


		showChest(x, y, displayX, displayY, iChest, name, chestFileOfs);
		for (u16 iStack = 0; iStack < maxItems; iStack++)
		{
			u32 itemFileOfs = w.tellg();
			auto stackSize = w.read<u16>();
			if (stackSize > 0)
			{
				auto itemid = w.read<i32>();
				auto prefix = w.read<u8>();
				//printf("(%d, %d) Chest #%d (%s) %s %s x%d (itemid %d, prefix id %d)\n",
				//	   x, y, iChest, name, "[prefix]", itemNamesById[itemid], stackSize, itemid, prefix);
				//std::cout << "(" << x << ", " << y << ") [" << iChest << "] " << "\"" << name << "\" {" << stackSize << ", " << itemid << ", " << (int)prefix << "} "
				//	//<< prefixNamesById[prefix] << " "
				//	<< stackSize << " x " << itemNamesById[itemid] << "\n";

				showItem(x, y, displayX, displayY, iChest, name, iStack, stackSize, itemid, prefix, chestFileOfs, itemFileOfs);
			}
		}
	}

	delete [] worldData;
	return 0;
}
