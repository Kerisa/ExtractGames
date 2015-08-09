#pragma once

#define MAXPATH 350

typedef struct
{
	wchar_t		  bgm_name[MAX_PATH];
	unsigned long offset_head;
	unsigned long head_len;
	unsigned long offset_loop;
	unsigned long loop_len;
}PAIR, *PPAIR;


const PAIR offset_th07 [] = {
	L"01 妖々夢　〜 Snow or Cherry Petal.wav",			      0x10, 0x129300,   0x129310,  0xE9E420,
	L"02 無何有の郷　〜 Deep Mountain.wav",				  0xfc7730, 0x2B9F80,  0x12816b0, 0x12816B0,
	L"03 クリスタライズシルバー.wav",						 0x1E8F730, 0x1D2EA0,  0x20625D0,  0x9E515C,
	L"04 遠野幻想物語.wav",								 0x2A4772C, 0x406C58,  0x2E4E384, 0x1378E34,
	L"05 ティアオイエツォン(withered leaf).wav",			 0x41C71B8,  0x9DE28,  0x4264FE0,  0x8D97D0,
	L"06 ブクレシュティの人形師.wav",						 0x4B3E7B0, 0x156E00,  0x4C955B0, 0x13A8C00,
	L"07 人形裁判 ～ 人の形弄びし少女.wav",				 0x603E1B0, 0x4C9800,  0x65079B0, 0x196E800,
	L"08 天空の花の都.wav",								 0x7E761B0, 0x15E180,  0x7FD4330, 0x27D3280,
	L"09 幽霊楽団 ～ Phantom Ensemble.wav",				 0xA7A75B0,  0xB6300,  0xA85D8B0, 0x1708C00,
	L"10 東方妖々夢 ～ Ancient Temple.wav",				 0xBF664B0, 0x1A5400,  0xC10B8B0, 0x1764800,
	L"11 広有射怪鳥事 ～ Till When？.wav",				 0xD8700B0, 0x121C00,  0xD991CB0, 0x11B6000,
	L"12 アルティメットトゥルース.wav",					 0xEB47CB0,  0x78800,  0xEBC04B0,  0x932C00,
	L"13 幽雅に咲かせ、墨染の桜 ～ Border of Life.wav",	 0xF4F30B0, 0x2E9E00,  0xF7DCEB0, 0x1552600,
	L"14 ボーダーオブライフ.wav",							0x10D2F4B0, 0xB1C400, 0x1184B8B0,  0x440900,
	L"15 妖々跋扈.wav", 									0x137CE2B0,  0xF8C00, 0x138C6EB0, 0x1709300,
	L"16 少女幻葬 ～ Necro-Fantasy.wav", 				0x14FD01B0,  0x3EF00, 0x1500F0B0, 0x23B3980,
	L"17 妖々跋扈 ～ Who done it!.wav",					0x173C2A30,  0xF8C00, 0x174BB630, 0x1709300,
	L"18 ネクロファンタジア.wav", 						0x18BC4930,  0x55500, 0x18C19E30, 0x1BD2B00,
	L"19 春風の夢.wav", 									0x11C8C1B0,  0xCA100, 0x11D562B0,  0xA18000,
	L"20 さくらさくら ～ Japanize Dream....wav",			0x1276E2B0, 0x500B00, 0x12C6EDB0,  0xB5F500
};

const PAIR offset_th08 [] = {
	L"01 永夜抄 ～ Eastern Night.wav",				0x10, 0xF1AC0, 0xF1AD0, 0xB2EA40,
	L"02 幻視の夜 ～ Ghostly Eyes.wav",				0xC20510, 0xAAE00, 0xCCB310, 0x1563D00,
	L"03 蠢々秋月 ～ Mooned Insect.wav",			0x222F010, 0x2A8000, 0x24D7010, 0xA78000,
	L"04 夜雀の歌声 ～ Night Bird.wav",				0x2F4F010, 0x76F00, 0x2FC5F10, 0x18B6100,
	L"05 もう歌しか聞こえない.wav",					0x487C010, 0x44D680, 0x4CC9690, 0xBE3A80,
	L"06 懐かしき東方の血 ～ Old World.wav",			0x58AD110, 0x29D200, 0x5B4A310, 0xCEAE00,
	L"07 プレインエイジア.wav",						0x6835110, 0x48A600, 0x6CBF710, 0xEB8400,
	L"08 永夜の報い ～ Imperishable Night.wav",		0x7B77B10, 0x164B00, 0x7CDC610, 0xEA3500,
	L"09 少女綺想曲 ～ Dream Battle.wav",			0x8B7FB10, 0x23AB00, 0x8DBA610, 0x195D500,
	L"10 恋色マスタースパーク.wav",					0xA717B10, 0x1BAC00, 0xA8D2710, 0x1397400,
	L"11 シンデレラケージ ～ Kagome-Kagome.wav",		0xBC69B10, 0x11FB00, 0xBD89610, 0x1C3AE00,
	L"12 狂気の瞳 ～ Invisible Full Moon.wav",		0xD9C4410, 0x82680, 0xDA46A90, 0x13B5780,
	L"13 ヴォヤージュ1969.wav",						0xEDFC210, 0x196AC0, 0xEF92CD0, 0xCF49E0,
	L"14 千年幻想郷 ～ History of the Moon.wav",	0xFC876B0, 0x12DB00, 0xFDB51B0, 0x1CCA900,
	L"15 竹取飛翔 ～ Lunatic Princess.wav",			0x128476B0, 0x65E700, 0x12EA5DB0, 0x17E8900,
	L"16 ヴォヤージュ1970.wav",						0x11A7FAB0, 0xBC93A0, 0x12648E50, 0x1FE860,
	L"17 エクステンドアッシュ ～ 蓬莱人.wav",		0x16763DB0, 0xD7280, 0x1683B030, 0x19E0A00,
	L"18 月まで届け、不死の煙.wav",					0x1821BA30, 0x145E20, 0x18361850, 0x1C77970,
	L"19 月見草.wav",								0x1468E6B0, 0x2F1A00, 0x149800B0, 0xC26800,
	L"20 Eternal Dream ～ 幽玄の槭樹.wav",			0x155A68B0, 0x346980, 0x158ED230, 0xE76B80,
	L"21 東方妖怪小町.wav",							0x19FD91C0, 0x10DC00, 0x1A0E6DC0, 0xC36E80
};

const PAIR offset_th09 [] = {
	L"01 花映塚 ～ Higan Retour.wav", 						0x10, 0xD5380, 0xD5390, 0xB20F80,
	L"02 春色小径 ～ Colorful Path.wav",					0x2E77990, 0x170A00, 0x2FE8390, 0x196E600,
	L"03 オリエンタルダークフライト.wav", 					0xBF6310, 0x1D5C00, 0xDCBF10, 0x14D4000,
	L"04 フラワリングナイト.wav", 							0x4956990, 0xCB640, 0x4A21FD0, 0x151BF00,
	L"05 東方妖々夢 ～ Ancient Temple.wav",					0xABF69D0, 0x290800, 0xAE871D0, 0x1832400,
	L"06 狂気の瞳 ～ Invisible Full Moon.wav",				0x101722D0, 0xA4000, 0x102162D0, 0x13B5800,
	L"07 おてんば恋娘の冒険.wav", 							0x5F3DED0, 0x1D3400, 0x61112D0, 0x172E800,
	L"08 幽霊楽団 ～ Phantom Ensemble.wav",					0x9437AD0, 0xB6300, 0x94EDDD0, 0x1708C00,
	L"09 もう歌しか聞こえない ～ Flower Mix.wav",			0x783FAD0, 0x1B7000, 0x79F6AD0, 0x1A41000,
	L"10 お宇佐さまの素い幡.wav", 							0x115CBAD0, 0xF3000, 0x116BEAD0, 0x117F220,
	L"11 風神少女(Short Version).wav",						0xC6B95D0, 0x2AFD00, 0xC9692D0, 0x1B61D00,
	L"12 ポイズンボディ ～ Forsaken Doll.wav",				0x1530F0F0, 0x10A500, 0x154195F0, 0x12C1500,
	L"13 今昔幻想郷 ～ Flower Land.wav", 					0x166DAAF0, 0x1CF700, 0x168AA1F0, 0x191B100,
	L"14 彼岸帰航 ～ Riverside View.wav", 					0xE4CAFD0, 0x268900, 0xE7338D0, 0x1A3EA00,
	L"15 六十年目の東方裁判 ～ Fate of Sixty Years.wav",		0x1283DCF0, 0x106C00, 0x129448F0, 0x29CA800,
	L"16 花の映る塚.wav",									0x229FF10, 0xB7900, 0x2357810, 0xB20180,
	L"17 此岸の塚.wav",										0x18A181F0, 0x34D680, 0x18D65870, 0x325880,
	L"18 花は幻想のままに.wav",								0x181C52F0, 0x61680, 0x18226970, 0x7F1880,
	L"19 魂の花 ～ Another Dream.wav", 						0x1908B0F0, 0xD8E00, 0x19163EF0, 0x1211D80
};

const PAIR offset_th10 [] = {
	L"01 封印されし神々.wav", 							0x10, 0x3C3000, 0x3C3010, 0xA6CA00,
	L"02 人恋し神樣 ～ Romantic Fall.wav",				0xE2FA10, 0x2975F0, 0x10C7000, 0x144D210,
	L"03 稲田姫樣に叱られるから.wav", 					0x2514210, 0x3ADF0, 0x254F000, 0x746210,
	L"04 厄神樣の通り道 ～ Dark Road.wav",				0x2C95210, 0x9F200, 0x2D34410, 0x13CCA00,
	L"05 運命のダークサイド.wav", 						0x4100E10, 0x7C1F0, 0x417D000, 0x114E410,
	L"06 神々が恋した幻想郷.wav", 						0x52CB410, 0x300000, 0x55CB410, 0x1BAB600,//
	L"07 芥川龍之介の河童 ～ Candid Friend.wav",			0x7176A10, 0x1AA5F0, 0x7321000, 0x16EDF10,
	L"08 フォールオブフォール ～ 秋めく滝.wav",			0x8A0EF10, 0x1A00F0, 0x8BAF000, 0x20B2710,
	L"09 妖怪の山 ～ Mysterious Mountain.wav",			0xAC61710, 0x193800, 0xADF4F10, 0xC7F600,
	L"10 少女が見た日本の原風景.wav", 					0xBA74510, 0x82000, 0xBAF6510, 0x1E47A00,//
	L"11 信仰は儚き人間の為に.wav", 						0xD93DF10, 0x272400, 0xDBB0310, 0x1B87900,//
	L"12 御柱の墓場 ～ Grave of Being.wav",				0xF737C10, 0x218800, 0xF950410, 0x948900,
	L"13 神さびた古戦場 ～ Suwa Foughten Field.wav",		0x10298D10, 0x4D92F0, 0x10772000, 0x168BD10,
	L"14 明日ハレの日、ケの昨日.wav", 					0x11DFDD10, 0x642F0, 0x11E62000, 0x2158890,
	L"15 ネイティブフェイス.wav", 						0x13FBA890, 0xD6770, 0x14091000, 0x1C30ED0,
	L"16 麓の神社.wav", 								0x15CC1ED0, 0x7A2130, 0x16464000, 0x7130D0,
	L"17 神は恵みの雨を降らす ～ Sylphid Dream.wav",		0x16B770D0, 0xA9DA04, 0x17614AD4, 0x492FA0,
	L"18 プレイヤーズスコア.wav",						0x17AA7A74, 0x1CACC8, 0x17C7273C, 0x4A2FF8
};

typedef struct
{
	int bgm_num;
	const PAIR *pp;
} IDX, *PIDX;

IDX Index [] = {
	20, offset_th07,
	00, 0,
	21, offset_th08,
	19, offset_th09,
	00, 0,
	18, offset_th10,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
};


wchar_t GameName[][MAXPATH] = {
	L"東方妖々夢　～ Perfect Cherry Blossom.",
	L"", // L"東方萃夢想　～ Immaterial and Missing Power."
	L"東方永夜抄　～ Imperishable Night.",
	L"東方花映塚　～ Phantasmagoria of Flower View.",
	L"", // L"東方文花帖　～ Shoot the Bullet."
	L"東方風神録　～ Mountain of Faith.",
	L"", // L"東方緋想天　～ Scarlet Weather Rhapsody."
	L"", // L"東方地霊殿　～ Subterranean Animism.",
	L"", // L"東方星蓮船　～ Undefined Fantastic Object.",
	L"", // L"東方非想天則　～ 超弩級ギニョルの謎を追え",
	L"", // L"ダブルスポイラー　～ 東方文花帖",
	L"", // L"妖精大戦争　～ 東方三月精",
	L"", // L"東方神霊廟　～ Ten Desires.",
	L"", // L"東方輝針城　～ Double Dealing Character."
	L"",  // L"弹幕天邪鬼　～ Impossible Spell Card."
	L"",  // L"东方深秘录　～ Urban Legend in Limbo."
	L"",  // L"东方心绮楼　～ Hopeless Masquerade."
	L""  // L"東方輝針城　～ Double Dealing Character."
};