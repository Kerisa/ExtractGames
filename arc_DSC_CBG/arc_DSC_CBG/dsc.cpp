#include "dsc.h"

int Is_DSC(const u8* in)
{
	return !memcmp(in, "DSC FORMAT 1.00", 16);
}

u32 new_key(u32 *pkey)
{
	u32 edx, eax;

	edx = 0x4e35 * (*pkey & 0xffff);
	eax = 0x4e35 * (*pkey >> 16) + 0x15a * (*pkey) + (edx >> 0x10);
	*pkey = (edx & 0xffff) + ((eax & 0xffff) << 0x10) + 1;
	return (eax & 0x7fff);
}

void sort_arc(u32* arr, int len)
{
	int i, j;
	u32 t;
	for (i=0; i<len-1; ++i)
		for (j=i+1; j<len; ++j)
			if (arr[i] > arr[j])
			{
				t = arr[i]; arr[i] = arr[j]; arr[j] = t;
			}
}

struct shiranai
{
	u32 mem1;
	u32 mem2;
	u32 a[2];
};

int DecodeDSC(u8** raw_data, u8* in, int in_size)
{
	u32 arr1[1024] = {0};
	u32 arr2[2048] = {0};
	struct shiranai arr3[2048];

	if (!Is_DSC(in))
		return ERR_FILE_DISMATCH;

	u8 *out;
	u32 key		 = *((u32*)in + 4);		// ////////////////
	u32 out_size = *((u32*)in + 5);

	if (out_size > 0x2000000)	// 32MB
		return ERR_FILE_ERROR;

	if (!(out = (u8*)malloc(out_size)))
		return ERR_MEM;

	memset(arr3, 0, sizeof(struct shiranai)*1024);

	u32 len = 0;
	for (int i=0; i<512; ++i)
	{
		u8 byte = in[0x20+i] - (u8)new_key(&key);
		if (byte)
			arr1[len++] = (byte << 16) + i;
	}
	const u32 arr1_len = len;
	sort_arc(arr1, arr1_len);

{
	arr1[arr1_len] = 0; // arr2[0] = 0;		sizeof(arr2) = 1024 ? 1025
	
	u32 v20 = 0, v18 = 0;
	u32 v10 = 1, edi = 1;
	u32 *ecx = arr2;
	
	u32 *v24, *ebx;
	u32 v, v1c, esi;

	for (v1c=0; v1c<arr1_len; ++v18)
	{
		v20 ^= 0x200;
		v24 = ebx = &arr2[v20];
		esi = 0;
		for(;v18 == (arr1[v1c]>>16); ++ecx, ++v1c, ++esi)
		{
			arr3[*ecx].mem1 = 0;
			arr3[*ecx].mem2 = arr1[v1c] & 0x1ff;
		}

		v = 2*(v10 - esi);
		if (v10 > esi)
		{
			v10 -= esi;
			do{
				arr3[*ecx].mem1 = 1;
				for (int dd=0; dd<2; ++dd, ++edi, ++ebx)
				{
					arr3[*ecx].a[dd] = *ebx = edi;
				}
				++ecx;
			}while (--v10);
		}
		v10 = v;
		ecx = v24;
	}
}									// ¡üok¡ü  DSC_Process1
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	u8* in_ptr = in + 0x220;
	u8* in_end = in + in_size;

	u32 ebx = 0, v20 = 0;
	u8  byte = 0;
	
	u32 _edi;
	int _v1e;
	u16 _v1c;

	while (in_ptr < in_end && v20 < out_size )// vc< out_size 1420
	{
		u32 ecx = 0;
		do{
			if (!ebx)
			{
				byte = *in_ptr++;
				ebx = 8;
			}
			--ebx;
			ecx = arr3[ecx].a[byte >> 7];	////////////////////////////////
			byte <<= 1;						////////////////////////////////
		}while(arr3[ecx].mem1);
		
		_v1c = arr3[ecx].mem2 & 0xffff;
		if (_v1c >> 8 == 1)
		{
			u32 byte_1 = byte >> (8 - ebx);
			u32 esi = ebx;
			
			if (ebx < 12)
			{
				u32 ecx_1 = ((11 - ebx) >> 3) + 1;
				esi = (ecx_1 << 3) + ebx;
				do
				{
					byte_1 = (byte_1 << 8) + *in_ptr++;		// byte ¸Ä byte_1
				} while (--ecx_1);
			}
			ebx = esi - 12;

			byte = (u8)(byte_1 << (8 - ebx));
			_v1e = ((u32)byte_1 >> ebx);	/////////////////////////////
			_edi = v20 - _v1e - 2;

			for (int m=(u8)_v1c+2; m; --m)
				out[v20++] = out[_edi++];
		}
		else
		{
			out[v20++] = (u8)_v1c;
		}
	}
	*raw_data = out;
	return out_size;
}

int Is_DSC_Image(const u8 const *raw_data, u32 len)
{
	do{
		if (len < 16) break;

		if (*(u16*)raw_data == 0) break;

		if (*(u16*)(raw_data+2) == 0) break;

		if (*(raw_data+4) == 0 || *(raw_data+4) != 8 && *(raw_data+4) != 24 &&  *(raw_data+4) != 32)
			 break;

		int i;
		for (i=5; i<16; ++i)
			if (*(raw_data+i) != 0)
				 break;
		if (i != 16) break;

		return 1;
	}while(0);

	return 0;
}

u8* Save_DSC_Bmp(const u8 *raw_data, u32 len, u32 *bmp_len)
{
	const u32 arr_len = sizeof(BMP_head_raw)/sizeof(BMP_head_raw[0]);

	u32 width  = *(u16*)(raw_data);
	u32 height = *(u16*)(raw_data+2);
	u32 bpp	   = *(raw_data+4);

	*bmp_len = arr_len + width * height * bpp;
	u8 *bmp = (u8*)malloc(*bmp_len);
	if (!bmp) return 0;

	memcpy(bmp, BMP_head_raw, arr_len);

	*(u32*)(bmp + 0x02) = *bmp_len;
	*(u32*)(bmp + 0x12) = width;
	*(u32*)(bmp + 0x16) = height;

	if (bpp == 32)
	{
		const u32 *data_p = (const u32*)raw_data + 4 + width * (height-1);
		u32 *p = (u32*)(bmp + arr_len);

		while (data_p >= (const u32*)raw_data + 4)
		{
			memcpy((void*)p, data_p, 4*width);
			p += width;
			data_p -= width;
		}
	}
	else if (bpp == 24)
	{
		const u32 *data_p = (const u32*)(raw_data + 0x10 + width * (height-1) * 3);
		u32 *dst = (u32*)(bmp + arr_len);

		while (data_p >= (const u32*)raw_data + 4)
		{
			const u32 *src = (const u32*)data_p;
			for (int i=0; i<width; ++i)
			{
				u32 t = *src | 0xff000000;
				src = (u32*)((u8*)src + 3);
				*dst++ = t;
			}
			data_p = (u32*)((u8*)data_p - 3 * width);
		}
	}
	else if (bpp == 8)
	{
		*(u32*)(bmp + 0x1c) = 8;

		const u8 *data_p = raw_data + 0x10 + width * (height-1);
		
		u8 *p = bmp + arr_len;
		while (data_p >= raw_data + 0x10)
		{
			memcpy(p, data_p, width);
			data_p -= width;
			p += width;
		}
	}

	return bmp;
}