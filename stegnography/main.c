#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "encode.h"
#include "decode.h"
#include "common.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[]) // .aout -e <source_file> <secret_data_file> <optional>
{

    //check the numbner of arguments
    if(argc<3){
        printf("Enter valid argument\n");
        return 1;
    }
    //step 1 -> check check_operation_type(argv[1]) is returning e_encode or not
    OperationType operation=check_operation_type(argv[1]);

        // yes -> EncodeInfo encInfo;
    if(operation==e_encode){
        EncodeInfo encInfo;

        // check read_and_validate_encode_args(argv, &encInfo) is returning e_success or e_failure
                // failure -> print error msg and stop
                // success -> print success msg 
        if(read_and_validate_encode_args(argv,&encInfo)==e_failure){
            printf("Invalid encoding arguments\n");
            return 1;
        }
        printf("Encoding arguments validated\n");
        // check do_encoding(&encInfo) is returning e_success or e_failure
                // failure -> print error msg and stop
                // success -> print success msg and stop
        if(do_encoding(&encInfo)==e_failure){
            printf("Encoding failed\n");
            return 1;
        }
        printf("Encoding completed\n");
         
        }else if(operation==e_decode){
            DecodeInfo decInfo;

            //validate decoding arguments
            if(read_and_validate_decode_args(argv,&decInfo)==e_failure){
                printf("Invalid decoding arguments\n");
                return 1;
            }
            printf("Decode argument validated\n");

            //perform decoding
            if(do_decoding(&decInfo)==e_failure){
                printf("Decode failed\n");
                return 1;
            }
            printf("Decode completed\n");
        }else{
            printf("Unsupported argument\n");
            printf("Use -e for encoding or -d for decoding\n");
            return 1;
        }
}

OperationType check_operation_type(char *symbol)
{
    //step 1 -> check symbol is -e or not
        // yes -> return e_encode
    if(strcmp(symbol,"-e")==0){
        return e_encode;
    }

    // step 2 -> check symbol is -d or not
        // yes -> return e_decode
    if(strcmp(symbol,"-d")==0){
        return e_decode;
    }

    // return e_unsupported
    return e_unsupported;

}
