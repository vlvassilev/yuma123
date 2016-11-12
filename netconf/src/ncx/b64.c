/*
 * Copyright (c) 2008 - 2012, Andy Bierman, All Rights Reserved.
 * Copyright (c) 2013 - 2016, Vladimir Vassilev, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */

#include "b64.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"

/** translation Table used for encoding characters */
static const char encode_character_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "abcdefghijklmnopqrstuvwxyz"
                                           "0123456789+/";

/** translation Table used for decoding characters */
static const char decode_character_table[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

/**
 * Determine if a character is a valid base64 value.
 *
 * \param c the character to test.
 * \return true if c is a base64 value.
 */
static bool is_base64(unsigned char c)
{
   return (isalnum(c) || (c == '+') || (c == '/'));
}

/**
 * Count the number of valid base64 characters in the supplied input
 * buffer.
 * @desc this function counts the number of valid base64 characters,
 * skipping CR and LF characters. Invalid characters are treated as
 * end of buffer markers.
 *
 * \param inbuff the buffer to check.
 * \param inputlen the length of the input buffer.
 * \return the number of valid base64 characters.
 */
static uint32_t get_num_valid_characters( const uint8_t* inbuff, 
                                          size_t inputlen ) 
{
    uint32_t valid_char_count=0;
    uint32_t idx=0;

    while ( idx < inputlen )
    {
        if ( is_base64( inbuff[idx] ) )
        {
            ++valid_char_count;
            ++idx;
        }
        else if ( inbuff[idx] == '\r' || inbuff[idx] == '\n' ) 
        {
            ++idx;
        }
        else
        {
            break;
        }
    }
    return valid_char_count;
}

/**
 * Decode the valid_bytes_count+1 byte array pf base64 values into valid_bytes_count bytes.
 *
 * \param inbuff the buffer to decode
 * \param outbuff the output buffer for decoded bytes.
 * \param valid_data_bytes_count number of expected result bytes to convert
 */
static void decode_bytes( const uint8_t* b64, uint8_t* data, unsigned int valid_data_bytes_count)
{
    assert(valid_data_bytes_count>0 && valid_data_bytes_count<=3);
    data[0] = (decode_character_table[ b64[0] ] << 2) +
                 ((decode_character_table[ b64[1] ] & 0x30) >> 4);
    if(valid_data_bytes_count==1) return;
    data[1] = ((decode_character_table[ b64[1] ] & 0xf) << 4) +
                 ((decode_character_table[ b64[2] ] & 0x3c) >> 2);
    if(valid_data_bytes_count==2) return;
    data[2] = ((decode_character_table[ b64[2] ] & 0x3) << 6) +
                 decode_character_table[ b64[3] ];
}

static void encode_3bytes(const uint8_t data[3], uint8_t b64[4])
{

    b64[0] = encode_character_table[ ( data[0] & 0xfc) >> 2 ];
    b64[1] = encode_character_table[ ( ( data[0] & 0x03 ) << 4 ) +
                                          ( ( data[1] & 0xf0 ) >> 4 ) ];
    b64[2] = encode_character_table[ ( ( data[1] & 0x0f ) << 2 ) +
                                          ( ( data[2] & 0xc0 ) >> 6 ) ];
    b64[3] = encode_character_table[ data[2] & 0x3f ];
}

static void encode_last_2bytes(const uint8_t data[3], uint8_t b64[4])
{

    b64[0] = encode_character_table[ ( data[0] & 0xfc) >> 2 ];
    b64[1] = encode_character_table[ ( ( data[0] & 0x03 ) << 4 ) +
                                          ( ( data[1] & 0xf0 ) >> 4 ) ];
    b64[2] = encode_character_table[ ( ( data[1] & 0x0f ) << 2 )];
    b64[3] = '=';
}

static void encode_last_1byte(const uint8_t data[3], uint8_t b64[4])
{

    b64[0] = encode_character_table[ ( data[0] & 0xfc) >> 2 ];
    b64[1] = encode_character_table[ ( ( data[0] & 0x03 ) << 4 )]; 
    b64[2] = '=';
    b64[3] = '=';
}

/*************** E X T E R N A L    F U N C T I O N S  *************/

uint32_t b64_get_encoded_str_len( uint32_t inbufflen, uint32_t linesize )
{
    uint32_t outbufflen = inbufflen%3 ? 4 * ( 1 + inbufflen/3 )  
                                          : 4 * ( inbufflen/3 );
    if(linesize != 0) {
        outbufflen += 2 * ( inbufflen/linesize );  /* allow for line breaks*/
    }
    outbufflen += 1; /* NULL termination */
    return outbufflen;
}

uint32_t b64_get_decoded_str_len( const uint8_t* inbuff, size_t inputlen )
{
    uint32_t valid_char_count= get_num_valid_characters( inbuff, inputlen );

    uint32_t required_buf_len = 3*(valid_char_count/4);
    uint32_t rem=valid_char_count%4;
    if ( rem )
    {
        required_buf_len += rem-1;
    }

    return required_buf_len;
}

status_t b64_encode ( const uint8_t* inbuff, uint32_t inbufflen,
                            uint8_t* outbuff, uint32_t outbufflen,
                            uint32_t linesize, uint32_t* retlen)
{
    uint32_t i,j,wrapindex;
    uint8_t b64_4[4];
    uint8_t* outptr;

    assert( inbuff && "b64_decode() inbuff is NULL!" );
    assert( outbuff && "b64_decode() outbuff is NULL!" );
    if ( b64_get_encoded_str_len( inbufflen, linesize ) > outbufflen ) {
        return ERR_BUFF_OVFL;
    }

    outptr=outbuff;
    wrapindex=0;

    for(i=0;i<((inbufflen+2)/3);i++) {

        if((inbufflen-i*3)==1) {
            encode_last_1byte(inbuff+i*3,b64_4);
        } else if((inbufflen-i*3)==2) {
            encode_last_2bytes(inbuff+i*3,b64_4);
        } else {
            encode_3bytes(inbuff+i*3,b64_4);
        }

        for(j=0;j<4;j++) {
            *outptr++=b64_4[j];
            if(linesize && ++wrapindex==linesize) {
                *outptr++='\r';
                *outptr++='\n';
                wrapindex=0;
            }
        }
    }

    *retlen=outptr-outbuff;
    *outptr++='\0';
    return NO_ERR;
}

status_t b64_decode( const uint8_t* inbuff, uint32_t inbufflen,
                            uint8_t* outbuff, uint32_t outbufflen,
                            uint32_t* retlen )
{
    int i;
    int b64_byte_index;
    uint8_t b64_4[4];
    int padding;

    assert( inbuff && "b64_decode() inbuff is NULL!" );
    assert( outbuff && "b64_decode() outbuff is NULL!" );

    b64_byte_index=0;
    padding=0;
    *retlen=0;
    for(i=0;i<inbufflen;i++) {
        if(is_base64(inbuff[i]) && padding==0) {
            b64_4[b64_byte_index++]=inbuff[i];
        } else if(inbuff[i]=='\r' || inbuff[i]=='\n') {
            /*do nothing skip*/
        } else if(inbuff[i]=='=' && b64_byte_index>=2) {
            if(padding==0) {
                padding=4-b64_byte_index;
            }
            b64_4[b64_byte_index++]=inbuff[i];
        } else {
            /* encountered a dodgy character */
            log_warn( "b64_decode() encountered invalid character(%c), "
                      "output string truncated!", inbuff[i]);
            return ERR_NCX_WRONG_TKVAL;
        }
        if(b64_byte_index==4) {
            if((*retlen+3-padding)>outbufflen) {
                return ERR_BUFF_OVFL;
            }
            b64_byte_index=0;
            decode_bytes(b64_4, outbuff+*retlen, 3-padding);
            *retlen+=3-padding;
        }
    }

    if(b64_byte_index!=0) {
        /* encountered a dodgy character */
        log_warn( "b64_decode() encountered trailing %d bytes data not aligned to 4 bytes!", b64_byte_index);
        return ERR_NCX_WRONG_TKVAL;
    }

    return NO_ERR;
}

/* END b64.c */
