#include "cxdec.h"

static int xcode_building_stage0(struct cxdec_xcode_status *xcode, int stage);
static int xcode_building_stage1(struct cxdec_xcode_status *xcode, int stage);

static BYTE EncryptionControlBlock[4096] = {
        0x20, 0x45, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x63, 0x6f, 0x6e, 0x74, 
        0x72, 0x6f, 0x6c, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x20, 0x2d, 0x2d, 0x20, 0x53, 0x74, 0x61, 
        0x74, 0x69, 0x63, 0x61, 0x6c, 0x6c, 0x79, 0x20, 0x6f, 0x72, 0x20, 0x64, 0x79, 0x6e, 0x61, 0x6d, 
        0x69, 0x63, 0x61, 0x6c, 0x6c, 0x79, 0x2c, 0x20, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x6c, 0x79, 
        0x20, 0x6f, 0x72, 0x20, 0x69, 0x6e, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x6c, 0x79, 0x2c, 0x20, 
        0x75, 0x73, 0x69, 0x6e, 0x67, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, 
        0x61, 0x6d, 0x20, 0x61, 0x6e, 0x64, 0x2f, 0x6f, 0x72, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x20, 
        0x66, 0x72, 0x6f, 0x6d, 0x20, 0x6f, 0x74, 0x68, 0x65, 0x72, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, 
        0x61, 0x6d, 0x73, 0x20, 0x77, 0x69, 0x6c, 0x6c, 0x20, 0x62, 0x65, 0x20, 0x69, 0x6c, 0x6c, 0x65, 
        0x67, 0x61, 0x6c, 0x20, 0x62, 0x79, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6c, 0x69, 0x63, 0x65, 0x6e, 
        0x73, 0x65, 0x20, 0x61, 0x67, 0x72, 0x65, 0x65, 0x6d, 0x65, 0x6e, 0x74, 0x2e, 0x20, 0x82, 0xb1, 
        0x82, 0xcc, 0x83, 0x76, 0x83, 0x8d, 0x83, 0x4f, 0x83, 0x89, 0x83, 0x80, 0x82, 0xe2, 0x83, 0x75, 
        0x83, 0x8d, 0x83, 0x62, 0x83, 0x4e, 0x82, 0xf0, 0x81, 0x41, 0x90, 0xc3, 0x93, 0x49, 0x82, 0xc5, 
        0x82, 0xa0, 0x82, 0xea, 0x93, 0xae, 0x93, 0x49, 0x82, 0xc5, 0x82, 0xa0, 0x82, 0xea, 0x81, 0x41, 
        0x92, 0xbc, 0x90, 0xda, 0x93, 0x49, 0x82, 0xc5, 0x82, 0xa0, 0x82, 0xea, 0x8a, 0xd4, 0x90, 0xda, 
        0x93, 0x49, 0x82, 0xc5, 0x82, 0xa0, 0x82, 0xea, 0x81, 0x41, 0x91, 0xbc, 0x82, 0xcc, 0x83, 0x76, 
        0x83, 0x8d, 0x83, 0x4f, 0x83, 0x89, 0x83, 0x80, 0x82, 0xa9, 0x82, 0xe7, 0x97, 0x70, 0x82, 0xa2, 
        0x82, 0xe9, 0x82, 0xb1, 0x82, 0xc6, 0x82, 0xcd, 0x83, 0x89, 0x83, 0x43, 0x83, 0x5a, 0x83, 0x93, 
        0x83, 0x58, 0x82, 0xc9, 0x82, 0xe6, 0x82, 0xe8, 0x8b, 0xd6, 0x82, 0xb6, 0x82, 0xe7, 0x82, 0xea, 
        0x82, 0xc4, 0x82, 0xa2, 0x82, 0xdc, 0x82, 0xb7, 0x81, 0x42, 0x0a, 0x93, 0xfa, 0x96, 0x7b, 0x82, 
        0xc5, 0x97, 0x42, 0x88, 0xea, 0x82, 0xcc, 0x8f, 0xed, 0x89, 0xc4, 0x82, 0xcc, 0x93, 0x87, 0x81, 
        0x77, 0x93, 0x83, 0x8c, 0xb7, 0x93, 0x87, 0x81, 0x78, 0x0a, 0x89, 0xab, 0x93, 0xea, 0x82, 0xc6, 
        0x95, 0xcf, 0x82, 0xed, 0x82, 0xe7, 0x82, 0xc8, 0x82, 0xa2, 0x88, 0xdc, 0x93, 0x78, 0x82, 0xc9, 
        0x82, 0xa0, 0x82, 0xe8, 0x82, 0xc8, 0x82, 0xaa, 0x82, 0xe7, 0x88, 0xea, 0x94, 0x4e, 0x82, 0xf0, 
        0x92, 0xca, 0x82, 0xb5, 0x82, 0xc4, 0x90, 0x5e, 0x89, 0xc4, 0x82, 0xc9, 0x8b, 0xdf, 0x82, 0xa2, 
        0x8b, 0x43, 0x89, 0xb7, 0x82, 0xf0, 0x95, 0xdb, 0x82, 0xc2, 0x95, 0x73, 0x8e, 0x76, 0x8b, 0x63, 
        0x82, 0xc8, 0x93, 0x87, 0x82, 0xc5, 0x82, 0xa0, 0x82, 0xe9, 0x81, 0x42, 0x0a, 0x89, 0xbd, 0x8c, 
        0xcc, 0x82, 0xbb, 0x82, 0xf1, 0x82, 0xc8, 0x95, 0x73, 0x8e, 0x76, 0x8b, 0x63, 0x82, 0xaa, 0x8b, 
        0x4e, 0x82, 0xb1, 0x82, 0xe8, 0x93, 0xbe, 0x82, 0xe9, 0x82, 0xcc, 0x82, 0xa9, 0x81, 0x48, 0x0a, 
        0x82, 0xbb, 0x82, 0xea, 0x82, 0xcd, 0x8a, 0x43, 0x92, 0xea, 0x89, 0xce, 0x8e, 0x52, 0x82, 0xa9, 
        0x82, 0xe7, 0x82, 0xad, 0x82, 0xe9, 0x8a, 0x43, 0x97, 0xac, 0x82, 0xcc, 0x89, 0x65, 0x8b, 0xbf, 
        0x82, 0xc8, 0x82, 0xc7, 0x82, 0xc6, 0x8c, 0xbe, 0x82, 0xed, 0x82, 0xea, 0x82, 0xc4, 0x82, 0xa2, 
        0x82, 0xaa, 0x81, 0x41, 0x82, 0xc7, 0x82, 0xea, 0x82, 0xe0, 0x89, 0x5c, 0x82, 0xcc, 0x88, 0xe6, 
        0x82, 0xf0, 0x8f, 0x6f, 0x82, 0xb8, 0x81, 0x41, 0x90, 0x5e, 0x8e, 0xc0, 0x82, 0xf0, 0x92, 0x6d, 
        0x82, 0xe9, 0x8e, 0xd2, 0x82, 0xcd, 0x82, 0xa2, 0x82, 0xc8, 0x82, 0xa2, 0x81, 0x63, 0x81, 0x63, 
        0x0a, 0x0a, 0x28, 0x63, 0x29, 0x32, 0x30, 0x30, 0x38, 0x59, 0x55, 0x5a, 0x55, 0x53, 0x4f, 0x46, 
        0x54, 0x2f, 0x4a, 0x55, 0x4e, 0x4f, 0x53, 0x69, 0x6e, 0x63, 0x2e, 0x0a, 0x0a, 0x81, 0x7c, 0x8a, 
        0x9d, 0x89, 0x48, 0x96, 0xeb, 0x82, 0xcc, 0x8f, 0xcd, 0x81, 0x7c, 0x0a, 0x0a, 0x93, 0x7e, 0x8b, 
        0x78, 0x82, 0xdd, 0x82, 0xf0, 0x97, 0x98, 0x97, 0x70, 0x82, 0xb5, 0x81, 0x41, 0x93, 0x87, 0x82, 
        0xc5, 0x83, 0x79, 0x83, 0x93, 0x83, 0x56, 0x83, 0x87, 0x83, 0x93, 0x82, 0xf0, 0x8c, 0x6f, 0x89, 
        0x63, 0x82, 0xb7, 0x82, 0xe9, 0x8f, 0x66, 0x95, 0xea, 0x82, 0xcc, 0x82, 0xe0, 0x82, 0xc6, 0x82, 
        0xc9, 0x96, 0x4b, 0x82, 0xea, 0x82, 0xbd, 0x81, 0x77, 0x92, 0xa9, 0x91, 0x71, 0x91, 0x73, 0x91, 
        0xbe, 0x81, 0x78, 0x0a, 0x83, 0x79, 0x83, 0x93, 0x83, 0x56, 0x83, 0x87, 0x83, 0x93, 0x82, 0xc5, 
        0x83, 0x41, 0x83, 0x8b, 0x83, 0x6f, 0x83, 0x43, 0x83, 0x67, 0x82, 0xf0, 0x82, 0xb5, 0x82, 0xc8, 
        0x82, 0xaa, 0x82, 0xe7, 0x8f, 0x5d, 0x96, 0x85, 0x82, 0xc5, 0x82, 0xa0, 0x82, 0xe9, 0x81, 0x77, 
        0x8e, 0x4f, 0x8d, 0x44, 0x97, 0x52, 0x94, 0xe4, 0x8e, 0x71, 0x81, 0x78, 0x81, 0x41, 0x81, 0x77, 
        0x91, 0x6f, 0x97, 0x74, 0x81, 0x78, 0x8e, 0x6f, 0x96, 0x85, 0x82, 0xe2, 0x81, 0x41, 0x97, 0x63, 
        0x93, 0xe9, 0x90, 0xf5, 0x82, 0xcc, 0x81, 0x77, 0x98, 0x5a, 0x8a, 0x70, 0x8c, 0xdc, 0x98, 0x59, 
        0x81, 0x78, 0x82, 0xc6, 0x8a, 0x79, 0x82, 0xb5, 0x82, 0xa2, 0x93, 0xfa, 0x81, 0x58, 0x82, 0xf0, 
        0x89, 0xdf, 0x82, 0xb2, 0x82, 0xb5, 0x82, 0xc4, 0x82, 0xa2, 0x82, 0xad, 0x81, 0x42, 0x0a, 0x0a, 
        0x82, 0xbb, 0x82, 0xf1, 0x82, 0xc8, 0x92, 0x86, 0x81, 0x41, 0x91, 0x73, 0x91, 0xbe, 0x82, 0xcd, 
        0x97, 0xa7, 0x82, 0xbf, 0x8a, 0xf1, 0x82, 0xc1, 0x82, 0xbd, 0x8b, 0x69, 0x92, 0x83, 0x93, 0x58, 
        0x82, 0xc5, 0x88, 0xea, 0x90, 0x6c, 0x82, 0xcc, 0x8f, 0xad, 0x8f, 0x97, 0x82, 0xc6, 0x8f, 0x6f, 
        0x89, 0xef, 0x82, 0xc1, 0x82, 0xbd, 0x81, 0x42, 0x0a, 0x8b, 0x43, 0x82, 0xb3, 0x82, 0xad, 0x82, 
        0xc5, 0x96, 0xbe, 0x82, 0xe9, 0x82, 0xa2, 0x81, 0x77, 0x8f, 0xe3, 0x8d, 0xe2, 0x8a, 0x9d, 0x89, 
        0x48, 0x96, 0xeb, 0x81, 0x78, 0x82, 0xc6, 0x82, 0xa2, 0x82, 0xa4, 0x8f, 0xad, 0x8f, 0x97, 0x82, 
        0xc6, 0x8c, 0xf0, 0x97, 0xac, 0x82, 0xf0, 0x90, 0x5b, 0x82, 0xdf, 0x82, 0xe9, 0x91, 0x73, 0x91, 
        0xbe, 0x81, 0x42, 0x0a, 0x8a, 0x79, 0x82, 0xb5, 0x82, 0xb0, 0x82, 0xc9, 0x98, 0x62, 0x82, 0xb7, 
        0x82, 0xa4, 0x82, 0xbf, 0x82, 0xc9, 0x81, 0x41, 0x91, 0x73, 0x91, 0xbe, 0x82, 0xcd, 0x94, 0xde, 
        0x8f, 0x97, 0x82, 0xa9, 0x82, 0xe7, 0x95, 0x83, 0x90, 0x65, 0x82, 0xcc, 0x92, 0x61, 0x90, 0xb6, 
        0x93, 0xfa, 0x83, 0x76, 0x83, 0x8c, 0x83, 0x5b, 0x83, 0x93, 0x83, 0x67, 0x82, 0xcc, 0x91, 0x8a, 
        0x92, 0x6b, 0x82, 0xf0, 0x8e, 0xf3, 0x82, 0xaf, 0x82, 0xe9, 0x81, 0x42, 0x0a, 0x0a, 0x96, 0xbe, 
        0x8c, 0xe3, 0x93, 0xfa, 0x82, 0xc9, 0x92, 0x61, 0x90, 0xb6, 0x93, 0xfa, 0x82, 0xf0, 0x8d, 0x54, 
        0x82, 0xa6, 0x82, 0xc8, 0x82, 0xaa, 0x82, 0xe7, 0x82, 0xe0, 0x81, 0x41, 0x82, 0xdc, 0x82, 0xbe, 
        0x91, 0xa1, 0x82, 0xe9, 0x83, 0x76, 0x83, 0x8c, 0x83, 0x5b, 0x83, 0x93, 0x83, 0x67, 0x82, 0xc9, 
        0x8c, 0x88, 0x82, 0xdf, 0x82, 0xa9, 0x82, 0xcb, 0x82, 0xc4, 0x82, 0xa2, 0x82, 0xe9, 0x8a, 0x9d, 
        0x89, 0x48, 0x96, 0xeb, 0x81, 0x42, 0x0a, 0x91, 0x73, 0x91, 0xbe, 0x82, 0xcd, 0x90, 0x5e, 0x8c, 
        0x95, 0x82, 0xc8, 0x94, 0xde, 0x8f, 0x97, 0x82, 0xcc, 0x91, 0x8a, 0x92, 0x6b, 0x82, 0xc9, 0x8f, 
        0xe6, 0x82, 0xe8, 0x81, 0x41, 0x83, 0x41, 0x83, 0x68, 0x83, 0x6f, 0x83, 0x43, 0x83, 0x58, 0x82, 
        0xf0, 0x8d, 0x73, 0x82, 0xa2, 0x81, 0x41, 0x82, 0xc8, 0x82, 0xf1, 0x82, 0xc6, 0x82, 0xa9, 0x83, 
        0x76, 0x83, 0x8c, 0x83, 0x5b, 0x83, 0x93, 0x83, 0x67, 0x82, 0xcc, 0x8e, 0xe8, 0x8d, 0xec, 0x82, 
        0xe8, 0x83, 0x50, 0x81, 0x5b, 0x83, 0x4c, 0x82, 0xf0, 0x8d, 0xec, 0x82, 0xe8, 0x8f, 0xe3, 0x82, 
        0xb0, 0x82, 0xe9, 0x82, 0xb1, 0x82, 0xc6, 0x82, 0xc9, 0x90, 0xac, 0x8c, 0xf7, 0x82, 0xb7, 0x82, 
        0xe9, 0x81, 0x42, 0x0a, 0x0a, 0x8a, 0xec, 0x82, 0xd1, 0x8d, 0x87, 0x82, 0xa4, 0x93, 0xf1, 0x90, 
        0x6c, 0x82, 0xcd, 0x81, 0x41, 0x88, 0xea, 0x8f, 0x8f, 0x82, 0xc9, 0x89, 0xdf, 0x82, 0xb2, 0x82, 
        0xb7, 0x8e, 0x9e, 0x8a, 0xd4, 0x82, 0xcc, 0x92, 0x86, 0x82, 0xc5, 0x81, 0x41, 0x8c, 0xdd, 0x82, 
        0xa2, 0x82, 0xc9, 0x92, 0x57, 0x82, 0xa2, 0x97, 0xf6, 0x95, 0xe7, 0x82, 0xf0, 0x95, 0xf8, 0x82, 
        0xad, 0x82, 0xe6, 0x82, 0xa4, 0x82, 0xc9, 0x82, 0xc8, 0x82, 0xc1, 0x82, 0xc4, 0x82, 0xa2, 0x82, 
        0xbd, 0x81, 0x42, 0x0a, 0x82, 0xdc, 0x82, 0xe9, 0x82, 0xc5, 0x8e, 0xe4, 0x82, 0xa9, 0x82, 0xea, 
        0x8d, 0x87, 0x82, 0xa4, 0x82, 0xb1, 0x82, 0xc6, 0x82, 0xaa, 0x89, 0x5e, 0x96, 0xbd, 0x82, 0xc5, 
        0x82, 0xa0, 0x82, 0xe9, 0x82, 0xa9, 0x82, 0xcc, 0x82, 0xe6, 0x82, 0xa4, 0x82, 0xc9, 0x81, 0x42, 
        0x0a, 0x82, 0xbb, 0x82, 0xb5, 0x82, 0xc4, 0x93, 0xf1, 0x90, 0x6c, 0x82, 0xcd, 0x81, 0x41, 0x97, 
        0x5b, 0x95, 0xe9, 0x82, 0xea, 0x82, 0xcc, 0x95, 0x6c, 0x95, 0xd3, 0x82, 0xc5, 0x93, 0xb1, 0x82, 
        0xa9, 0x82, 0xea, 0x82, 0xe9, 0x82, 0xe6, 0x82, 0xa4, 0x82, 0xc9, 0x83, 0x4c, 0x83, 0x58, 0x82, 
        0xf0, 0x8c, 0xf0, 0x82, 0xed, 0x82, 0xb7, 0x81, 0x42, 0x0a, 0x0a, 0x8f, 0xc6, 0x82, 0xea, 0x82, 
        0xc8, 0x82, 0xaa, 0x82, 0xe7, 0x91, 0x96, 0x82, 0xe8, 0x8b, 0x8e, 0x82, 0xe9, 0x8a, 0x9d, 0x89, 
        0x48, 0x96, 0xeb, 0x81, 0x42, 0x0a, 0x82, 0xbb, 0x82, 0xcc, 0x8e, 0x70, 0x82, 0xf0, 0x81, 0x41, 
        0x82, 0xe2, 0x82, 0xcd, 0x82, 0xe8, 0x8f, 0xc6, 0x82, 0xea, 0x82, 0xc8, 0x82, 0xaa, 0x82, 0xe7, 
        0x8c, 0xa9, 0x91, 0x97, 0x82, 0xe9, 0x91, 0x73, 0x91, 0xbe, 0x81, 0x42, 0x0a, 0x97, 0x82, 0x93, 
        0xfa, 0x82, 0xc9, 0x82, 0xc8, 0x82, 0xe8, 0x81, 0x41, 0x91, 0x73, 0x91, 0xbe, 0x82, 0xcd, 0x8d, 
        0xf0, 0x93, 0xfa, 0x90, 0x53, 0x82, 0xcc, 0x92, 0x86, 0x82, 0xc9, 0x90, 0xb6, 0x82, 0xdc, 0x82, 
        0xea, 0x82, 0xbd, 0x8d, 0x4b, 0x95, 0x9f, 0x8a, 0xb4, 0x82, 0xc9, 0x90, 0x5a, 0x82, 0xe8, 0x82, 
        0xc8, 0x82, 0xaa, 0x82, 0xe7, 0x81, 0x41, 0x8b, 0x69, 0x92, 0x83, 0x93, 0x58, 0x82, 0xd6, 0x82, 
        0xc6, 0x95, 0x8b, 0x82, 0xa2, 0x82, 0xbd, 0x81, 0x42, 0x0a, 0x93, 0x96, 0x91, 0x52, 0x81, 0x41, 
        0x8a, 0x9d, 0x89, 0x48, 0x96, 0xeb, 0x82, 0xc9, 0x89, 0xef, 0x82, 0xa4, 0x82, 0xbd, 0x82, 0xdf, 
        0x82, 0xc9, 0x96, 0x4b, 0x82, 0xea, 0x82, 0xbd, 0x91, 0x73, 0x91, 0xbe, 0x82, 0xbe, 0x82, 0xc1, 
        0x82, 0xbd, 0x82, 0xaa, 0x81, 0x41, 0x82, 0xbb, 0x82, 0xcc, 0x90, 0xe6, 0x82, 0xc5, 0x91, 0xcc, 
        0x8c, 0xb1, 0x82, 0xb5, 0x82, 0xbd, 0x82, 0xcc, 0x82, 0xcd, 0x91, 0x7a, 0x91, 0x9c, 0x82, 0xf0, 
        0x95, 0xa2, 0x82, 0xb7, 0x82, 0xe6, 0x82, 0xa4, 0x82, 0xc8, 0x8f, 0x6f, 0x97, 0x88, 0x8e, 0x96, 
        0x82, 0xbe, 0x82, 0xc1, 0x82, 0xbd, 0x81, 0x42, 0x0a, 0x0a, 0x81, 0x75, 0x82, 0xbb, 0x82, 0xe0, 
        0x82, 0xbb, 0x82, 0xe0, 0x81, 0x41, 0x82, 0xa0, 0x82, 0xc8, 0x82, 0xbd, 0x82, 0xcd, 0x92, 0x4e, 
        0x82, 0xc8, 0x82, 0xf1, 0x82, 0xc5, 0x82, 0xb7, 0x82, 0xa9, 0x81, 0x48, 0x81, 0x76, 0x0a, 0x0a, 
        0x8d, 0xc4, 0x89, 0xef, 0x82, 0xb5, 0x82, 0xbd, 0x8a, 0x9d, 0x89, 0x48, 0x96, 0xeb, 0x82, 0xcd, 
        0x97, 0xe2, 0x82, 0xbd, 0x82, 0xad, 0x96, 0xe2, 0x82, 0xa4, 0x81, 0x42, 0x0a, 0x82, 0xdc, 0x82, 
        0xe9, 0x82, 0xc5, 0x81, 0x41, 0x92, 0x6d, 0x82, 0xe7, 0x82, 0xc8, 0x82, 0xa2, 0x90, 0x6c, 0x8a, 
        0xd4, 0x82, 0xf0, 0x8c, 0xa9, 0x82, 0xe9, 0x82, 0xe6, 0x82, 0xa4, 0x82, 0xc8, 0x93, 0xb5, 0x82, 
        0xf0, 0x91, 0x73, 0x91, 0xbe, 0x82, 0xc9, 0x8c, 0xfc, 0x82, 0xaf, 0x82, 0xc4, 0x81, 0x63, 0x81, 
        0x63, 0x81, 0x42, 0x0a, 0x0a, 0x9c, 0xb1, 0x91, 0x52, 0x82, 0xc6, 0x82, 0xb7, 0x82, 0xe9, 0x8e, 
        0xe5, 0x90, 0x6c, 0x8c, 0xf6, 0x82, 0xcd, 0x8a, 0x9d, 0x89, 0x48, 0x96, 0xeb, 0x82, 0xcc, 0x94, 
        0xe9, 0x96, 0xa7, 0x82, 0xf0, 0x92, 0x6d, 0x82, 0xe9, 0x82, 0xb1, 0x82, 0xc6, 0x82, 0xc9, 0x82, 
        0xc8, 0x82, 0xe9, 0x81, 0x42, 0x0a, 0x8e, 0x96, 0x8c, 0xcc, 0x82, 0xc5, 0x92, 0x5a, 0x8a, 0xfa, 
        0x8b, 0x4c, 0x89, 0xaf, 0x8f, 0xe1, 0x8a, 0x51, 0x82, 0xc9, 0x8a, 0xd7, 0x82, 0xc1, 0x82, 0xbd, 
        0x8f, 0xad, 0x8f, 0x97, 0x81, 0x63, 0x81, 0x63, 0x8b, 0x4c, 0x89, 0xaf, 0x82, 0xf0, 0x8e, 0x4f, 
        0x93, 0xfa, 0x82, 0xb5, 0x82, 0xa9, 0x88, 0xdb, 0x8e, 0x9d, 0x82, 0xc5, 0x82, 0xab, 0x82, 0xc8, 
        0x82, 0xa2, 0x82, 0xc6, 0x82, 0xa2, 0x82, 0xa4, 0x8f, 0xe3, 0x8d, 0xe2, 0x8a, 0x9d, 0x89, 0x48, 
        0x96, 0xeb, 0x81, 0x42, 0x0a, 0x0a, 0x8f, 0x6f, 0x89, 0xef, 0x82, 0xa2, 0x82, 0xe0, 0x81, 0x41, 
        0x93, 0xf1, 0x90, 0x6c, 0x82, 0xc5, 0x94, 0x59, 0x82, 0xf1, 0x82, 0xbe, 0x82, 0xb1, 0x82, 0xc6, 
        0x82, 0xe0, 0x81, 0x41, 0x83, 0x4c, 0x83, 0x58, 0x82, 0xf0, 0x82, 0xb5, 0x82, 0xbd, 0x82, 0xb1, 
        0x82, 0xc6, 0x82, 0xe0, 0x81, 0x63, 0x81, 0x63, 0x91, 0x53, 0x82, 0xc4, 0x8e, 0xb8, 0x82, 0xed, 
        0x82, 0xea, 0x82, 0xc4, 0x82, 0xb5, 0x82, 0xdc, 0x82, 0xc1, 0x82, 0xbd, 0x8a, 0x9d, 0x89, 0x48, 
        0x96, 0xeb, 0x82, 0xcc, 0x8b, 0x4c, 0x89, 0xaf, 0x81, 0x42, 0x0a, 0x0a, 0x82, 0xbb, 0x82, 0xcc, 
        0x8e, 0x96, 0x8f, 0xee, 0x82, 0xf0, 0x92, 0x6d, 0x82, 0xc1, 0x82, 0xbd, 0x91, 0x73, 0x91, 0xbe, 
        0x82, 0xcd, 0x81, 0x41, 0x94, 0xde, 0x8f, 0x97, 0x82, 0xcc, 0x8b, 0x4c, 0x89, 0xaf, 0x82, 0xc6, 
        0x90, 0x5e, 0x90, 0xb3, 0x96, 0xca, 0x82, 0xa9, 0x82, 0xe7, 0x97, 0xa7, 0x82, 0xbf, 0x8c, 0xfc, 
        0x82, 0xa9, 0x82, 0xa4, 0x82, 0xb1, 0x82, 0xc6, 0x82, 0xf0, 0x8c, 0x88, 0x88, 0xd3, 0x82, 0xb7, 
        0x82, 0xe9, 0x81, 0x42, 0x0a, 0x0a, 0x8b, 0x4c, 0x89, 0xaf, 0x82, 0xf0, 0x8e, 0x4f, 0x93, 0xfa, 
        0x82, 0xb5, 0x82, 0xa9, 0x88, 0xdb, 0x8e, 0x9d, 0x82, 0xc5, 0x82, 0xab, 0x82, 0xc8, 0x82, 0xa2, 
        0x8f, 0xad, 0x8f, 0x97, 0x81, 0x41, 0x8f, 0xe3, 0x8d, 0xe2, 0x8a, 0x9d, 0x89, 0x48, 0x96, 0xeb, 
        0x82, 0xc6, 0x82, 0xcc, 0x8f, 0x6f, 0x89, 0xef, 0x82, 0xa2, 0x82, 0xaa, 0x92, 0xa9, 0x91, 0x71, 
        0x91, 0x73, 0x91, 0xbe, 0x82, 0xf0, 0x95, 0xa8, 0x8c, 0xea, 0x82, 0xcc, 0x92, 0x86, 0x82, 0xd6, 
        0x97, 0x55, 0x82, 0xc1, 0x82, 0xc4, 0x82, 0xa2, 0x82, 0xad, 0x81, 0x42, 0x0a, 0x82, 0xbb, 0x82, 
        0xb5, 0x82, 0xc4, 0x90, 0x47, 0x82, 0xea, 0x82, 0xe9, 0x93, 0x87, 0x82, 0xcc, 0x94, 0xe9, 0x96, 
        0xa7, 0x82, 0xc6, 0x81, 0x41, 0x8f, 0xad, 0x8f, 0x97, 0x82, 0xcc, 0x94, 0xe9, 0x96, 0xa7, 0x81, 
        0x42, 0x0a, 0x91, 0x53, 0x82, 0xc4, 0x82, 0xf0, 0x92, 0x6d, 0x82, 0xc1, 0x82, 0xbd, 0x93, 0xf1, 
        0x90, 0x6c, 0x82, 0xaa, 0x8c, 0xfc, 0x82, 0xab, 0x8d, 0x87, 0x82, 0xa4, 0x90, 0x5e, 0x8e, 0xc0, 
        0x82, 0xc6, 0x82, 0xcd, 0x81, 0x5c, 0x81, 0x5c, 0x81, 0x48, 0x0a, 0x0a, 0x81, 0x7c, 0x97, 0x52, 
        0x94, 0xe4, 0x8e, 0x71, 0x82, 0xcc, 0x8f, 0xcd, 0x81, 0x7c, 0x0a, 0x0a, 0x93, 0x7e, 0x8b, 0x78, 
        0x82, 0xdd, 0x82, 0xf0, 0x97, 0x98, 0x97, 0x70, 0x82, 0xb5, 0x81, 0x41, 0x93, 0x87, 0x82, 0xc5, 
        0x83, 0x79, 0x83, 0x93, 0x83, 0x56, 0x83, 0x87, 0x83, 0x93, 0x82, 0xf0, 0x8c, 0x6f, 0x89, 0x63, 
        0x82, 0xb7, 0x82, 0xe9, 0x8f, 0x66, 0x95, 0xea, 0x82, 0xcc, 0x82, 0xe0, 0x82, 0xc6, 0x82, 0xc9, 
        0x96, 0x4b, 0x82, 0xea, 0x82, 0xbd, 0x81, 0x77, 0x92, 0xa9, 0x91, 0x71, 0x91, 0x73, 0x91, 0xbe, 
        0x81, 0x78, 0x0a, 0x83, 0x79, 0x83, 0x93, 0x83, 0x56, 0x83, 0x87, 0x83, 0x93, 0x82, 0xc5, 0x83, 
        0x41, 0x83, 0x8b, 0x83, 0x6f, 0x83, 0x43, 0x83, 0x67, 0x82, 0xf0, 0x82, 0xb5, 0x82, 0xc8, 0x82, 
        0xaa, 0x82, 0xe7, 0x8f, 0x5d, 0x96, 0x85, 0x82, 0xc5, 0x82, 0xa0, 0x82, 0xe9, 0x81, 0x77, 0x8e, 
        0x4f, 0x8d, 0x44, 0x97, 0x52, 0x94, 0xe4, 0x8e, 0x71, 0x81, 0x78, 0x81, 0x41, 0x81, 0x77, 0x91, 
        0x6f, 0x97, 0x74, 0x81, 0x78, 0x8e, 0x6f, 0x96, 0x85, 0x82, 0xe2, 0x81, 0x41, 0x97, 0x63, 0x93, 
        0xe9, 0x90, 0xf5, 0x82, 0xcc, 0x81, 0x77, 0x98, 0x5a, 0x8a, 0x70, 0x8c, 0xdc, 0x98, 0x59, 0x81, 
        0x78, 0x82, 0xc6, 0x8a, 0x79, 0x82, 0xb5, 0x82, 0xa2, 0x93, 0xfa, 0x81, 0x58, 0x82, 0xf0, 0x89, 
        0xdf, 0x82, 0xb2, 0x82, 0xb5, 0x82, 0xc4, 0x82, 0xa2, 0x82, 0xad, 0x81, 0x42, 0x0a, 0x0a, 0x82, 
        0xa2, 0x82, 0xc2, 0x82, 0xe0, 0x82, 0xc6, 0x95, 0xcf, 0x82, 0xed, 0x82, 0xe7, 0x82, 0xca, 0x93, 
        0x87, 0x82, 0xc5, 0x82, 0xcc, 0x95, 0xe9, 0x82, 0xe7, 0x82, 0xb5, 0x81, 0x42, 0x0a, 0x82, 0xdd, 
        0x82, 0xf1, 0x82, 0xc8, 0x82, 0xc5, 0x82, 0xa8, 0x8d, 0xd5, 0x82, 0xe8, 0x82, 0xe2, 0x89, 0xd4, 
        0x89, 0xce, 0x82, 0xf0, 0x8a, 0x79, 0x82, 0xb5, 0x82, 0xde, 0x81, 0x42, 0x0a, 0x82, 0xbe, 0x82, 
        0xaa, 0x82, 0xa0, 0x82, 0xe9, 0x93, 0xfa, 0x81, 0x41, 0x91, 0x73, 0x91, 0xbe, 0x82, 0xc6, 0x97, 
        0x52, 0x94, 0xe4, 0x8e, 0x71, 0x82, 0xcc, 0x8f, 0x5d, 0x96, 0x85, 0x82, 0xc6, 0x82, 0xa2, 0x82, 
        0xa4, 0x8a, 0xd6, 0x8c, 0x57, 0x82, 0xcd, 0x81, 0x41, 0x8e, 0x76, 0x82, 0xed, 0x82, 0xca, 0x82, 
        0xc6, 0x82, 0xb1, 0x82, 0xeb, 0x82, 0xc5, 0x95, 0xf6, 0x82, 0xea, 0x8b, 0x8e, 0x82, 0xe9, 0x81, 
        0x42, 0x0a, 0x0a, 0x82, 0xd3, 0x82, 0xc6, 0x82, 0xb5, 0x82, 0xbd, 0x82, 0xb1, 0x82, 0xc6, 0x82, 
        0xf0, 0x82, 0xab, 0x82, 0xc1, 0x82, 0xa9, 0x82, 0xaf, 0x82, 0xc9, 0x81, 0x41, 0x97, 0x52, 0x94, 
        0xe4, 0x8e, 0x71, 0x82, 0xcd, 0x91, 0x73, 0x91, 0xbe, 0x82, 0xc9, 0x91, 0x7a, 0x82, 0xa2, 0x82, 
        0xf0, 0x93, 0x60, 0x82, 0xa6, 0x82, 0xc4, 0x82, 0xab, 0x82, 0xbd, 0x82, 0xcc, 0x82, 0xbe, 0x81, 
        0x42, 0x0a, 0x82, 0xbe, 0x82, 0xaa, 0x81, 0x41, 0x8e, 0x71, 0x8b, 0x9f, 0x82, 0xcc, 0x8d, 0xa0, 
        0x82, 0xa9, 0x82, 0xe7, 0x88, 0xea, 0x8f, 0x8f, 0x82, 0xbe, 0x82, 0xc1, 0x82, 0xbd, 0x82, 0xb1, 
        0x82, 0xc6, 0x82, 0xe0, 0x82, 0xa0, 0x82, 0xe8, 0x81, 0x41, 0x97, 0x52, 0x94, 0xe4, 0x8e, 0x71, 
        0x82, 0xcc, 0x8b, 0x43, 0x8e, 0x9d, 0x82, 0xbf, 0x82, 0xc9, 0x8d, 0xa2, 0x98, 0x66, 0x82, 0xb7, 
        0x82, 0xe9, 0x91, 0x73, 0x91, 0xbe, 0x81, 0x42, 0x0a, 0x89, 0xfc, 0x82, 0xdf, 0x82, 0xc4, 0x8e, 
        0xa9, 0x95, 0xaa, 0x82, 0xcc, 0x8b, 0x43, 0x8e, 0x9d, 0x82, 0xbf, 0x82, 0xc9, 0x82, 0xc2, 0x82, 
        0xa2, 0x82, 0xc4, 0x8d, 0x6c, 0x82, 0xa6, 0x82, 0xe9, 0x81, 0x42, 0x0a, 0x82, 0xbb, 0x82, 0xb5, 
        0x82, 0xc4, 0x81, 0x41, 0x92, 0x48, 0x82, 0xe8, 0x92, 0x85, 0x82, 0xa2, 0x82, 0xbd, 0x93, 0x9a, 
        0x82, 0xa6, 0x81, 0x63, 0x81, 0x63, 0x8e, 0xa9, 0x95, 0xaa, 0x82, 0xcd, 0x97, 0x52, 0x94, 0xe4, 
        0x8e, 0x71, 0x82, 0xcc, 0x82, 0xb1, 0x82, 0xc6, 0x82, 0xaa, 0x8d, 0x44, 0x82, 0xab, 0x82, 0xbe, 
        0x82, 0xc6, 0x82, 0xa2, 0x82, 0xa4, 0x8b, 0x43, 0x8e, 0x9d, 0x82, 0xbf, 0x81, 0x42, 0x0a, 0x0a, 
        0x82, 0xbb, 0x82, 0xcc, 0x8b, 0x43, 0x8e, 0x9d, 0x82, 0xbf, 0x82, 0xf0, 0x90, 0xb3, 0x92, 0xbc, 
        0x82, 0xc9, 0x93, 0x60, 0x82, 0xa6, 0x81, 0x41, 0x82, 0xe2, 0x82, 0xc1, 0x82, 0xc6, 0x82, 0xcc, 
        0x82, 0xb1, 0x82, 0xc6, 0x82, 0xc5, 0x81, 0x41, 0x95, 0x74, 0x82, 0xab, 0x8d, 0x87, 0x82, 0xa4, 
        0x82, 0xb1, 0x82, 0xc6, 0x82, 0xc6, 0x82, 0xc8, 0x82, 0xc1, 0x82, 0xbd, 0x93, 0xf1, 0x90, 0x6c, 
        0x81, 0x42, 0x0a, 0x90, 0xb0, 0x82, 0xea, 0x82, 0xc4, 0x81, 0x41, 0x97, 0x63, 0x93, 0xe9, 0x90, 
        0xf5, 0x82, 0xa9, 0x82, 0xe7, 0x97, 0xf6, 0x90, 0x6c, 0x82, 0xd6, 0x82, 0xc6, 0x8a, 0xd6, 0x8c, 
        0x57, 0x82, 0xf0, 0x90, 0x5b, 0x82, 0xdf, 0x82, 0xc4, 0x82, 0xa2, 0x82, 0xad, 0x81, 0x42, 0x0a, 
        0x0a, 0x82, 0xb5, 0x82, 0xa9, 0x82, 0xb5, 0x81, 0x41, 0x8d, 0x4b, 0x82, 0xb9, 0x82, 0xc8, 0x93, 
        0xfa, 0x81, 0x58, 0x82, 0xaa, 0x91, 0xb1, 0x82, 0xa2, 0x82, 0xc4, 0x82, 0xa2, 0x82, 0xad, 0x82, 
        0xc6, 0x8e, 0x76, 0x82, 0xed, 0x82, 0xea, 0x82, 0xbd, 0x8d, 0xc5, 0x92, 0x86, 0x81, 0x41, 0x93, 
        0xf1, 0x90, 0x6c, 0x82, 0xcc, 0x91, 0x4f, 0x82, 0xc9, 0x96, 0x59, 0x82, 0xea, 0x8b, 0x8e, 0x82, 
        0xc1, 0x82, 0xbd, 0x82, 0xcd, 0x82, 0xb8, 0x82, 0xcc, 0x89, 0xdf, 0x8b, 0x8e, 0x82, 0xaa, 0x97, 
        0xa7, 0x82, 0xbf, 0x82, 0xd3, 0x82, 0xb3, 0x82, 0xaa, 0x82, 0xc1, 0x82, 0xbd, 0x81, 0x5c, 0x81, 
        0x5c, 0x8b, 0x7d, 0x91, 0xac, 0x82, 0xc9, 0x95, 0xf6, 0x82, 0xea, 0x8b, 0x8e, 0x82, 0xc1, 0x82, 
        0xc4, 0x82, 0xa2, 0x82, 0xad, 0x93, 0xfa, 0x8f, 0xed, 0x82, 0xc9, 0x81, 0x41, 0x93, 0xf1, 0x90, 
        0x6c, 0x82, 0xcd, 0x82, 0xc7, 0x82, 0xa4, 0x97, 0xa7, 0x82, 0xbf, 0x8c, 0xfc, 0x82, 0xa9, 0x82, 
        0xc1, 0x82, 0xc4, 0x82, 0xa2, 0x82, 0xad, 0x82, 0xcc, 0x82, 0xa9, 0x81, 0x49, 0x81, 0x48, 0x0a, 
        0x82, 0xbb, 0x82, 0xb5, 0x82, 0xc4, 0x97, 0x52, 0x94, 0xe4, 0x8e, 0x71, 0x82, 0xaa, 0x96, 0x59, 
        0x82, 0xea, 0x8b, 0x8e, 0x82, 0xc1, 0x82, 0xbd, 0x89, 0xdf, 0x8b, 0x8e, 0x82, 0xc6, 0x82, 0xcd, 
        0x81, 0x5c, 0x81, 0x5c, 0x81, 0x48, 0x0a, 0x0a, 0x81, 0x7c, 0x8d, 0xb9, 0x81, 0x58, 0x97, 0x85, 
        0x98, 0x88, 0x78, 0x50, 0x7b, 0x22, 0xe6, 0x0d, 0x40, 0xc4, 0x4d, 0x84, 0x75, 0x41, 0x4e, 0xde, 
        0x1c, 0x20, 0x21, 0xd4, 0xd2, 0xd4, 0x82, 0x6b, 0x3e, 0x8f, 0xb2, 0x8b, 0x40, 0xab, 0xd1, 0x6c, 
        0xf3, 0x9d, 0x84, 0x54, 0x05, 0xbd, 0x7e, 0xd0, 0xba, 0x1d, 0x5a, 0x30, 0xd3, 0xf2, 0x26, 0xb1, 
        0xfb, 0xc0, 0xbe, 0x4c, 0xf0, 0xa6, 0x23, 0x29, 0x7b, 0xae, 0xa6, 0x94, 0xd1, 0xef, 0x56, 0xdf, 
        0x49, 0xed, 0xea, 0x16, 0xa1, 0x60, 0xaa, 0x7b, 0x58, 0x0c, 0x2c, 0x18, 0xd2, 0xb9, 0x6f, 0x6b, 
        0xbd, 0x08, 0x4c, 0x31, 0x71, 0xcc, 0x0c, 0xa1, 0xce, 0xaa, 0xd4, 0xd4, 0x78, 0xb5, 0x49, 0xa2, 
        0x1f, 0x97, 0x78, 0x4d, 0x0c, 0xa5, 0xd3, 0xc2, 0x89, 0x85, 0x5f, 0x22, 0x67, 0xc5, 0x14, 0xfd, 
        0xf6, 0x43, 0xaa, 0x55, 0xa7, 0x47, 0xe8, 0x85, 0x79, 0x63, 0x9d, 0x1e, 0x6e, 0x56, 0x66, 0xa4, 
        0xce, 0x94, 0x5d, 0x8d, 0x68, 0x66, 0x65, 0xb0, 0x10, 0x55, 0xa4, 0x4a, 0x8e, 0x54, 0xd1, 0x1e, 
        0x43, 0x04, 0x09, 0x33, 0x34, 0x5a, 0xe5, 0x89, 0x10, 0x80, 0x62, 0x19, 0x01, 0x3a, 0x6c, 0x44, 
        0x84, 0x5c, 0x20, 0x26, 0xec, 0x63, 0xa0, 0xe3, 0xf1, 0xc8, 0x82, 0xf1, 0x54, 0x3f, 0x69, 0x28, 
        0xd0, 0x59, 0xaf, 0x99, 0x50, 0x32, 0x05, 0x33, 0x6a, 0x27, 0x5a, 0xfe, 0x00, 0xe3, 0x20, 0x76, 
        0xb0, 0xf9, 0x45, 0x00, 0x4f, 0x31, 0x5b, 0x4d, 0x07, 0x40, 0x7f, 0x7d, 0x5d, 0x9b, 0xf1, 0x9d, 
        0xe4, 0x7d, 0xa8, 0xd7, 0xc6, 0x4c, 0x2f, 0x31, 0x64, 0x24, 0x20, 0xdd, 0x4f, 0x92, 0x17, 0x96, 
        0x98, 0x21, 0x2b, 0xe8, 0x60, 0x26, 0xac, 0xf3, 0x00, 0xc6, 0x7b, 0xd4, 0xcd, 0xd5, 0xe3, 0x3f, 
        0x43, 0x89, 0x10, 0xd5, 0x66, 0x68, 0x1e, 0x72, 0x6c, 0x28, 0xa6, 0xd8, 0xe7, 0x8e, 0xcf, 0xff, 
        0x55, 0x87, 0x42, 0xad, 0xff, 0xb1, 0x90, 0xb0, 0x9c, 0x63, 0xe7, 0x74, 0xe0, 0xc0, 0x0c, 0x94, 
        0xc8, 0x5f, 0x5b, 0x4a, 0x12, 0x5f, 0x91, 0xab, 0xd7, 0xa8, 0xdc, 0x16, 0x94, 0xc1, 0x17, 0x62, 
        0x43, 0xa4, 0x3e, 0xa2, 0x93, 0xcd, 0xea, 0xf2, 0xb4, 0xe4, 0xf6, 0x2e, 0xe6, 0x70, 0x40, 0xf7, 
        0xe3, 0xdf, 0x7d, 0x85, 0x44, 0x49, 0x9a, 0xe4, 0xbe, 0x1b, 0x0b, 0xc1, 0x79, 0x7b, 0x42, 0x95, 
        0x13, 0x87, 0x17, 0x2f, 0x14, 0x92, 0x8d, 0xfb, 0x1f, 0xac, 0x24, 0x65, 0xfd, 0x7d, 0xd2, 0x58, 
        0x56, 0x44, 0x42, 0xd7, 0x75, 0x12, 0xe0, 0xb7, 0xfa, 0xcf, 0x8d, 0x98, 0xf7, 0x4f, 0x2a, 0x31, 
        0x1b, 0x4f, 0x57, 0x0f, 0x91, 0x56, 0xd1, 0x52, 0x4e, 0xb2, 0xfb, 0x04, 0xa0, 0xd5, 0x6b, 0xfa, 
        0x39, 0x15, 0x08, 0x96, 0xb4, 0x8f, 0xce, 0x96, 0x4d, 0xe2, 0x03, 0xac, 0xc8, 0xd2, 0x76, 0x13, 
        0x83, 0xb2, 0x32, 0x93, 0x9a, 0x44, 0x74, 0x3f, 0x40, 0x00, 0xae, 0x75, 0x06, 0x37, 0x4d, 0x64, 
        0x66, 0xb5, 0x40, 0xab, 0x28, 0x54, 0x62, 0x85, 0x3c, 0x4a, 0x1d, 0xa6, 0x77, 0x70, 0x3a, 0x6b, 
        0x38, 0xfb, 0x69, 0x5c, 0x54, 0xee, 0x86, 0xe2, 0xfd, 0xb4, 0x03, 0x89, 0x3f, 0xb9, 0x75, 0xe0, 
        0x0c, 0x55, 0x31, 0x54, 0x44, 0x7b, 0xef, 0x5b, 0x0c, 0x7f, 0xe5, 0xfe, 0x57, 0x1a, 0x58, 0x7e, 
        0x2c, 0x85, 0x06, 0xa4, 0x49, 0xa6, 0x1c, 0x6b, 0x1c, 0x00, 0x01, 0xf5, 0xae, 0xc4, 0xfb, 0x8e, 
        0xfe, 0x7d, 0xa5, 0xe2, 0x57, 0xf2, 0x4b, 0x36, 0xec, 0x8c, 0x65, 0x44, 0x8a, 0xfe, 0x38, 0x67, 
        0xe2, 0xf3, 0x55, 0xa6, 0x18, 0xc4, 0x9d, 0x61, 0x9e, 0xdc, 0xc6, 0xf2, 0xc8, 0x45, 0xc8, 0xa0, 
        0xe3, 0xb9, 0xba, 0x7a, 0xed, 0x7a, 0x21, 0x73, 0x99, 0xa3, 0xfd, 0xda, 0x5a, 0xb5, 0x46, 0x02, 
        0x44, 0xe8, 0x35, 0x21, 0x61, 0xf4, 0x08, 0x5f, 0x44, 0x40, 0xc5, 0x3c, 0x18, 0x6b, 0x99, 0x1f, 
        0x57, 0xd8, 0xa3, 0xd7, 0xed, 0x2a, 0x81, 0x18, 0x59, 0x78, 0x1f, 0xb8, 0x93, 0xfb, 0xc0, 0xef, 
        0x00, 0x61, 0xab, 0xe8, 0x09, 0x36, 0x9d, 0x52, 0x13, 0x7c, 0xf0, 0x24, 0x2d, 0x4c, 0xc2, 0xa4, 
        0xb6, 0x3c, 0x55, 0x06, 0x0c, 0xeb, 0xcb, 0x7e, 0x33, 0x82, 0xd1, 0x57, 0x84, 0x8e, 0x7d, 0x25, 
        0x85, 0xa7, 0x00, 0xa3, 0x7a, 0xde, 0xb0, 0xbb, 0xf6, 0x30, 0x12, 0xcd, 0x59, 0xd1, 0x56, 0x1f, 
        0xc5, 0xde, 0xda, 0xa8, 0x15, 0x7c, 0x68, 0x7d, 0x82, 0x4f, 0x72, 0xb6, 0x23, 0xf0, 0x10, 0xaa, 
        0x28, 0x5f, 0xfd, 0x0e, 0xb1, 0xc4, 0xea, 0xf3, 0x9b, 0x55, 0x88, 0xf2, 0x41, 0x26, 0x81, 0x93, 
        0xdb, 0xc8, 0xe1, 0x64, 0xea, 0x6c, 0x43, 0x8b, 0xc8, 0xfe, 0xde, 0xe7, 0xe9, 0xb9, 0x28, 0xad, 
        0x84, 0x7b, 0x57, 0xb8, 0x90, 0x02, 0x47, 0x12, 0xf2, 0x33, 0xf4, 0xce, 0x32, 0x8d, 0x3f, 0x56, 
        0xa2, 0x1a, 0xdd, 0x61, 0x6d, 0xd5, 0x49, 0x17, 0x57, 0xc3, 0xc9, 0x51, 0x4f, 0x12, 0x93, 0x31, 
        0x73, 0x47, 0x3e, 0xc4, 0x8c, 0x99, 0xd9, 0xf5, 0xd4, 0xea, 0x08, 0xa1, 0x79, 0x59, 0xe1, 0x86, 
        0x1a, 0x06, 0xf3, 0x5f, 0x45, 0x8c, 0xbc, 0xeb, 0xc9, 0x19, 0x7b, 0x6f, 0xfa, 0xbd, 0x37, 0x68, 
        0x38, 0xde, 0xac, 0xab, 0x1f, 0xde, 0xf4, 0xd1, 0x59, 0x1c, 0x21, 0x41, 0x67, 0xf5, 0xcf, 0x7e, 
        0x06, 0x87, 0xeb, 0x6e, 0x6d, 0xd2, 0xc4, 0xf3, 0x38, 0xf9, 0x7e, 0x3f, 0x0c, 0xba, 0x39, 0x5b, 
        0xa7, 0x34, 0xbd, 0xa9, 0xd5, 0xe6, 0x80, 0xe9, 0xde, 0xfe, 0xaa, 0x55, 0xf5, 0x0c, 0x30, 0xf5, 
        0x2c, 0x93, 0x38, 0xb5, 0x86, 0xb9, 0xdc, 0xdf, 0xd4, 0x04, 0x71, 0x92, 0xac, 0xdf, 0x56, 0x39, 
        0x9a, 0xbc, 0xa0, 0xba, 0xd1, 0xe0, 0x3b, 0xda, 0x26, 0x6c, 0xd5, 0x1b, 0xec, 0xd0, 0xc4, 0xae, 
        0x65, 0x4c, 0x48, 0xab, 0xb2, 0xc5, 0x82, 0xa3, 0x32, 0x70, 0xc9, 0x62, 0xa9, 0xe9, 0x9d, 0x72, 
        0xac, 0x30, 0xcb, 0x05, 0xb7, 0x88, 0xe5, 0x80, 0x08, 0xc3, 0x16, 0xa2, 0xdc, 0x94, 0x9c, 0xc7, 
        0x74, 0xe7, 0xe3, 0x60, 0x99, 0x50, 0x08, 0x07, 0xaa, 0x73, 0xfb, 0xe7, 0xa2, 0xd7, 0x23, 0xbb, 
        0xd0, 0x5e, 0xf6, 0x81, 0x41, 0xb6, 0x12, 0x7d, 0xdc, 0x26, 0xa8, 0x90, 0xd5, 0x93, 0x4e, 0x6e, 
        0xda, 0x98, 0x63, 0x65, 0x06, 0x99, 0xd7, 0x66, 0xd6, 0x1c, 0x54, 0xa0, 0xfd, 0x2a, 0xff, 0x6c, 
        0xca, 0xa6, 0x6b, 0xa7, 0x3e, 0x7f, 0xec, 0x97, 0xc2, 0xcb, 0x57, 0x5d, 0xfe, 0xc8, 0xc1, 0xa6, 
        0xdd, 0xec, 0xe6, 0xcf, 0x99, 0xa1, 0x16, 0x72, 0x38, 0xc8, 0x31, 0xe0, 0x21, 0x3a, 0x1b, 0xb5, 
        0x71, 0x7e, 0x1c, 0x8a, 0x7d, 0x7f, 0xde, 0x39, 0x29, 0x7c, 0x93, 0x15, 0x62, 0xb5, 0x77, 0x37, 
        0xa9, 0xc2, 0xcf, 0xc8, 0x36, 0xbc, 0x07, 0xee, 0x93, 0xae, 0x86, 0x0d, 0x0a, 0xb1, 0xc8, 0xc4, 
        0x84, 0x6f, 0x0d, 0xbd, 0xe4, 0x75, 0xea, 0x91, 0x29, 0x49, 0xe4, 0x4a, 0xa0, 0xdf, 0xae, 0x2b, 
        0x2e, 0x4e, 0x92, 0xcc, 0xc3, 0x28, 0xbc, 0x72, 0x6f, 0xd4, 0xeb, 0x61, 0x98, 0x8e, 0xaa, 0x60, 
        0x85, 0x48, 0x06, 0x55, 0x3c, 0x88, 0x34, 0x10, 0xac, 0x42, 0x91, 0x8e, 0xd9, 0x8d, 0xbd, 0xb0, 
        0x1b, 0xc6, 0x4b, 0xbe, 0xbb, 0xdf, 0x4a, 0xda, 0xb8, 0x19, 0xbc, 0x2d, 0xc2, 0x55, 0xe9, 0x51, 
        0xe8, 0xd7, 0x94, 0xce, 0xe5, 0x9d, 0x75, 0xfd, 0x5f, 0x1e, 0xa8, 0x8d, 0xf6, 0xed, 0x9d, 0xcc, 
        0x0b, 0x26, 0xd9, 0x94, 0xb0, 0xe5, 0x9d, 0xc6, 0x6d, 0xdb, 0x05, 0x41, 0x7e, 0x4b, 0xb0, 0xb3, 
};

static int xcode_building_first_stage(struct cxdec_xcode_status *xcode)
{
      switch (xcode_rand(xcode) % 3) {
      case 0:
        // MOV ESI, EncryptionControlBlock
        // MOV EAX, DWORD PTR DS:[ESI+((xcode_rand(xcode) & 0x3ff) << 2)]
        if (!push_bytexcode(xcode, 0xbe)
                || !push_dwordxcode(xcode, (DWORD)EncryptionControlBlock)
                || !push_2bytesxcode(xcode, 0x8b, 0x86)
                || !push_dwordxcode(xcode, (xcode_rand(xcode) & 0x3ff) << 2))
            return 0;
        break;
    case 2:
          // MOV EAX, EDI
          if (!push_2bytesxcode(xcode, 0x8b, 0xc7))
              return 0;
        break;
      case 1:
        // MOV EAX, xcode_rand(xcode)
        if (!push_bytexcode(xcode, 0xb8)
                || !push_dwordxcode(xcode, xcode_rand(xcode)))
            return 0;
          break;
      }
      return 1;
}

static int xcode_building_stage0(struct cxdec_xcode_status *xcode, int stage)
{
    if (stage == 1)
        return xcode_building_first_stage(xcode);

    if (xcode_rand(xcode) & 1) {
        if (!xcode_building_stage1(xcode, stage - 1))
            return 0;
    } else {
        if (!xcode_building_stage0(xcode, stage - 1))
            return 0;
    }

    switch (xcode_rand(xcode) & 7) {
    case 5:
        // NOT EAX
        if (!push_2bytesxcode(xcode, 0xf7, 0xd0))
            return 0;
        break;
    case 7:
        // MOV ESI, EncryptionControlBlock
        // AND EAX, 3FFh
        // MOV EAX, DWORD PTR DS:[ESI+EAX*4]
        if (!push_bytexcode(xcode, 0xbe)
                  || !push_dwordxcode(xcode, (DWORD)EncryptionControlBlock)
                || !push_bytexcode(xcode, 0x25)
                || !push_dwordxcode(xcode, 0x3ff)
                || !push_3bytesxcode(xcode, 0x8b, 0x04, 0x86))
            return 0;
        break;
    case 4:
        // DEC EAX
        if (!push_bytexcode(xcode, 0x48))
            return 0;
        break;
    case 1:
        // NEG EAX
        if (!push_2bytesxcode(xcode, 0xf7, 0xd8))
            return 0;
        break;
    case 0:
        if (xcode_rand(xcode) & 1) {
            // ADD EAX, xcode_rand(xcode)
            if (!push_bytexcode(xcode, 0x05))
                return 0;
        } else {
            // SUB EAX, xcode_rand(xcode)
            if (!push_bytexcode(xcode, 0x2d))
                return 0;
        }
        if (!push_dwordxcode(xcode, xcode_rand(xcode)))
            return 0;
        break;
    case 6:
        // PUSH EBX
        // MOV EBX, EAX
        // AND EBX, AAAAAAAA
        // AND EAX, 55555555
        // SHR EBX, 1
        // SHL EAX, 1
        // OR EAX, EBX
        // POP EBX
        if (!push_bytexcode(xcode, 0x53)
                || !push_2bytesxcode(xcode, 0x89, 0xc3)
                || !push_6bytesxcode(xcode, 0x81, 0xe3, 0xaa, 0xaa, 0xaa, 0xaa)
                || !push_5bytesxcode(xcode, 0x25, 0x55, 0x55, 0x55, 0x55)
                || !push_2bytesxcode(xcode, 0xd1, 0xeb)
                || !push_2bytesxcode(xcode, 0xd1, 0xe0)
                || !push_2bytesxcode(xcode, 0x09, 0xd8)
                || !push_bytexcode(xcode, 0x5b))
            return 0;
        break;
    case 2:
        // INC EAX
        if (!push_bytexcode(xcode, 0x40))
            return 0;
        break;
    case 3:
        // XOR EAX, xcode_rand(xcode)
        if (!push_bytexcode(xcode, 0x35) 
                || !push_dwordxcode(xcode, xcode_rand(xcode)))
            return 0;
        break;
    }
    return 1;
}

static int xcode_building_stage1(struct cxdec_xcode_status *xcode, int stage)
{
    if (stage == 1)
        return xcode_building_first_stage(xcode);

    // PUSH EBX
    if (!push_bytexcode(xcode, 0x53))
        return 0;

    if (xcode_rand(xcode) & 1) {
        if (!xcode_building_stage1(xcode, stage - 1))
            return 0;
    } else {
        if (!xcode_building_stage0(xcode, stage - 1))
            return 0;
    }

    // MOV EBX, EAX
    if (!push_2bytesxcode(xcode, 0x89, 0xc3))
        return 0;

    if (xcode_rand(xcode) & 1) {
        if (!xcode_building_stage1(xcode, stage - 1))
            return 0;
    } else {
        if (!xcode_building_stage0(xcode, stage - 1))
            return 0;
    }

    switch (xcode_rand(xcode) % 6) {
    case 0:
        // ADD EAX, EBX
        if (!push_2bytesxcode(xcode, 0x01, 0xd8))
            return 0;
        break;
    case 3:
        // PUSH ECX
        // MOV ECX, EBX
        // AND ECX, 0F
        // SHR EAX, CL
        // POP ECX
        if (!push_bytexcode(xcode, 0x51)
                || !push_2bytesxcode(xcode, 0x89, 0xd9)
                || !push_3bytesxcode(xcode, 0x83, 0xe1, 0x0f)
                || !push_2bytesxcode(xcode, 0xd3, 0xe8)
                || !push_bytexcode(xcode, 0x59))
            return 0;
        break;
    case 4:
        // PUSH ECX
        // MOV ECX, EBX
        // AND ECX, 0F
        // SHL EAX, CL
        // POP ECX
        if (!push_bytexcode(xcode, 0x51) 
                || !push_2bytesxcode(xcode, 0x89, 0xd9)
                || !push_3bytesxcode(xcode, 0x83, 0xe1, 0x0f)
                || !push_2bytesxcode(xcode, 0xd3, 0xe0)
                || !push_bytexcode(xcode, 0x59))
            return 0;
        break;
    case 1:
        // NEG EAX, ADD EAX, EBX
        if (!push_2bytesxcode(xcode, 0xf7, 0xd8) 
                || !push_2bytesxcode(xcode, 0x01, 0xd8))
            return 0;
        break;
    case 2:
        // IMUL EAX, EBX
        if (!push_3bytesxcode(xcode, 0x0f,  0xaf, 0xc3))
            return 0;
        break;
    case 5:
        // SUB EAX, EBX
        if (!push_2bytesxcode(xcode, 0x29, 0xd8))
            return 0;
        break;
    }
    // POP EBX
    return push_bytexcode(xcode, 0x5b);
}

struct cxdec_callback natukana_cxdec_callback = {
    "natukana",
    { 0x2f5, 0x6f0 },
    xcode_building_stage1
};
