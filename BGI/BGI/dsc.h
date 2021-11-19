#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "error.h"

extern const u8 BMP_head_raw[54];

int Is_DSC(const u8* in);
u32 new_key(u32 *pkey);
int DecodeDSC(u8** raw_data, u8* in, int in_size);
int Is_DSC_Image(const u8 *raw_data, u32 len);
u8* Save_DSC_Bmp(const u8 *raw_data, u32 len, u32 *bmp_len);