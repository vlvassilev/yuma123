#include <inttypes.h>

void ieee_1588_to_yang_date_and_time(uint64_t sec, uint32_t nsec, char* date_and_time);
int yang_date_and_time_to_ieee_1588(char* date_and_time, uint64_t* sec, uint32_t* nsec);
