#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr,0,SEEK_END);
    uint size=ftell(fptr);
    rewind(fptr);
    // return secret file size
    return size;

}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // step 1 -> check argv[2] is having .bmp or not
        //yes -> store the file name into encInfo -> src_image_fname = argv[2]
        //no - > return e_failure

        if(strstr(argv[2],".bmp")!=NULL){
            encInfo->src_image_fname=argv[2];
        }else{
            return e_failure;
        }

    // step 2 -> check argv[3] is having extn there or not 
        // yes - > store the file name into encInfo -> secret_fname = argv[3]
        // no -> return e_failure
        if(strstr(argv[3],".") != NULL){
        encInfo->secret_fname = argv[3];

        char *extn = strrchr(argv[3], '.');

        if(strlen(extn) >= sizeof(encInfo->extn_secret_file))
        {
        printf("Extension is too long\n");
        return e_failure;
        }

        strcpy(encInfo->extn_secret_file, extn);
        }else{
        return e_failure;
        }

    
    // step 3 -> check argv[4] is NULL or not
        //yes -> check argv[4] is having .bmp or not
                //yes -> store the file name into encInfo -> stego_image_fname = argv[4]
                //no - > return e_failure
        
        //no -> store default name the encInfo -> stego_image_fname = "stego.bmp"
        if(argv[4]!=NULL){
            if(strstr(argv[4],".bmp")!=NULL){
                encInfo->stego_image_fname=argv[4];
            }else{
                return e_failure;
            }
        }else{
            encInfo->stego_image_fname="stego.bmp";
        }

    // return e_success;
    return e_success; 


}

Status open_encode_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    printf("Secret filename: %s\n", encInfo->secret_fname);
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    
    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }
    //printf("Secret file size = %ld bytes\n",encInfo->size_secret_file);
    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // Magic                 = 2 × 8 = 16
    // Extension size        = 4 × 8 = 32
    // Extension             = 4 × 8 = 32
    // Secret size           = 4 × 8 = 32
    // Secret data           = 9 × 8 = 72

    //step 1 -> encInfo -> image_capacity = call get_image_size_for_bmp(encoInfo -> fptr_src_image)
    encInfo->image_capacity=get_image_size_for_bmp(encInfo->fptr_src_image);

    //step 2 -> encInfo -> size_secret_file = call get_file_size(encInfo -> fptr_secret)
    encInfo -> size_secret_file =get_file_size(encInfo -> fptr_secret);
    
    //step 3 -> check encInfo -> image_capacity > 16 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8)
                //t -> return e_success;
                //f -> return e_failure
    if(encInfo->image_capacity>(16+32+(strlen(encInfo->extn_secret_file)*8)+32+(encInfo->size_secret_file*8))){
        return e_success;
    }
    return e_failure;
   
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{   

    // step 1 -> rewind source file pointer
    rewind(fptr_src_image);

    // declare one character buffer of size 54
    char header[54];

    // step 2 -> read 54 bytes from source file fread()
    fread(header,54,1,fptr_src_image);

    // step 3 -> write 54 bytes to dest file fwrite()
    fwrite(header,54,1,fptr_dest_image);

    // return e_success
    return e_success;
}


Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i = 0; i < strlen(magic_string); i++)
    {
        // Read 8 bytes from source image
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
        {
            return e_failure;
        }

        // Encode one magic-string character
        encode_byte_to_lsb(magic_string[i], buffer);

        // Write encoded bytes to stego image
        if (fwrite(buffer, 8, 1, encInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }
    }
    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    // char buffer[32];
    char buffer[32];

    //step 1 -> read 32 bytes from source file
    fread(buffer,32,1,encInfo->fptr_src_image);

    //step 2 -> call encode_size_to_lsb(size, buffer)
    encode_size_to_lsb(size,buffer);

    //step 3 -> write the buffer into dest file
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // char buffer[8];
    char buffer[8];

    for(int i=0;i<strlen(file_extn);i++){
    //step 1 -> read 8 bytes from source file
    fread(buffer,8,1,encInfo->fptr_src_image);

    //step 2 -> call encode_byte_to_lsb(file_extn[i], buffer)
    encode_byte_to_lsb(file_extn[i],buffer);

    //step 3 -> write the buffer into dest file
    fwrite(buffer,8,1,encInfo->fptr_stego_image);

    // repeat this for strlen(file_extn) times from step 1  
    } 
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // char buffer[32];
    char buffer[32];

    //step 1 -> read 32 bytes from source file
    if(fread(buffer, 1, 32, encInfo->fptr_src_image) != 32){
        return e_failure;
    }

    //step 2 -> call encode_size_to_lsb(file_size, buffer)
    for(int i = 0; i < 32; i++){
        int bit = (file_size  >> i) & 1;
        buffer[i] = (buffer[i] & 0xFE) | bit;
    }

    //step 3 -> write the buffer into dest file
    if(fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32) {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);

    char buffer[8];
    char data;

    for(long i = 0; i < encInfo->size_secret_file; i++)
    {
        // Read one byte from secret file
        if(fread(&data, 1, 1, encInfo->fptr_secret) != 1)
        {
            return e_failure;
        }

        // Read 8 bytes from source image
        if(fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        // Encode secret byte into image LSBs
        encode_byte_to_lsb(data, buffer);

        // Write encoded bytes into stego image
        if(fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while(fread(&ch, 1, 1, fptr_src) == 1)   // Read remaining bytes
    {
        fwrite(&ch, 1, 1, fptr_dest);       // Write to destination
    }

    if(ftell(fptr_src) == ftell(fptr_dest))  // Check if both pointers match
    {
        return e_success;
    }
    return e_failure;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // Repeat for 8 bits
    for(int i=0;i<8;i++){

    // Extract LSB from data
       int bit= data & 1;

    // Clear LSB of image byte
       image_buffer[i]=image_buffer[i] & (~1); 

    // Put data bit into image LSB
       image_buffer[i]=image_buffer[i] | bit;

    // Shift data right
       data=data >> 1;
    }
    return e_success;

}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    // write logic to encode the int
    for(int i=0;i<32;i++){
        int bit=size & 1;
        imageBuffer[i]=imageBuffer[i] & (~1);
        imageBuffer[i]=imageBuffer[i] | bit;
        size=size >> 1;
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //step 1 -> check open_files(encInfo) is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
    if(open_encode_files(encInfo)==e_failure){
        printf("Unable to open files\n");
        return e_failure;
    }else{
        printf("File opened successfully\n");
    }

    //step 2 -> check check_capacity(encInfo) is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
    if(check_capacity(encInfo)==e_failure){
        printf("Insufficient image capacity\n");
        return e_failure;
    }else{
        printf("Image capacity is sufficient\n");
    }

    //step 3 -> call copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure){
        printf("Unable to copy BMP header\n");
        e_failure;
    }else{
        printf("BMP header copied\n");
    }

    //step 4 -> call encode_magic_string(MAGIC_STRING, encInfo)
    if(encode_magic_string(MAGIC_STRING,encInfo)==e_failure){
        printf("Unable to encode magic string\n");
        return e_failure;
    }

    //step 5 -> call encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo)
    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo)==e_failure){
        printf("Unable to encode extension size\n");
        return e_failure;
    }

    //step 6 -> call encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_failure){
        printf("Unable to encode file extension\n");
        return e_failure;
    }

    //step 7 -> call encode_secret_file_size(encInfo ->  size_secret_file, encInfo)
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_failure){
        printf("Unable to encode extension size\n");
        return e_failure;
    }

    //step 8 -> call encode_secret_file_data(encInfo)
    if(encode_secret_file_data(encInfo)==e_failure){
        printf("Unable to ecode file data\n");
        return e_failure;
    }

    //step 9 -> copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure){
        printf("Unable to copy remaining image data\n");
        return e_failure;
    }else{
        printf("Remaining image data copied\n");
    }

    // return e_success
    return e_success;
}