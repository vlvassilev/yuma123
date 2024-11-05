#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>

void ieee_1588_to_yang_date_and_time(uint64_t sec, uint32_t nsec, char* date_and_time)
{
    struct tm my_tm;
    time_t time_t_sec = (time_t)sec;
    gmtime_r(&(time_t_sec), &my_tm);
    (void)sprintf((char *)date_and_time,
                          "%04u-%02u-%02uT%02u:%02u:%02u.%09uZ",
                          (uint32_t)(my_tm.tm_year+1900),
                          (uint32_t)(my_tm.tm_mon+1),
                          (uint32_t)my_tm.tm_mday,
                          (uint32_t)my_tm.tm_hour,
                          (uint32_t)my_tm.tm_min,
                          (uint32_t)my_tm.tm_sec,
                          (uint32_t)nsec);

}

int yang_date_and_time_to_ieee_1588(char* date_and_time, uint64_t* sec, uint32_t* nsec)
{
    /* YYYY-MM-DDThh:mm:ss.n*Z */
    struct tm tm;
    char nsec_str[]="000000000";

    unsigned int date_and_time_len;
    unsigned int nsec_digits;
    unsigned int i;
    int ret;
    char* ptr;

    *sec=0;
    *nsec=0;

    date_and_time_len = strlen(date_and_time);


    if(date_and_time_len < strlen("YYYY-MM-DDThh:mm:ssZ")) {
        return -1;
    }

    if(date_and_time[date_and_time_len-1] != 'Z') {
        return -1;
    }

    memset(&tm, 0, sizeof(struct tm));
    ret = sscanf(date_and_time, "%04d-%02d-%02dT%02d:%02d:%02d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    if(ret!=6) {
        return -1;
    }
    tm.tm_year = tm.tm_year - 1900;
    tm.tm_mon = tm.tm_mon - 1;

    tm.tm_isdst = 0;
    ptr = date_and_time + strlen("YYYY-MM-DDThh:mm:ss");

    *sec = mktime(&tm);

    if(ptr==(date_and_time+date_and_time_len-1)) {
        return 0;
    }

    if(*ptr!='.') {
        return -1;
    }

    ptr++;

    nsec_digits = date_and_time_len-strlen("YYYY-MM-DDThh:mm:ss.")-strlen("Z");

    for(i=0;i<nsec_digits;i++) {
        if(ptr[i]<'0' || ptr[i]>'9') {
            return -1;
        }
        nsec_str[i] = ptr[i];
    }

    *nsec = atoi(nsec_str);

    return 0;
}

