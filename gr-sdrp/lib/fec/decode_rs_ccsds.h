#ifndef DECODE_RS_CCSDS_H
#define DECODE_RS_CCSDS_H

#ifdef __cplusplus
extern "C" {
#endif

int decode_rs_ccsds(unsigned char *data,int *eras_pos,int no_eras,int pad);

#ifdef __cplusplus
}
#endif

#endif
