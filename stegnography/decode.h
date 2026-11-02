#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include "types.h"

typedef struct _DecodeInfo
{
    //stego image info 
    char *stego_image_fname;
    FILE *fptr_stego_image;

    //secret file info
    char *secret_fname;
    FILE *fptr_secret;
    int extn_size;
    char extn_secret_file[5];
    char secret_data[100];
    long size_secret_file;

}DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Store Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/*decode extension size*/
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Encode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* decode a byte from LSB of image data array */
Status decode_byte_from_lsb(DecodeInfo *decInfo);

// decode a size from lsb
Status decode_size_from_lsb(DecodeInfo *decInfo);

#endif
