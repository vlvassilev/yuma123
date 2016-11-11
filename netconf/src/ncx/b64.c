/*
 * Copyright (c) 2008 - 2012, Andy Bierman, All Rights Reserved.
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
static const char decodeCharacterTable[256] = {
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

// ---------------------------------------------------------------------------|
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

// ---------------------------------------------------------------------------|
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
    uint32_t validCharCount=0;
    uint32_t idx=0;

    while ( idx < inputlen )
    {
        if ( is_base64( inbuff[idx] ) )
        {
            ++validCharCount;
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
    return validCharCount;
}
// ---------------------------------------------------------------------------|
/**
 * Decode the valid_bytes_count+1 byte array pf base64 values into valid_bytes_count bytes.
 *
 * \param inbuff the buffer to decode
 * \param outbuff the output buffer for decoded bytes.
 * \param valid_bytes_count number of expected result bytes to convert
 */
static void decode_bytes( const uint8_t inbuff[4], uint8_t* outbuff, unsigned int valid_bytes_count)
{
    assert(valid_bytes_count>0 && valid_bytes_count<=3);
    outbuff[0] = (decodeCharacterTable[ inbuff[0] ] << 2) + 
                 ((decodeCharacterTable[ inbuff[1] ] & 0x30) >> 4);
    if(valid_bytes_count==1) return;
    outbuff[1] = ((decodeCharacterTable[ inbuff[1] ] & 0xf) << 4) + 
                 ((decodeCharacterTable[ inbuff[2] ] & 0x3c) >> 2);
    if(valid_bytes_count==2) return;
    outbuff[2] = ((decodeCharacterTable[ inbuff[2] ] & 0x3) << 6) + 
                 decodeCharacterTable[ inbuff[3] ];
}

// ---------------------------------------------------------------------------|
/**
 * Extract up to 4 bytes of base64 data to decode.
 * this function skips CR anf LF characters and extracts up to 4 bytes
 * from the input buffer. Any invalid characters are treated as end of
 * buffer markers.
 *
 * \param iterPos the start of the data to decode. This value is updated.
 * \param endpos the end of the buffer marker.
 * \param arr4 the output buffer for extracted bytes (zero padded if
 * less than 4 bytes were extracted)
 * \param numExtracted the number of valid bytes that were extracted.
 * output buffer for extracted bytes.
 * \return true if the end of buffer was reached.
 */
static bool extract_4bytes( const uint8_t** iterPos, const uint8_t* endPos, 
                            uint8_t arr4[4], uint32_t* numExtracted )
{
    const uint8_t* iter=*iterPos;
    uint32_t validBytes = 0;
    bool endReached = false;

    while ( iter < endPos && validBytes<4 && !endReached )
    {
        if ( is_base64( *iter ) )
        {
            arr4[validBytes++] = *iter;
            ++iter;
        }
        else if ( *iter == '\r' || *iter == '\n' ) 
        {
            ++iter;
        }
        else 
        {
            // encountered a dodgy character or an =
            if ( *iter != '=' )
            {
               log_warn( "b64_decode() encountered invalid character(%c), "
                         "output string truncated!", *iter );
            }

            // pad the remaining characters to decode
            size_t pad = validBytes;
            for ( ; pad<4; ++pad )
            {
                arr4[pad] = 0;
            }
            endReached = true;
        }
    }

    *numExtracted = validBytes;
    *iterPos = iter;
    return endReached ? endReached : iter >= endPos;
}

// ---------------------------------------------------------------------------|
static bool extract_3bytes( const uint8_t** iterPos, const uint8_t* endPos, 
                            uint8_t arr3[3], uint32_t* numExtracted )
{
    const uint8_t* iter=*iterPos;
    uint32_t byteCount = 0;

    while ( iter < endPos && byteCount < 3 )
    {
        arr3[ byteCount++ ] = *iter;
        ++iter;
    }

    *numExtracted = byteCount;
    while ( byteCount < 3 )
    {
        arr3[byteCount++] = 0;
    }

    *iterPos = iter;
    return iter >= endPos;
}


// ---------------------------------------------------------------------------|
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

// ---------------------------------------------------------------------------|
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

// ---------------------------------------------------------------------------|
uint32_t b64_get_decoded_str_len( const uint8_t* inbuff, size_t inputlen )
{
    uint32_t validCharCount= get_num_valid_characters( inbuff, inputlen );

    uint32_t requiredBufLen = 3*(validCharCount/4);
    uint32_t rem=validCharCount%4;
    if ( rem )
    {
        requiredBufLen += rem-1;
    }

    return requiredBufLen;
}

// ---------------------------------------------------------------------------|
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

status_t b64_decode ( const uint8_t* inbuff, uint32_t inbufflen,
                            uint8_t* outbuff, uint32_t outbufflen,
                            uint32_t* retlen )
{
    uint8_t arr4[4];
    uint32_t numExtracted = 0;
    bool endReached = false;
    const uint8_t* endPos = inbuff+inbufflen;
    const uint8_t* iter = inbuff;

    assert( inbuff && "b64_decode() inbuff is NULL!" );
    assert( outbuff && "b64_decode() outbuff is NULL!" );

    *retlen=0;

    while ( !endReached ) {
        endReached = extract_4bytes( &iter, endPos, arr4, &numExtracted ); 

        if ( numExtracted ) {
            if ( (*retlen+numExtracted-1)>outbufflen) {
                return ERR_BUFF_OVFL;
            }
            decode_bytes( arr4, outbuff+*retlen, numExtracted-1);
            *retlen += numExtracted-1;
        }
    }

    return NO_ERR;
}

/* END b64.c */

