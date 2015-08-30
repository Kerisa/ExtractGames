#pragma once

const int MAXPATH = 350;

typedef struct
{
	wchar_t		  bgm_name[MAX_PATH];
	unsigned long offset_head;
	unsigned long head_len;
	unsigned long offset_loop;
	unsigned long loop_len;
}PAIR, *PPAIR;

const unsigned char WAVEHEAD [] = {
	0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 
	0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x44, 0xAC, 0x00, 0x00, 0x10, 0xB1, 0x02, 0x00, 
	0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00
};	// 用的时候只要修改文件长度和波形数据长度就够了
	// 其他参数似乎也都是一致的

const PAIR offset_th07 [] = {
    L"01 妖々夢　〜 Snow or Cherry Petal.wav",                   0x10, 0x129300,   0x129310,  0xE9E420,
    L"02 無何有の郷　〜 Deep Mountain.wav",                  0xfc7730, 0x2B9F80,  0x12816b0, 0x12816B0,
    L"03 クリスタライズシルバー.wav",                      0x1E8F730, 0x1D2EA0,  0x20625D0,  0x9E515C,
    L"04 遠野幻想物語.wav",                                0x2A4772C, 0x406C58,  0x2E4E384, 0x1378E34,
    L"05 ティアオイエツォン(withered leaf).wav",           0x41C71B8,  0x9DE28,  0x4264FE0,  0x8D97D0,
    L"06 ブクレシュティの人形師.wav",                      0x4B3E7B0, 0x156E00,  0x4C955B0, 0x13A8C00,
    L"07 人形裁判 ～ 人の形弄びし少女.wav",                0x603E1B0, 0x4C9800,  0x65079B0, 0x196E800,
    L"08 天空の花の都.wav",                                0x7E761B0, 0x15E180,  0x7FD4330, 0x27D3280,
    L"09 幽霊楽団 ～ Phantom Ensemble.wav",                0xA7A75B0,  0xB6300,  0xA85D8B0, 0x1708C00,
    L"10 東方妖々夢 ～ Ancient Temple.wav",                0xBF664B0, 0x1A5400,  0xC10B8B0, 0x1764800,
    L"11 広有射怪鳥事 ～ Till When？.wav",                 0xD8700B0, 0x121C00,  0xD991CB0, 0x11B6000,
    L"12 アルティメットトゥルース.wav",                    0xEB47CB0,  0x78800,  0xEBC04B0,  0x932C00,
    L"13 幽雅に咲かせ、墨染の桜 ～ Border of Life.wav",    0xF4F30B0, 0x2E9E00,  0xF7DCEB0, 0x1552600,
    L"14 ボーダーオブライフ.wav",                         0x10D2F4B0, 0xB1C400, 0x1184B8B0,  0x440900,
    L"15 妖々跋扈.wav",                                   0x137CE2B0,  0xF8C00, 0x138C6EB0, 0x1709300,
    L"16 少女幻葬 ～ Necro-Fantasy.wav",                  0x14FD01B0,  0x3EF00, 0x1500F0B0, 0x23B3980,
    L"17 妖々跋扈 ～ Who done it!.wav",                   0x173C2A30,  0xF8C00, 0x174BB630, 0x1709300,
    L"18 ネクロファンタジア.wav",                         0x18BC4930,  0x55500, 0x18C19E30, 0x1BD2B00,
    L"19 春風の夢.wav",                                   0x11C8C1B0,  0xCA100, 0x11D562B0,  0xA18000,
    L"20 さくらさくら ～ Japanize Dream....wav",          0x1276E2B0, 0x500B00, 0x12C6EDB0,  0xB5F500
};

const PAIR offset_th075 [] = {
	L"01 萃夢想／U2.wav",                                       0x111040C2, 0x1274AC, 0x1122B56E,  0xEFCBFC,
	L"02 東方妖恋談／NKZ.wav",                                       0xE5A,  0x59CF4,    0x5AB4E,  0xDE8F68,
	L"03 少女綺想曲 ～ Capriccio／U2.wav",                        0xE43AB6, 0x111420,   0xF54ED6,  0xB2DFC4,
	L"04 恋色マジック／NKZ.wav",                                 0x320B80A,  0x87578,  0x3292D82,  0xA2221C,
	L"05 魔女達の舞踏会／U2.wav",                                0x3CB4F9E,  0x1A918,  0x3CCF8B6,  0xB14774,
	L"06 メイドと血の懷中時計／NKZ.wav",                         0x47E402A,  0x349F0,  0x4818A1A, 0x1112BA4,
	L"07 月時計 ～ ルナダイアル／U2.wav",                        0x592B5BE,  0x75E00,  0x59762DA,  0xE4078C,
	L"08 ブクレシュティの人形師／NKZ.wav",                       0x67B6A66,  0x8690C,  0x683D372, 0x1392CF8,
	L"09 人形裁判／U2.wav",                                      0x7BD006A,  0x68B8C,  0x7C38BF6,  0xA81D1C,
	L"10 ラクトガール ～ 少女密室／NKZ.wav",                     0x86BA912,  0x768A8,  0x87311BA, 0x10F37F0,
	L"11 ヴワル魔法図書館／U2.wav",                              0x98249AA, 0x2326E4,  0x9A5708E,  0xCAADC4,
	L"12 広有射怪鳥事 ～ Till When？／NKZ.wav",                  0xA701E52,  0x78F70,  0xA77ADC2,  0xB702D4,
	L"13 東方妖々夢 ～ Ancient Temple／U2.wav",                  0xB2EB096, 0x26F9CC,  0xB55AA62,  0xC2EADC, 
	L"14 亡き王女の為のセプテット／NKZ.wav",                     0xC18953E, 0x3F3FBC,  0xC57D4FA,  0x83E130,
	L"15 幽雅に咲かせ、墨染の桜 ～ Border of Life／NKZ.wav",     0xCDBB62A, 0x230B6C,  0xCFC10B2,  0x758F34,
	L"16 Demystify Feast／ZUN.wav",                              0x1A82E9A, 0x1AEC44,  0x1C31ADE, 0x15D9D2C,
	L"17 夜が降りてくる ～ Evening Star／ZUN.wav",               0xD719FE6, 0x10251C,  0xD81C502, 0x17DF488,
	L"18 御伽の国の鬼が島 ～ Missing Power／ZUN.wav",            0xEFFB98A,  0x96FC0,  0xF09294A, 0x2071778,
	L"19 夏明き／NKZ.wav",                                      0x13813DAE,  0xA8814, 0x138BC5C2,  0x9247E0,
	L"20 東方萃夢想／U2.wav",                                   0x1212816A,  0x2B110, 0x121532A6,  0xDE4734,
	L"21 魔所／NKZ.wav",                                        0x12F379DA,  0x2C52C, 0x12F63F06,  0x58D3E0,
	L"22 月輪／NKZ.wav",                                        0x134F12E6, 0x12BC94, 0x1361CF7A,  0x1F6E34,
	L"23 遍參／NKZ.wav",                                        0x141E0DA2,  0x84614, 0x142653B6,  0xC47944,
	L"24 裏心／NKZ.wav",                                        0x14EACCFA,  0x1B934, 0x14EC862E,  0x3AE480,
	L"25 Intermezzo／NKZ.wav",                                  0x15276AAE,  0x93BF8, 0x1530A6A6,  0x4A8E90,
	L"26 あゆのかぜ／NKZ.wav",                                  0x157B3536,  0x43C38, 0x157F716E,  0x676548,
	L"27 森閑／U2.wav",                                         0x188371E6,  0x4B6C8, 0x188828AE,  0x6E8DE0,
	L"28 仰空／U2.wav",                                         0x171D840A,  0x9B9B4, 0x17273DBE,  0x74CAD8,
	L"29 幽境／U2.wav",                                         0x15E6D6B6,  0x2DB48, 0x15E9B1FE,  0x61ECBC,
	L"30 珍客／U2.wav",                                         0x18F6B68E,  0x811AC, 0x18FEC83A,  0x840680,
	L"31 紅夜／U2.wav",                                         0x164B9EBA,  0x43E24, 0x164FDCDE,  0x5F1284,
	L"32 戰迅／U2.wav",                                         0x16AEEF62,  0x998C0, 0x16B88822,  0x64FBE8,
	L"33 禍機／U2.wav",                                         0x181FB13E,  0xACCC4, 0x182A7E02,  0x58F3E4,
	L"34 碎月／U2.wav",                                         0x179C0896,  0x7D374, 0x17A3DC0A,  0x7BD534
};

const PAIR offset_th08 [] = {
    L"01 永夜抄 ～ Eastern Night.wav",                 0x10,  0xF1AC0,    0xF1AD0,  0xB2EA40,
    L"02 幻視の夜 ～ Ghostly Eyes.wav",            0xC20510,  0xAAE00,   0xCCB310, 0x1563D00,
    L"03 蠢々秋月 ～ Mooned Insect.wav",          0x222F010, 0x2A8000,  0x24D7010,  0xA78000,
    L"04 夜雀の歌声 ～ Night Bird.wav",           0x2F4F010,  0x76F00,  0x2FC5F10, 0x18B6100,
    L"05 もう歌しか聞こえない.wav",               0x487C010, 0x44D680,  0x4CC9690,  0xBE3A80,
    L"06 懐かしき東方の血 ～ Old World.wav",      0x58AD110, 0x29D200,  0x5B4A310,  0xCEAE00,
    L"07 プレインエイジア.wav",                   0x6835110, 0x48A600,  0x6CBF710,  0xEB8400,
    L"08 永夜の報い ～ Imperishable Night.wav",   0x7B77B10, 0x164B00,  0x7CDC610,  0xEA3500,
    L"09 少女綺想曲 ～ Dream Battle.wav",         0x8B7FB10, 0x23AB00,  0x8DBA610, 0x195D500,
    L"10 恋色マスタースパーク.wav",               0xA717B10, 0x1BAC00,  0xA8D2710, 0x1397400,
    L"11 シンデレラケージ ～ Kagome-Kagome.wav",  0xBC69B10, 0x11FB00,  0xBD89610, 0x1C3AE00,
    L"12 狂気の瞳 ～ Invisible Full Moon.wav",    0xD9C4410,  0x82680,  0xDA46A90, 0x13B5780,
    L"13 ヴォヤージュ1969.wav",                   0xEDFC210, 0x196AC0,  0xEF92CD0,  0xCF49E0,
    L"14 千年幻想郷 ～ History of the Moon.wav",  0xFC876B0, 0x12DB00,  0xFDB51B0, 0x1CCA900,
    L"15 竹取飛翔 ～ Lunatic Princess.wav",      0x128476B0, 0x65E700, 0x12EA5DB0, 0x17E8900,
    L"16 ヴォヤージュ1970.wav",                  0x11A7FAB0, 0xBC93A0, 0x12648E50,  0x1FE860,
    L"17 エクステンドアッシュ ～ 蓬莱人.wav",    0x16763DB0,  0xD7280, 0x1683B030, 0x19E0A00,
    L"18 月まで届け、不死の煙.wav",              0x1821BA30, 0x145E20, 0x18361850, 0x1C77970,
    L"19 月見草.wav",                            0x1468E6B0, 0x2F1A00, 0x149800B0,  0xC26800,
    L"20 Eternal Dream ～ 幽玄の槭樹.wav",       0x155A68B0, 0x346980, 0x158ED230,  0xE76B80,
    L"21 東方妖怪小町.wav",                      0x19FD91C0, 0x10DC00, 0x1A0E6DC0,  0xC36E80
};

const PAIR offset_th09 [] = {
    L"01 花映塚 ～ Higan Retour.wav",                          0x10,   0xD5380,    0xD5390,  0xB20F80,
    L"02 春色小径 ～ Colorful Path.wav",                   0x2E77990, 0x170A00,  0x2FE8390, 0x196E600,
    L"03 オリエンタルダークフライト.wav",                   0xBF6310, 0x1D5C00,   0xDCBF10, 0x14D4000,
    L"04 フラワリングナイト.wav",                          0x4956990,  0xCB640,  0x4A21FD0, 0x151BF00,
    L"05 東方妖々夢 ～ Ancient Temple.wav",                0xABF69D0, 0x290800,  0xAE871D0, 0x1832400,
    L"06 狂気の瞳 ～ Invisible Full Moon.wav",            0x101722D0,  0xA4000, 0x102162D0, 0x13B5800,
    L"07 おてんば恋娘の冒険.wav",                          0x5F3DED0, 0x1D3400,  0x61112D0, 0x172E800,
    L"08 幽霊楽団 ～ Phantom Ensemble.wav",                0x9437AD0,  0xB6300,  0x94EDDD0, 0x1708C00,
    L"09 もう歌しか聞こえない ～ Flower Mix.wav",          0x783FAD0, 0x1B7000,  0x79F6AD0, 0x1A41000,
    L"10 お宇佐さまの素い幡.wav",                         0x115CBAD0,  0xF3000, 0x116BEAD0, 0x117F220,
    L"11 風神少女(Short Version).wav",                     0xC6B95D0, 0x2AFD00,  0xC9692D0, 0x1B61D00,
    L"12 ポイズンボディ ～ Forsaken Doll.wav",            0x1530F0F0, 0x10A500, 0x154195F0, 0x12C1500,
    L"13 今昔幻想郷 ～ Flower Land.wav",                  0x166DAAF0, 0x1CF700, 0x168AA1F0, 0x191B100,
    L"14 彼岸帰航 ～ Riverside View.wav",                  0xE4CAFD0, 0x268900,  0xE7338D0, 0x1A3EA00,
    L"15 六十年目の東方裁判 ～ Fate of Sixty Years.wav",  0x1283DCF0, 0x106C00, 0x129448F0, 0x29CA800,
    L"16 花の映る塚.wav",                                  0x229FF10,  0xB7900,  0x2357810,  0xB20180,
    L"17 此岸の塚.wav",                                   0x18A181F0, 0x34D680, 0x18D65870,  0x325880,
    L"18 花は幻想のままに.wav",                           0x181C52F0,  0x61680, 0x18226970,  0x7F1880,
    L"19 魂の花 ～ Another Dream.wav",                    0x1908B0F0,  0xD8E00, 0x19163EF0, 0x1211D80
};

const PAIR offset_th095 [] = {
	L"01 天狗の手帖 ～ Mysterious Note.wav",         0x10,  0x148000,  0x148010,  0xEF0000,
	L"02 風の循環 ～ Wind Tour.wav",            0x1038010,   0x4CAE0, 0x1084AF0,  0xCFD940,
	L"03 天狗が見ている ～ Black Eyes.wav",     0x1D82430,   0x6BA00, 0x1DEDE30, 0x1492A00,
	L"04 東の国の眠らない夜.wav",              0x03280830,  0x100000, 0x3380830, 0x1541800,
	L"05 レトロスペクティブ京都.wav",           0x48C2030,   0xFA800, 0x49BC830,  0xFA9100,
	L"06 風神少女.wav",                         0x5965930,  0x2AFD00, 0x5C15630, 0x1B61D00
};

const PAIR offset_th10 [] = {
    L"01 封印されし神々.wav",                                0x10, 0x3C3000,   0x3C3010,  0xA6CA00,
    L"02 人恋し神樣 ～ Romantic Fall.wav",               0xE2FA10, 0x297C00,  0x10C7610, 0x144CC00,
    L"03 稲田姫樣に叱られるから.wav",                   0x2514210,  0x3B600,  0x254F810,  0x745A00,
    L"04 厄神樣の通り道 ～ Dark Road.wav",              0x2C95210,  0x9F200,  0x2D34410, 0x13CCA00,
    L"05 運命のダークサイド.wav",                       0x4100E10,  0x7CA00,  0x417D810, 0x114DC00,
    L"06 神々が恋した幻想郷.wav",                       0x52CB410, 0x300000,  0x55CB410, 0x1BAB600,
    L"07 芥川龍之介の河童 ～ Candid Friend.wav",        0x7176A10, 0x1AA800,  0x7321210, 0x16EDD00,
    L"08 フォールオブフォール ～ 秋めく滝.wav",         0x8A0EF10, 0x1A1000,  0x8BAFF10, 0x20B1800,
    L"09 妖怪の山 ～ Mysterious Mountain.wav",          0xAC61710, 0x193800,  0xADF4F10,  0xC7F600,
    L"10 少女が見た日本の原風景.wav",                   0xBA74510,  0x82000,  0xBAF6510, 0x1E47A00,
    L"11 信仰は儚き人間の為に.wav",                     0xD93DF10, 0x272400,  0xDBB0310, 0x1B87900,
    L"12 御柱の墓場 ～ Grave of Being.wav",             0xF737C10, 0x218800,  0xF950410,  0x948900,
    L"13 神さびた古戦場 ～ Suwa Foughten Field.wav",   0x10298D10, 0x4D9B00, 0x10772810, 0x168B500,
    L"14 明日ハレの日、ケの昨日.wav",                  0x11DFDD10,  0x65100, 0x11E62E10, 0x2157A80,
    L"15 ネイティブフェイス.wav",                      0x13FBA890,  0xD7220, 0x14091AB0, 0x1C30420,
    L"16 麓の神社.wav",                                0x15CC1ED0, 0x7A2C00, 0x16464AD0,  0x712600,
    L"17 神は恵みの雨を降らす ～ Sylphid Dream.wav",   0x16B770D0, 0xA9DA04, 0x17614AD4,  0x492FA0,
    L"18 プレイヤーズスコア.wav",                      0x17AA7A74, 0x1CACC8, 0x17C7273C,  0x4A2FF8
};

const PAIR offset_th11 [] = {
    L"01 地霊達の起床.wav",                            0x10,  0x53E00,    0x53E10,  0xCC7F80,
    L"02 暗闇の風穴.wav",                          0xD1BD90,  0x64D80,   0xD80B10, 0x1046C00,
    L"03 封じられた妖怪 ～ Lost Place.wav",       0x1DC7710,  0xC7000,  0x1E8E710,  0x890880,
    L"04 渡る者の途絶えた橋.wav",                 0x271EF90,  0xA3A00,  0x27C2990, 0x1220800,
    L"05 緑眼のジェラシー.wav",                   0x39E3190,  0x90800,  0x3A73990, 0x120E600,
    L"06 旧地獄街道を行く.wav",                   0x4C81F90,  0x8F280,  0x4D11210, 0x1305800,
    L"07 華のさかづき大江山.wav",                 0x6016A10, 0x101700,  0x6118110, 0x1131500,
    L"08 ハートフェルトファンシー.wav",           0x7249610, 0x15D600,  0x73A6C10, 0x1635100,
    L"09 少女さとり ～ 3rd eye.wav",              0x89DBD10,  0x9BE00,  0x8A77B10, 0x1468A80,
    L"10 廃獄ララバイ.wav",                       0x9EE0590,  0xACDA8,  0x9F8D338, 0x1DB62D0,
    L"11 死体旅行 ～ Be of good cheer!.wav",      0xBD43608,  0x84600,  0xBDC7C08, 0x108CF00,
    L"12 業火マントル",                           0xCE54B08,  0x96800,  0xCEEB308, 0x1313800,
    L"13 霊知の太陽信仰 ～ Nuclear Fusion.wav",   0xE1FEB08,  0x9A900,  0xE299408, 0x1BF2500,
    L"14 ラストリモート",                         0xFE8B908, 0x10D780,  0xFF99088, 0x1F13500,
    L"15 ハルトマンの妖怪少女.wav",              0x11EAC588, 0x3EBD00, 0x12298288, 0x1607500,
    L"16 地霊達の帰宅.wav",                      0x1389F788,  0xBEE00, 0x1395E588,  0xB6EE00,
    L"17 エネルギー黎明 ～ Future Dream....wav", 0x144CD388, 0x521BE0, 0x149EEF68,  0x9193E0,
    L"18 プレイヤーズスコア.wav",                0x15308348, 0x1CACC8, 0x154D3010,  0x4A2FF8
};

const PAIR offset_th12 [] = {
	L"01 青空の影.wav",                          0x10,  0xF8040,    0xF8050,  0x909FC0,
	L"02 春の湊に.wav",                      0xA02010, 0x24F900,   0xC51910,  0xE96680,
	L"03 小さな小さな賢将.wav",             0x1AE7F90, 0x2B7E00,  0x1D9FD90,  0xD5C300,
	L"04 閉ざせし雲の通い路.wav",           0x2AFC090,  0x89000,  0x2B85090, 0x1278A00,
	L"05 万年置き傘にご注意を.wav",         0x3DFDA90, 0x24F500,  0x404CF90, 0x14F1E00,
	L"06 スカイルーイン.wav",               0x553ED90, 0x12E300,  0x566D090, 0x1548800,
	L"07 時代親父とハイカラ少女.wav",       0x6BB5890, 0x14C400,  0x6D01C90, 0x110FE00,
	L"08 幽霊客船の時空を越えた旅.wav",     0x7E11A90,  0x9B000,  0x7EACA90, 0x1790600,
	L"09 キャプテン·ムラサ.wav",            0x963D090, 0x173300,  0x97B0410, 0x1430100,
	L"10 魔界地方都市エソテリア.wav",       0xABE0510, 0x31CA00,  0xAEFCF10, 0x1A9A200,
	L"11 虎柄の毘沙門天.wav",               0xC997110, 0x4F0300,  0xCE87410, 0x1E59680,
	L"12 法界の火.wav",                     0xECE0A90,  0x8D180,  0xED6DC10, 0x109C700,
	L"13 感情の摩天楼 ～ Cosmic Mind.wav",  0xFE0A310, 0x2C96F4, 0x100D3A04, 0x23E0504,
	L"14 夜空のユーフォーロマンス.wav",    0x124B3F08, 0x13B000, 0x125EEF08, 0x1DE8800,
	L"15 平安のエイリアン.wav",            0x143D7708, 0x10D400, 0x144E4B08, 0x1E38C00,
	L"16 妖怪寺.wav",                      0x1631D708, 0x10C430, 0x16429B38,  0x813B50,
	L"17 空の帰り道 ～ Sky Dream.wav",     0x16C3D688,  0xE4EC0, 0x16D22548, 0x1253C60,
	L"18 プレイヤーズスコア.wav",          0x17F761A8, 0x1CACC8, 0x18140E70,  0x4A2FF8
};

const PAIR offset_th13 [] = {
	L"01 欲深き霊魂.wav",                           0x10,  0x3ACD80,   0x3ACD90,  0x6A3080,
	L"02 死霊の夜桜.wav",                       0xA4FE10,  0x2E72A8,   0xD370B8,  0xF618C8,
	L"03 ゴーストリード.wav",                  0x25BCF38,   0xAED00,  0x266BC38,  0xD89C00,
	L"04 妖怪寺へようこそ.wav",                0x3B11CB8,   0x61580,  0x3B73238, 0x119D0C0,
	L"05 門前の妖怪小娘.wav",                  0x560F618,   0x53D00,  0x5663318,  0xBCC5C0,
	L"06 素敵な墓場で暮しましょ.wav",          0x683FA38,  0x297B20,  0x6AD7558, 0x14DA2E0,
	L"07 リジッドパラダイス.wav",              0x8B6A738,   0xACD40,  0x8C17478,  0xC57D40,
	L"08 デザイアドライブ.wav",                0x9EF16F8,  0x188200,  0xA0798F8, 0x18BE300,
	L"09 古きユアンシェン.wav",                0xC65AE78,  0x173180,  0xC7CDFF8, 0x1002600,
	L"10 夢殿大祀廟.wav",                      0xE08B1B8,   0x43430,  0xE0CE5E8, 0x1366370,
	L"11 大神神話伝.wav",                      0xFE09528,   0xA1800,  0xFEAAD28, 0x18EA800,
	L"12 小さな欲望の星空.wav",               0x1245B528,  0x2B3E40, 0x1270F368, 0x1038F60,
	L"13 聖徳伝説 ～ True Administrator.wav", 0x140BE998,  0x27B4D8, 0x14339E70, 0x1C92D08,
	L"14 妖怪裏参道.wav",                     0x187E7178,  0x163C80, 0x1894ADF8, 0x15846A0,
	L"15 佐渡の二ッ岩.wav",                   0x1AA43628,  0x249AA0, 0x1AC8D0C8, 0x1611880,
	L"16 神社の新しい風.wav",                 0x15FCCB78,   0x3F600, 0x1600C178,  0xE8FC00,
	L"17 デザイアドリーム.wav",               0x16E9BD78, 0x170D100, 0x185A8E78,  0x23E300,
	L"18 プレイヤーズスコア.wav",             0x1CECC2D8,  0x1CACC8, 0x1D096FA0,  0x4A2FF8
};

typedef struct
{
	int bgm_num;
	const PAIR *pp;
} BGMIDX, *PBGMIDX;

BGMIDX Index [] = {
	20, offset_th07,
	34, offset_th075,
	21, offset_th08,
	19, offset_th09,
	06, offset_th095,
	18, offset_th10,
	00, 0,
	18, offset_th11,
	18, offset_th12,
	00, 0,
	00, 0,
	00, 0,
	18, offset_th13,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
	00, 0,
};


wchar_t GameName[][MAXPATH] = {
	L"東方妖々夢　～ Perfect Cherry Blossom.",
	L"東方萃夢想　～ Immaterial and Missing Power."
	L"東方永夜抄　～ Imperishable Night.",
	L"東方花映塚　～ Phantasmagoria of Flower View.",
	L"東方文花帖　～ Shoot the Bullet."
	L"東方風神録　～ Mountain of Faith.",
	L"", // L"東方緋想天　～ Scarlet Weather Rhapsody."
	L"東方地霊殿　～ Subterranean Animism.",
	L"東方星蓮船　～ Undefined Fantastic Object.",
	L"", // L"東方非想天則　～ 超弩級ギニョルの謎を追え",
	L"", // L"ダブルスポイラー　～ 東方文花帖",
	L"", // L"妖精大戦争　～ 東方三月精",
	L"東方神霊廟　～ Ten Desires.",
	L"", // L"東方輝針城　～ Double Dealing Character."
	L"",  // L"弹幕天邪鬼　～ Impossible Spell Card."
	L"",  // L"东方深秘录　～ Urban Legend in Limbo."
	L"",  // L"东方心绮楼　～ Hopeless Masquerade."
	L""  // L"東方輝針城　～ Double Dealing Character."
};