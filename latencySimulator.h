//
// Created by wushuohan on 2019-11-09.
//Descriptions:
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include <zconf.h>

#include <list>
#include <vector>

#include "rudp.h"

#ifndef LATENCYSIMULATOR_H
#define LATENCYSIMULATOR_H

/* get system time */
static inline void itimeofday(long *sec, long *usec)
{
    struct timeval time;
	gettimeofday(&time, NULL);
	if (sec) *sec = time.tv_sec;
	if (usec) *usec = time.tv_usec;
}

static inline IINT64 iclock64(void)
{
    long s, u;
    IINT64 value;
    itimeofday(&s, &u);
    value = ((IINT64)s) * 1000 + (u / 1000);
    return value;
}

static inline IUINT32 iclock()
{
    return (IUINT32)(iclock64() & 0xfffffffful);
}

static inline void isleep(unsigned long millisecond)
{
    struct timespec ts;
	ts.tv_sec = (time_t)(millisecond / 1000);
	ts.tv_nsec = (long)((millisecond % 1000) * 1000000);
	/*nanosleep(&ts, NULL);*/
	usleep((millisecond << 10) - (millisecond << 4) - (millisecond << 3));
}

// 带延迟的数据包
class DelayPacket
{
public:
    virtual ~DelayPacket() {
        if (_ptr) delete _ptr;
        _ptr = NULL;
    }

    DelayPacket(int size, const void *src = NULL) {
        _ptr = new unsigned char[size];
        _size = size;
        if (src) {
            memcpy(_ptr, src, size);
        }
    }

    unsigned char* ptr() { return _ptr; }
    const unsigned char* ptr() const { return _ptr; }

    int size() const { return _size; }
    IUINT32 ts() const { return _ts; }
    void setts(IUINT32 ts) { _ts = ts; }

protected:
    unsigned char *_ptr;
    int _size;
    IUINT32 _ts;
};

// 均匀分布的随机数
class Random
{
public:
    Random(int size) {
        this->size = 0;
        seeds.resize(size);
    }

    int random() {
        int x, i;
        if (seeds.size() == 0) return 0;
        if (size == 0) {
            for (i = 0; i < (int)seeds.size(); i++) {
                seeds[i] = i;
            }
            size = (int)seeds.size();
        }
        i = rand() % size;
        x = seeds[i];
        seeds[i] = seeds[--size];
        return x;
    }

protected:
    int size;
    std::vector<int> seeds;
};

class LatencySimulator
{
public:

    virtual ~LatencySimulator() {
        clear();
    }

    LatencySimulator(int lostrate = 10, int rttmin = 60, int rttmax = 125, int nmax = 1000):
            r12(100), r21(100) {
        current = iclock();
        this->lostrate = lostrate / 2;	// 上面数据是往返丢包率，单程除以2
        this->rttmin = rttmin / 2;
        this->rttmax = rttmax / 2;
        this->nmax = nmax;
        tx1 = tx2 = 0;
    }

    // 清除数据
    void clear() {
        DelayTunnel::iterator it;
        for (it = p12.begin(); it != p12.end(); it++) {
            delete *it;
        }
        for (it = p21.begin(); it != p21.end(); it++) {
            delete *it;
        }
        p12.clear();
        p21.clear();
    }

    void send(int peer, const void *data, int size) {
        if (peer == 0) {
            tx1++;
            if (r12.random() < lostrate) return;
            if ((int)p12.size() >= nmax) return;
        }	else {
            tx2++;
            if (r21.random() < lostrate) return;
            if ((int)p21.size() >= nmax) return;
        }
        DelayPacket *pkt = new DelayPacket(size, data);
        current = iclock();
        IUINT32 delay = rttmin;
        if (rttmax > rttmin) delay += rand() % (rttmax - rttmin);
        pkt->setts(current + delay);
        if (peer == 0) {
            p12.push_back(pkt);
        }	else {
            p21.push_back(pkt);
        }
    }

    int recv(int peer, void *data, int maxsize) {
        DelayTunnel::iterator it;
        if (peer == 0) {
            it = p21.begin();
            if (p21.size() == 0) return -1;
        }	else {
            it = p12.begin();
            if (p12.size() == 0) return -1;
        }
        DelayPacket *pkt = *it;
        current = iclock();
        if (current < pkt->ts()) return -2;
        if (maxsize < pkt->size()) return -3;
        if (peer == 0) {
            p21.erase(it);
        }	else {
            p12.erase(it);
        }
        maxsize = pkt->size();
        memcpy(data, pkt->ptr(), maxsize);
        delete pkt;
        return maxsize;
    }

public:
    int tx1;
    int tx2;

protected:
    IUINT32 current;
    int lostrate;
    int rttmin;
    int rttmax;
    int nmax;
    typedef std::list<DelayPacket*> DelayTunnel;
    DelayTunnel p12;
    DelayTunnel p21;
    Random r12;
    Random r21;
};

#endif //LATENCYSIMULATOR_H

