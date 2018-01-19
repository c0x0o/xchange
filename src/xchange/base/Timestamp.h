#ifndef XCHANGE_BASE_TIMESTAMP_H_
#define XCHANGE_BASE_TIMESTAMP_H_

#include <time.h>
#include <stdio.h>

#include <string>

namespace xchange {

namespace base {

    class Timestamp {
        public:
            explicit Timestamp() {
                clock_gettime(CLOCK_REALTIME, &time_);
            }
            Timestamp(const Timestamp &old) {
                time_.tv_sec = old.time_.tv_sec;
                time_.tv_nsec = old.time_.tv_nsec;
            }
            Timestamp(const Timestamp &&old) {
                time_.tv_sec = old.time_.tv_sec;
                time_.tv_nsec = old.time_.tv_nsec;
            }
            Timestamp(const struct timespec *spec) {
                time_.tv_sec = spec->tv_sec;
                time_.tv_nsec = spec->tv_nsec;
            }
            Timestamp(double spec) {
                unsigned long totalNs = static_cast<unsigned long>(spec * 1000000000);

                time_.tv_sec = totalNs / 1000000000;
                time_.tv_nsec = totalNs % 1000000000;
            }

            static Timestamp now() {return Timestamp();}
            Timestamp& operator=(const Timestamp &a) {
                time_.tv_sec = a.time_.tv_sec;
                time_.tv_nsec = a.time_.tv_nsec;

                return *this;
            }
            Timestamp& operator+=(const xchange::base::Timestamp &a) {
                time_.tv_sec += a.time_.tv_sec;
                time_.tv_nsec += a.time_.tv_nsec;

                return *this;
            }
            Timestamp& operator-=(const xchange::base::Timestamp &a) {
                time_.tv_sec -= a.time_.tv_sec;
                time_.tv_nsec -= a.time_.tv_nsec;

                return *this;
            }
            Timestamp operator+(const xchange::base::Timestamp &ts) const {
                return Timestamp(*this)+=ts;
            }
            Timestamp operator-(const xchange::base::Timestamp &ts) const {
                return Timestamp(*this)-=ts;
            }
            std::string toString() const {
                struct tm parsed;
                char buf[32] = {0};

                localtime_r(&time_.tv_sec, &parsed);
                snprintf(buf, 31, "%s, %02d %s %d %02d:%02d:%02d.%ld %s",
                        getDay(parsed.tm_wday), parsed.tm_mday, getMonth(parsed.tm_mon), parsed.tm_year+1900,
                        parsed.tm_hour, parsed.tm_min, parsed.tm_sec, time_.tv_nsec, parsed.tm_zone);

                return buf;
            }
            std::string toUTCString() const {
                struct tm parsed;
                char buf[32] = {0};

                gmtime_r(&time_.tv_sec, &parsed);
                snprintf(buf, 31, "%s, %02d %s %d %02d:%02d:%02d.%ld %s",
                        getDay(parsed.tm_wday), parsed.tm_mday, getMonth(parsed.tm_mon), parsed.tm_year+1900,
                        parsed.tm_hour, parsed.tm_min, parsed.tm_sec, time_.tv_nsec, parsed.tm_zone);

                return buf;
            }


            friend bool operator>(const Timestamp &a, const Timestamp &b) {
                if (a.time_.tv_sec > b.time_.tv_sec) {
                    return true;
                } else if (a.time_.tv_sec == b.time_.tv_sec && a.time_.tv_nsec > b.time_.tv_nsec) {
                    return true;
                }

                return false;
            }

            friend bool operator<(const Timestamp &a, const Timestamp &b) {
                if (a.time_.tv_sec < b.time_.tv_sec) {
                    return true;
                } else if (a.time_.tv_sec == b.time_.tv_sec && a.time_.tv_nsec < b.time_.tv_nsec) {
                    return true;
                }

                return false;
            }

            friend bool operator==(const Timestamp &a, const Timestamp &b) {
                if (a.time_.tv_sec == b.time_.tv_sec && a.time_.tv_nsec == b.time_.tv_nsec) {
                    return true;
                }

                return false;
            }

            friend bool operator!=(const Timestamp &a, const Timestamp &b) {
                if (a.time_.tv_sec != b.time_.tv_sec || a.time_.tv_nsec != b.time_.tv_nsec) {
                    return true;
                }

                return false;
            }

            friend bool operator>=(const Timestamp &a, const Timestamp &b) {
                if (a.time_.tv_sec > b.time_.tv_sec) {
                    return true;
                } else if (a.time_.tv_sec == b.time_.tv_sec && a.time_.tv_nsec >= b.time_.tv_nsec) {
                    return true;
                }

                return false;
            }

            friend bool operator<=(const Timestamp &a, const Timestamp &b) {
                if (a.time_.tv_sec < b.time_.tv_sec) {
                    return true;
                } else if (a.time_.tv_sec == b.time_.tv_sec && a.time_.tv_nsec <= b.time_.tv_nsec) {
                    return true;
                }

                return false;
            }


        private:
            struct timespec time_;

            inline const char *getMonth(int num) const {
                static const char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

                return month[num];
            }

            inline const char *getDay(int num) const {
                static const char *day[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

                return day[num];
            }
    };
}

}

#endif
