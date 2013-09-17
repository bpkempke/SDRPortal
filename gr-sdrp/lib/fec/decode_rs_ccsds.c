/* This function wraps around the fixed 8-bit decoder, performing the
 * basis transformations necessary to meet the CCSDS standard
 *
 * Copyright 2002, Phil Karn, KA9Q
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */

#include "fixed.h"
#include <string.h>

char CCSDS_alpha_to[] = {
0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x87,0x89,0x95,0xad,0xdd,0x3d,0x7a,0xf4,
0x6f,0xde,0x3b,0x76,0xec,0x5f,0xbe,0xfb,0x71,0xe2,0x43,0x86,0x8b,0x91,0xa5,0xcd,
0x1d,0x3a,0x74,0xe8,0x57,0xae,0xdb,0x31,0x62,0xc4,0x0f,0x1e,0x3c,0x78,0xf0,0x67,
0xce,0x1b,0x36,0x6c,0xd8,0x37,0x6e,0xdc,0x3f,0x7e,0xfc,0x7f,0xfe,0x7b,0xf6,0x6b,
0xd6,0x2b,0x56,0xac,0xdf,0x39,0x72,0xe4,0x4f,0x9e,0xbb,0xf1,0x65,0xca,0x13,0x26,
0x4c,0x98,0xb7,0xe9,0x55,0xaa,0xd3,0x21,0x42,0x84,0x8f,0x99,0xb5,0xed,0x5d,0xba,
0xf3,0x61,0xc2,0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x07,0x0e,0x1c,0x38,0x70,0xe0,
0x47,0x8e,0x9b,0xb1,0xe5,0x4d,0x9a,0xb3,0xe1,0x45,0x8a,0x93,0xa1,0xc5,0x0d,0x1a,
0x34,0x68,0xd0,0x27,0x4e,0x9c,0xbf,0xf9,0x75,0xea,0x53,0xa6,0xcb,0x11,0x22,0x44,
0x88,0x97,0xa9,0xd5,0x2d,0x5a,0xb4,0xef,0x59,0xb2,0xe3,0x41,0x82,0x83,0x81,0x85,
0x8d,0x9d,0xbd,0xfd,0x7d,0xfa,0x73,0xe6,0x4b,0x96,0xab,0xd1,0x25,0x4a,0x94,0xaf,
0xd9,0x35,0x6a,0xd4,0x2f,0x5e,0xbc,0xff,0x79,0xf2,0x63,0xc6,0x0b,0x16,0x2c,0x58,
0xb0,0xe7,0x49,0x92,0xa3,0xc1,0x05,0x0a,0x14,0x28,0x50,0xa0,0xc7,0x09,0x12,0x24,
0x48,0x90,0xa7,0xc9,0x15,0x2a,0x54,0xa8,0xd7,0x29,0x52,0xa4,0xcf,0x19,0x32,0x64,
0xc8,0x17,0x2e,0x5c,0xb8,0xf7,0x69,0xd2,0x23,0x46,0x8c,0x9f,0xb9,0xf5,0x6d,0xda,
0x33,0x66,0xcc,0x1f,0x3e,0x7c,0xf8,0x77,0xee,0x5b,0xb6,0xeb,0x51,0xa2,0xc3,0x00,
};

char CCSDS_index_of[] = {
255,  0,  1, 99,  2,198,100,106,  3,205,199,188,101,126,107, 42,
  4,141,206, 78,200,212,189,225,102,221,127, 49,108, 32, 43,243,
  5, 87,142,232,207,172, 79,131,201,217,213, 65,190,148,226,180,
103, 39,222,240,128,177, 50, 53,109, 69, 33, 18, 44, 13,244, 56,
  6,155, 88, 26,143,121,233,112,208,194,173,168, 80,117,132, 72,
202,252,218,138,214, 84, 66, 36,191,152,149,249,227, 94,181, 21,
104, 97, 40,186,223, 76,241, 47,129,230,178, 63, 51,238, 54, 16,
110, 24, 70,166, 34,136, 19,247, 45,184, 14, 61,245,164, 57, 59,
  7,158,156,157, 89,159, 27,  8,144,  9,122, 28,234,160,113, 90,
209, 29,195,123,174, 10,169,145, 81, 91,118,114,133,161, 73,235,
203,124,253,196,219, 30,139,210,215,146, 85,170, 67, 11, 37,175,
192,115,153,119,150, 92,250, 82,228,236, 95, 74,182,162, 22,134,
105,197, 98,254, 41,125,187,204,224,211, 77,140,242, 31, 48,220,
130,171,231, 86,179,147, 64,216, 52,176,239, 38, 55, 12, 17, 68,
111,120, 25,154, 71,116,167,193, 35, 83,137,251, 20, 93,248,151,
 46, 75,185, 96, 15,237, 62,229,246,135,165, 23, 58,163, 60,183,
};

char CCSDS_poly[] = {
  0,249, 59, 66,  4, 43,126,251, 97, 30,  3,213, 50, 66,170,  5,
 24,  5,170, 66, 50,213,  3, 30, 97,251,126, 43,  4, 66, 59,249,
  0,
};

unsigned char Taltab[] = {

0x00,0x7b,0xaf,0xd4,0x99,0xe2,0x36,0x4d,0xfa,0x81,0x55,0x2e,0x63,0x18,0xcc,0xb7,
0x86,0xfd,0x29,0x52,0x1f,0x64,0xb0,0xcb,0x7c,0x07,0xd3,0xa8,0xe5,0x9e,0x4a,0x31,
0xec,0x97,0x43,0x38,0x75,0x0e,0xda,0xa1,0x16,0x6d,0xb9,0xc2,0x8f,0xf4,0x20,0x5b,
0x6a,0x11,0xc5,0xbe,0xf3,0x88,0x5c,0x27,0x90,0xeb,0x3f,0x44,0x09,0x72,0xa6,0xdd,
0xef,0x94,0x40,0x3b,0x76,0x0d,0xd9,0xa2,0x15,0x6e,0xba,0xc1,0x8c,0xf7,0x23,0x58,
0x69,0x12,0xc6,0xbd,0xf0,0x8b,0x5f,0x24,0x93,0xe8,0x3c,0x47,0x0a,0x71,0xa5,0xde,
0x03,0x78,0xac,0xd7,0x9a,0xe1,0x35,0x4e,0xf9,0x82,0x56,0x2d,0x60,0x1b,0xcf,0xb4,
0x85,0xfe,0x2a,0x51,0x1c,0x67,0xb3,0xc8,0x7f,0x04,0xd0,0xab,0xe6,0x9d,0x49,0x32,
0x8d,0xf6,0x22,0x59,0x14,0x6f,0xbb,0xc0,0x77,0x0c,0xd8,0xa3,0xee,0x95,0x41,0x3a,
0x0b,0x70,0xa4,0xdf,0x92,0xe9,0x3d,0x46,0xf1,0x8a,0x5e,0x25,0x68,0x13,0xc7,0xbc,
0x61,0x1a,0xce,0xb5,0xf8,0x83,0x57,0x2c,0x9b,0xe0,0x34,0x4f,0x02,0x79,0xad,0xd6,
0xe7,0x9c,0x48,0x33,0x7e,0x05,0xd1,0xaa,0x1d,0x66,0xb2,0xc9,0x84,0xff,0x2b,0x50,
0x62,0x19,0xcd,0xb6,0xfb,0x80,0x54,0x2f,0x98,0xe3,0x37,0x4c,0x01,0x7a,0xae,0xd5,
0xe4,0x9f,0x4b,0x30,0x7d,0x06,0xd2,0xa9,0x1e,0x65,0xb1,0xca,0x87,0xfc,0x28,0x53,
0x8e,0xf5,0x21,0x5a,0x17,0x6c,0xb8,0xc3,0x74,0x0f,0xdb,0xa0,0xed,0x96,0x42,0x39,
0x08,0x73,0xa7,0xdc,0x91,0xea,0x3e,0x45,0xf2,0x89,0x5d,0x26,0x6b,0x10,0xc4,0xbf,
};

unsigned char Tal1tab[] = {
0x00,0xcc,0xac,0x60,0x79,0xb5,0xd5,0x19,0xf0,0x3c,0x5c,0x90,0x89,0x45,0x25,0xe9,
0xfd,0x31,0x51,0x9d,0x84,0x48,0x28,0xe4,0x0d,0xc1,0xa1,0x6d,0x74,0xb8,0xd8,0x14,
0x2e,0xe2,0x82,0x4e,0x57,0x9b,0xfb,0x37,0xde,0x12,0x72,0xbe,0xa7,0x6b,0x0b,0xc7,
0xd3,0x1f,0x7f,0xb3,0xaa,0x66,0x06,0xca,0x23,0xef,0x8f,0x43,0x5a,0x96,0xf6,0x3a,
0x42,0x8e,0xee,0x22,0x3b,0xf7,0x97,0x5b,0xb2,0x7e,0x1e,0xd2,0xcb,0x07,0x67,0xab,
0xbf,0x73,0x13,0xdf,0xc6,0x0a,0x6a,0xa6,0x4f,0x83,0xe3,0x2f,0x36,0xfa,0x9a,0x56,
0x6c,0xa0,0xc0,0x0c,0x15,0xd9,0xb9,0x75,0x9c,0x50,0x30,0xfc,0xe5,0x29,0x49,0x85,
0x91,0x5d,0x3d,0xf1,0xe8,0x24,0x44,0x88,0x61,0xad,0xcd,0x01,0x18,0xd4,0xb4,0x78,
0xc5,0x09,0x69,0xa5,0xbc,0x70,0x10,0xdc,0x35,0xf9,0x99,0x55,0x4c,0x80,0xe0,0x2c,
0x38,0xf4,0x94,0x58,0x41,0x8d,0xed,0x21,0xc8,0x04,0x64,0xa8,0xb1,0x7d,0x1d,0xd1,
0xeb,0x27,0x47,0x8b,0x92,0x5e,0x3e,0xf2,0x1b,0xd7,0xb7,0x7b,0x62,0xae,0xce,0x02,
0x16,0xda,0xba,0x76,0x6f,0xa3,0xc3,0x0f,0xe6,0x2a,0x4a,0x86,0x9f,0x53,0x33,0xff,
0x87,0x4b,0x2b,0xe7,0xfe,0x32,0x52,0x9e,0x77,0xbb,0xdb,0x17,0x0e,0xc2,0xa2,0x6e,
0x7a,0xb6,0xd6,0x1a,0x03,0xcf,0xaf,0x63,0x8a,0x46,0x26,0xea,0xf3,0x3f,0x5f,0x93,
0xa9,0x65,0x05,0xc9,0xd0,0x1c,0x7c,0xb0,0x59,0x95,0xf5,0x39,0x20,0xec,0x8c,0x40,
0x54,0x98,0xf8,0x34,0x2d,0xe1,0x81,0x4d,0xa4,0x68,0x08,0xc4,0xdd,0x11,0x71,0xbd,
};

int decode_rs_8(data_t *data, int *eras_pos, int no_eras, int pad){
  int retval;
 
  if(pad < 0 || pad > 222){
    return -1;
  }

#include "decode_rs.h"
  
  return retval;
}

int decode_rs_ccsds(unsigned char *data,int *eras_pos,int no_eras,int pad){
  int i,r;
  data_t cdata[NN];

  /* Convert data from dual basis to conventional */
  for(i=0;i<NN-pad;i++)
    cdata[i] = Tal1tab[data[i]];

  r = decode_rs_8(cdata,eras_pos,no_eras,pad);

  if(r > 0){
    /* Convert from conventional to dual basis */
    for(i=0;i<NN-pad;i++)
      data[i] = Taltab[cdata[i]];
  }
  return r;
}

