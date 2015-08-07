#include "cbg.h"

int Is_CBG(const u8* in)
{
	return memcmp(in, "CompressedBG___", 16) == 0 ? *(u16*)(in+0x2E) : 0;	// 返回CBG的版本（1或2）
}

struct ARR2
{
	u32 mem1;
	u32 mem2;
	u32 mem3;
	u32 mem4;
	u32 mem5;
	u32 mem6;
};

int Process_1(u32 *arr1, struct ARR2 *arr2)
{
	u32 v10, edi, edx, eax = 0x100;
	u32 v14[2];
	struct ARR2 tmp;

	v10 = edi = edx = 0;

	for (int m=0; m<256; ++m)
	{
		arr2[m].mem1 = arr1[m] > 0;
		arr2[m].mem2 = arr1[m];
		arr2[m].mem3 = 0;
		arr2[m].mem4 = -1;
		arr2[m].mem5 = m;
		arr2[m].mem6 = m;

		v10 += arr1[m];
	}
	if (!v10) return -1;

	tmp.mem1 = 0;	tmp.mem2 = 0;	tmp.mem3 = 1;
	tmp.mem4 = -1;	tmp.mem5 = -1;	tmp.mem6 = -1;

	for (edx=256; edx<511; ++edx)
		arr2[edx] = tmp;

	while(1)
	{
		for (int m=0; m<2; ++m)
		{
			edi = v14[m] = 0xffffffff;
			for (int n=0; n<eax; ++n)
				if (arr2[n].mem1 && arr2[n].mem2 < edi)
				{
					v14[m] = n;
					edi = arr2[n].mem2;
				}

			if (v14[m] != 0xffffffff)
			{
				arr2[v14[m]].mem1 = 0;
				arr2[v14[m]].mem4 = eax;
			}
		}

		tmp.mem1 = 1;
		tmp.mem2 = (v14[1] != 0xffffffff ? arr2[v14[1]].mem2 : 0) + arr2[v14[0]].mem2;
		tmp.mem3 = 1;
		tmp.mem4 = -1;
		tmp.mem5 = v14[0];
		tmp.mem6 = v14[1];

		arr2[eax++] = tmp; 		// eax相当于v38
	
		if (tmp.mem2 == v10)
			break;
	}
	return eax-1;
}

u32 Get_pixel(const u32* const data, u32 bpp)
{
	if (bpp == 32)
		return *data;
	else if (bpp == 24)
		return (0xff000000 | *data);
	else if (bpp == 8)
		return *(u8*)data;
	else return 0;
}

u32 pixel_add(u32 x, u32 y)
{
	u64 tmp = ((u64)x & 0xff000000) + ((u64)y & 0xff000000);
	u32 a = (tmp > 0xff000000 ? 0xff000000 : tmp ) & 0xff000000;
	u32 r = (((x & 0x00FF0000) + (y & 0x00FF0000))) & 0x00FF0000;
	u32 g = (((x & 0x0000FF00) + (y & 0x0000FF00))) & 0x0000FF00;
	u32 b = (((x & 0x000000FF) + (y & 0x000000FF))) & 0x000000FF;
	
	return (r | g | b | a);
}

u32 pixel_avg(u32 x, u32 y)
{
	u32 a = (((u64)x & 0xff000000) + ((u64)y & 0xff000000)) / 2 & 0xff000000;
	u32 r = (((x & 0x00FF0000) + (y & 0x00FF0000)) / 2) & 0x00FF0000;
	u32 g = (((x & 0x0000FF00) + (y & 0x0000FF00)) / 2) & 0x0000FF00;
	u32 b = (((x & 0x000000FF) + (y & 0x000000FF)) / 2) & 0x000000FF;
	
	return (r | g | b | a);
}

int DecodeCBG(u8** raw_data, u8* in, int in_size)
{
	u32 _vc, _v10, _v14, _v18;
	u32 out_size;
	u32 arr1[256];
	struct ARR2 arr2[511];

	u32 width			= *(u16*)(in + 0x10);	// 图像宽
	u32 height			= *(u16*)(in + 0x12);	// 图像高
	u32 bpp				= *(u32*)(in + 0x14);	// 每像素位数
	u8  check_sum		= *(in + 0x2c);
	u8  check_xor		= *(in + 0x2d);

	_vc  = *((u32*)in + 4);		// in + 0x10
	_v10 = *((u32*)in + 5);		// in + 0x14
	_v14 = *((u32*)in + 6);		// in + 0x18
	_v18 = *((u32*)in + 7);		// in + 0x1c

	out_size = (_vc & 0xfff) * ((_v10 & 0xffff) >> 3) * (_vc >> 16) + 10;

	u32 key		 = *(u32*)(in + 0x24);		// in + 0x24
	u32 buf_size = *(u32*)(in + 0x28);		// in + 0x28
	u8* buf		 = (u8*)malloc(buf_size);	// MemA
	u32 buf_cur;

	if (!buf) return ERR_MEM;

	memcpy(buf, in+0x30, buf_size);

	u8 dl, bl;
	for (dl=bl=buf_cur=0; buf_cur<buf_size; ++buf_cur)
	{
		buf[buf_cur] -= new_key(&key);
		dl += buf[buf_cur];
		bl ^= buf[buf_cur];
	}

	if (dl != check_sum || bl != check_xor)
	{
		free(buf);
		return ERR_FILE_ERROR;
	}

	u8* buf_p = buf;
	for (int edx=0; edx<0x100; ++edx)
	{
		u32 esi = 0, ebp = 0;
		u8  al;
		do
		{
			al = *buf_p++;
			ebp |= ((al & 0x7f) << /*(u8)*/esi);
			esi += 7;
		}while(al & 0x80 && buf_p - buf < buf_size);
		arr1[edx] = ebp;
	}
	free(buf);

	u32 Pro_1_ret_vlaue = Process_1(arr1, arr2);
	u32 buf1_len = *(u32*)(in + 0x20);
	u8* buf1 = (u8*)malloc(buf1_len*4);
	if (!buf1) return ERR_MEM;

	u8 *in_p = in + 0x30 + buf_size;		// v18 地址
	u8 mask = 0x80;
	for (int m=0; m<buf1_len; ++m)
	{
		u32 eax = Pro_1_ret_vlaue;
		if (arr2[Pro_1_ret_vlaue].mem3 == 1)
		{
			do{
				if (*in_p & mask)
					 eax = arr2[eax].mem6;
				else eax = arr2[eax].mem5;
				mask >>= 1;
				if (!mask)
				{
					++in_p;
					mask = 0x80;
				}
			}while (arr2[eax].mem3 == 1);
		}
		buf1[m] = (u8)eax;
	}

	u8* buf2 = (u8*)malloc(width*height*4);	// data3
	u8* buf2_p = buf2;
	u32 flag = 1;
	for (int edx=0; edx<buf1_len; )
	{
		u32 ebp = 0;
		u8  tmp, esi = 0;
		do{
			ebp |= ((buf1[edx] & 0x7f) << (u8)esi);
			tmp = buf1[edx] & 0x80;
			esi += 7;
			++edx;
		}while(tmp);
		u32 ecx = ebp;		// len
		if (!flag)
		{
			while(ecx--) *buf2_p++ = 0;
		}
		else
		{
			while(ecx--) *buf2_p++ = buf1[edx++];
		}
		flag ^= 1;
	}
	free(buf1);	//free MemC
										////// 解码完毕
	//////////////////////////////////////////////////////////////////////////
	u32 bytes_per_line	= width * bpp / 8;
	u32 bytes_per_pixel	= bpp / 8;

	u32 *data   = (u32*)malloc((bytes_per_pixel>1?4:1) * width * height);	// 32位bmp, 或8位(rule等单色图)
	if (!data) return ERR_MEM;

	buf2_p = buf2;
	u32* data_p = data;

	memset(data, 0, (bytes_per_pixel>1?4:1) * width * height);

	int ttmp = 0;
	if (bpp == 32 || bpp == 24)
	{
		for (int i=0; i<width; ++i)		// 预处理第1行
		{
			ttmp = pixel_add(ttmp, Get_pixel((u32*)buf2_p, bpp));
			*data_p++ = ttmp;
			if (bpp == 32) buf2_p += 4;
			else buf2_p += 3;
		}

		for (int i=1; i<height; ++i)
		{
			ttmp = pixel_add(Get_pixel((u32*)buf2_p, bpp), *(data_p - width));	// 预处理每行第1列
			*data_p++ = ttmp;
			if (bpp == 32) buf2_p += 4;
			else buf2_p += 3;
	
			for (int j=1; j<width; ++j)
			{
				ttmp = pixel_add(pixel_avg(ttmp, *(data_p-width)), Get_pixel((u32*)buf2_p, bpp));
				*data_p++ = ttmp;
				if (bpp == 32) buf2_p += 4;
				else buf2_p += 3;
			}
		}
	}
	else if (bpp == 8)
	{
		u8 ttmp = 0;
		for (int i=0; i<width; ++i)		// 预处理第1行
		{
			ttmp = pixel_add((u32)ttmp, Get_pixel((u32*)buf2_p, bpp));
			buf2_p++;
			*(u8*)data_p = ttmp;
			data_p = (u32*)((u8*)data_p + 1);
		}

		for (int i=1; i<height; ++i)
		{
			ttmp = pixel_add(Get_pixel((u32*)buf2_p, bpp), (u32)*((u8*)data_p-width));	// 预处理每行第1列
			buf2_p++;
			*(u8*)data_p = ttmp;
			data_p = (u32*)((u8*)data_p + 1);
				
			for (int j=1; j<width; ++j)
			{
				ttmp = pixel_add(pixel_avg(ttmp, (u32)*((u8*)data_p-width)), Get_pixel((u32*)buf2_p, bpp));
				buf2_p++;
				*(u8*)data_p = (u8)ttmp;
				data_p = (u32*)((u8*)data_p + 1);
			}
		}
	}
	free(buf2);

	u8* bmp;
	u32 file_size;
	if (bpp == 32 || bpp == 24)
	{
	//==========================	加入bmp文件头，默认颜色位数为32位, 颠倒的bmp
		file_size = 4*width*height + sizeof(BMP_head_raw)/sizeof(u8);
		bmp = (u8*)malloc(file_size);
		if (!bmp) return ERR_MEM;

		memcpy(bmp, BMP_head_raw, sizeof(BMP_head_raw)/sizeof(u8));

		*(u32*)(bmp + 0x02) = file_size;
		*(u32*)(bmp + 0x12) = width;
		*(u32*)(bmp + 0x16) = 0 - height;

		memcpy(bmp+sizeof(BMP_head_raw)/sizeof(u8), data, 4*width*height); //memcpy((void*)p, data_p, 4*width);
	//==========================
	}
	else if (bpp == 8)
	{
		file_size = width*height + sizeof(BMP_head_raw_v8)/sizeof(u8);
		bmp = (u8*)malloc(file_size);
		if (!bmp) return ERR_MEM;

		memcpy(bmp, BMP_head_raw_v8, sizeof(BMP_head_raw_v8)/sizeof(u8));

		*(u32*)(bmp + 0x02) = file_size;
		*(u32*)(bmp + 0x12) = width;
		*(u32*)(bmp + 0x16) = 0 - height;
		*(u32*)(bmp + 0x22) = file_size - 0x436;

		memcpy(bmp+sizeof(BMP_head_raw_v8)/sizeof(u8), data, width*height);
	}
	free(data);

	if (raw_data)
		*raw_data = (u8*)bmp;
	else *raw_data = 0;

	return file_size;
}