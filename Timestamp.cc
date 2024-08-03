#include "Timestamp.h"
#include<time.h>
Timestamp::Timestamp():microSecondsSinceEpoch_(0){}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
:microSecondsSinceEpoch_ (microSecondsSinceEpoch)
{}
Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
}
std::string Timestamp::toString() const
{
    char buf[128] = {0};
    tm *time_t =  localtime(&microSecondsSinceEpoch_);
    snprintf(buf,128,"%4d/%02d/%02d %02d:%02d:%02d", 
        time_t->tm_year + 1900,
        time_t->tm_mon + 1,
        time_t->tm_mday,
        time_t->tm_hour,
        time_t->tm_min,
        time_t->tm_sec);
        return buf;
}

