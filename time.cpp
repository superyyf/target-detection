#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

struct timespec init_time_remote;
struct timespec init_time_local;

struct Recv {
    uint8_t t_h, t_m, t_s;
    uint8_t t_ms; //unit: 10ms
};

struct SendData {
    uint8_t t_h, t_m, t_s;
    uint16_t t_ms; //unit: 1ms
};

//timestamp: years:months:days:hours:minutes:seconds
void init_time(const struct Recv *recv) {
    init_time_remote.tv_sec = recv->t_h * 3600l + recv->t_m * 60l + recv->t_s;
    init_time_remote.tv_nsec = recv->t_ms * 10l * 1000;
    clock_gettime(CLOCK_REALTIME, &init_time_local);
}

struct timespec timediff(const struct timespec *s1, const struct timespec *s2) {
    struct timespec ret;
    ret.tv_sec = s2->tv_sec - s1->tv_sec;
    ret.tv_nsec = s2->tv_nsec - s1->tv_nsec;
    if (ret.tv_nsec < 0) {
        --ret.tv_sec;
        ret.tv_nsec += 1000000000l;
    }
    return ret;
}

struct timespec timeadd(const struct timespec *s1, const struct timespec *s2) {
    struct timespec ret;
    ret.tv_sec = s1->tv_sec + s2->tv_sec;
    ret.tv_nsec = s1->tv_nsec + s2->tv_nsec;
    if (ret.tv_nsec >= 1000000000l) {
        ++ret.tv_sec;
        ret.tv_nsec -= 1000000000l;
    }
    return ret;
}

struct SendData timeconvert(const struct timespec *s) {
    struct SendData ret;
    long ms = s->tv_sec * 1000 + s->tv_nsec / 1000000;
    ret.t_ms = ms % 1000;
    ret.t_s = (ms / 1000) % 60;
    ret.t_m = (ms / (1000 * 60)) % 60;
    ret.t_h = ms / (1000 * 60 * 60);
    return ret;
}

void get_remote_time(struct SendData *send) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    struct timespec diff = timediff(&init_time_local, &now);
    struct timespec remote = timeadd(&init_time_remote, &diff);
    *send = timeconvert(&remote);
}

int main(void) {
    struct Recv recv;
    recv.t_h = 10;
    recv.t_m = 33;
    recv.t_s = 23;
    recv.t_ms = 12;

    init_time(&recv);
    struct timespec sl;
    sl.tv_sec = 12;
    sl.tv_nsec = 10240000;
    nanosleep(&sl, NULL);
    struct SendData data;
    get_remote_time(&data);
    printf("%u:%u:%u.%u", (uint32_t)data.t_h, (uint32_t)data.t_m, (uint32_t)data.t_s, (uint32_t)data.t_ms);
    return 0;
}
