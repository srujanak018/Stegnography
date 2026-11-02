#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo){
    //check input is .bmp or not
    if(strstr(argv[2],".bmp")!=NULL){
        decInfo->stego_image_fname=argv[2];
    }else{
        printf("Input image must be BMP file\n");
        return e_failure;
    }

    //check output file name is given
    if(argv[3]!=NULL){
        decInfo->secret_fname=argv[3];
    }else{
        decInfo->secret_fname="decoded";
    }

    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo){
    // Open stego image file in read mode
    decInfo->fptr_stego_image=fopen(decInfo->stego_image_fname,"rb");

    if(decInfo->fptr_stego_image==NULL){
        printf("Unable to open the stegno image\n");
        return e_failure;
    }

    // Open secret file in write mode
    decInfo->fptr_secret=fopen(decInfo->secret_fname,"wb");

    if(decInfo->fptr_secret==NULL){
        printf("Unable to create secret file\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success; 
}


Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8];
    char decode_char;

    for (int i = 0; i < strlen(magic_string); i++){
        // Read 8 bytes from the stego image
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1){
            printf("Unable to read magic string\n");
            return e_failure;
        }

        // Decode one character from LSBs
        decode_char = 0;

        for (int j = 0; j < 8; j++){
            int bit = buffer[j] & 1;
            decode_char = decode_char | (bit << j);
        }


        if (decode_char != magic_string[i]){
            printf("Magic string does not match\n");
            return e_failure;
        }
    }

    printf("Magic string matched\n");
    return e_success;
}


Status decode_secret_file_extn_size(DecodeInfo *decInfo){
    char buffer[32];
    int size=0;

    //read 32 bytes from stego image 
    fread(buffer,32,1,decInfo->fptr_stego_image);

    //extract 32 bits from lsbs
    for(int i=0;i<32;i++){
        int bits=buffer[i] & 1;
        size=size | (bits<<i);
    }

    //validate extention size
    if(size<=0 || size>=sizeof(decInfo->extn_secret_file)){
        printf("Invalid secret file extention size\n");
        return e_failure;
    }
    
    //storing the extention size
    decInfo->extn_size=size;
    printf("Secret file extention size extracted\n");
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo){
    char buffer[8];
    char decoded_char;

    for(int i=0;i<decInfo->extn_size;i++){
        //read 8 byte from stego image
        fread(buffer,8,1,decInfo->fptr_stego_image);

        //decode one character from lsbs
        decoded_char=0;
        for(int j=0;j<8;j++){
            int bit=buffer[j] & 1;
            decoded_char=decoded_char | (bit<<j);
        }

        //store decoded character
        decInfo->extn_secret_file[i]=decoded_char;
    }

    //add null character at the end
    decInfo->extn_secret_file[decInfo->extn_size]='\0';
    printf("Secret file extention extracted\n");
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo){
    char buffer[32];
    long size=0;

    //read 32 byte from stego image
    if(fread(buffer,1,32,decInfo->fptr_stego_image)!=32){
        return e_failure;
    }

    //extract size from the lsbs
    for(int i=0;i<32;i++){
        int bit=buffer[i] & 1;
        size=size | ((long)bit << i);
    }

    decInfo->size_secret_file=size;
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo){
    char buffer[8];
    char decoded_char;
    //repeat for size_secret_file times
    for(int i=0;i<decInfo->size_secret_file;i++){
        //read 8 byte from stego image
        //printf("Decoding character %d\n", i);
        if(fread(buffer,1,8,decInfo->fptr_stego_image)!=8){
            return e_failure;
        }

        //decode one character from lsbs
        decoded_char=0;

        for(int j=0;j<8;j++){
            int bit=buffer[j] & 1;
            decoded_char=decoded_char | (bit<<j);
        }

        //store decoded character
        decInfo->secret_data[i]=decoded_char;
    }
    decInfo->secret_data[decInfo->size_secret_file] = '\0';

    //printing the decoded data
    printf("Decoded data: %s\n",decInfo->secret_data);

    //write decoded data into secret file 
    fwrite(decInfo->secret_data,decInfo->size_secret_file,1,decInfo->fptr_secret);

    return e_success;
}

Status decode_byte_from_lsb(DecodeInfo *decInfo){
    char buffer[8];
    char decoded_char=0;

    //read 8 bytes from stego image
    if(fread(buffer,8,1,decInfo->fptr_stego_image)!=1){
        return e_failure;
    }

    //extract lsb from each byte
    for(int i=0;i<8;i++){
        int bit=buffer[i] & 1;
        decoded_char=decoded_char | (bit<<i);
    }

    //store decoded character
    decInfo->secret_data[0]=decoded_char;
    return e_success;
}

Status decode_size_from_lsb(DecodeInfo *decInfo){
    char buffer[32];
    long size=0;

    //read 32 byte from stego image
    if(fread(buffer,32,1,decInfo->fptr_stego_image)!=1){
        return e_failure;
    }

    //extract size from lsb
    for(int i=0;i<32;i++){
        int bit=buffer[i] & 1;
        size=size | ((long)bit <<i);
    }

    //store decoded size
    decInfo->size_secret_file=size;
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo){
    //open file
    if(open_decode_files(decInfo)==e_failure){
        printf("Unable to open files\n");
        return e_failure;
    }
    printf("File opened successfilly\n");
    
    //skip the header
    fseek(decInfo->fptr_stego_image,54,SEEK_SET);

    //decode and varify the magic string
    if(decode_magic_string(MAGIC_STRING,decInfo)==e_failure){
        printf("Invalid magic string\n");
        return e_failure;
    }

    //decode secret file extension
    if(decode_secret_file_extn_size(decInfo)==e_failure){
        printf("Unable to decode extension size\n");
        return e_failure;
    }

    //decode secret file extension
    if(decode_secret_file_extn(decInfo)==e_failure){
        printf("Unable to decode extension\n");
        return e_failure;
    }

    //decode secret file size
    if(decode_secret_file_size(decInfo)==e_failure){
        printf("Unable to decode secret file size\n");
        return e_failure;
    }

    //decode secret file data
    if(decode_secret_file_data(decInfo)==e_failure){
        printf("Unable to decode file data\n");
        return e_failure;
    }

    printf("Decoding completed\n");

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_secret);

    return e_success;
}
