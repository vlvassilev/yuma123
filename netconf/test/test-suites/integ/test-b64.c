#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "b64.h"

int check_b64_transcode(uint32_t data_len, uint32_t linesize)
{
    status_t res;

    uint8_t* inbuff;
    uint8_t* outbuff;
    uint8_t* transcoded_buff;
    uint32_t retlen;
    uint32_t transcoded_retlen;
    uint32_t expected_retlen;
    uint32_t u;
    inbuff=(uint8_t*)malloc(data_len);
    transcoded_buff=(uint8_t*)malloc(data_len);
    expected_retlen=4*((data_len+2)/3);
    if(linesize) {
        expected_retlen+=2*(expected_retlen/linesize);
    }

    outbuff=(uint8_t*)malloc(expected_retlen+1); /* NULL termination */

    for(u=0;u<((data_len+3)/4);u++) {
        inbuff[u*4+0]=(u&0x000000FF)>>0;
        if((u*4+1)>=data_len) break;
        inbuff[u*4+1]=(u&0x0000FF00)>>8;
        if((u*4+2)>=data_len) break;
        inbuff[u*4+2]=(u&0x00FF0000)>>16;
        if((u*4+3)>=data_len) break;
        inbuff[u*4+3]=(u&0xFF000000)>>24;
    }

    res=b64_encode(inbuff, data_len,
                   outbuff, expected_retlen+1,
                   linesize, &retlen);
    printf("%s\n",outbuff);
    assert(res==0);
    assert(retlen==expected_retlen);

    res=b64_decode(outbuff, retlen+1,
                   transcoded_buff, data_len, &transcoded_retlen);
    assert(transcoded_retlen==data_len);
    assert(0==memcmp(inbuff,transcoded_buff,data_len));
    free(inbuff);
    free(outbuff);
    free(transcoded_buff);
    return 0;
}

#define KNOWN_PAIRS_NUM 10
char* known_pairs[][2]={
{"1",          "MQ=="},
{"12",         "MTI="},
{"123",        "MTIz"},
{"1234",       "MTIzNA=="},
{"12345",      "MTIzNDU="},
{"123456",     "MTIzNDU2"},
{"1234567",    "MTIzNDU2Nw=="},
{"12345678",   "MTIzNDU2Nzg="},
{"123456789",  "MTIzNDU2Nzg5"},
{"1234567890", "MTIzNDU2Nzg5MA=="}
};

void check_b64_transcoder_w_known_pairs()
{
    status_t res;
    int i;
    uint32_t result_len;
    uint32_t expected_result_len;
    uint8_t* result_buff;
    printf("%d known pairs\n",KNOWN_PAIRS_NUM);
    for(i=0;i<KNOWN_PAIRS_NUM;i++) {
        /*encoder*/
        expected_result_len=strlen(known_pairs[i][1]);
        result_buff=(uint8_t*)malloc(expected_result_len+1);
        res=b64_encode(known_pairs[i][0], strlen(known_pairs[i][0]),
                       result_buff, expected_result_len+1,
                       0, &result_len);
        assert(res==NO_ERR);
        printf("%s should be %s\n", result_buff, known_pairs[i][1]);
        assert(0==strcmp(known_pairs[i][1],result_buff));
        assert(result_len==strlen(known_pairs[i][1]));
        free(result_buff);

        /*decoder*/
        expected_result_len=strlen(known_pairs[i][0]);
        result_buff=(uint8_t*)malloc(expected_result_len+1);
        result_buff[expected_result_len]=0;
        res=b64_decode(known_pairs[i][1], strlen(known_pairs[i][1]),
                       result_buff, expected_result_len+1,
                       &result_len);
        assert(res==NO_ERR);        
        printf("%s should be %s\n", result_buff, known_pairs[i][0]);
        assert(0==strcmp(known_pairs[i][0],result_buff));
        assert(result_len==strlen(known_pairs[i][0]));
        free(result_buff);

    }
}


int main(char** argv, unsigned int argc)
{
    int ret;
    int i,j;
    uint32_t data_len[] = {4, 10, 100, 1000, 10000};
    uint32_t linesize[] = {0, 1, 10, 100, 1000, 10000};

    check_b64_transcoder_w_known_pairs();
    
    for(i=0;i<sizeof(data_len)/sizeof(uint32_t);i++) {
        for(j=0;j<sizeof(linesize)/sizeof(uint32_t);j++) {
            printf("check_b64_transcode: data_len=%u, linesize=%u ... ",data_len[i],linesize[j]);
            ret=check_b64_transcode(data_len[i], linesize[j]);
            printf("OK\n");
        }
    }

    return 0;
}
