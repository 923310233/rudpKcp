
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>


typedef unsigned int ISTDUINT32;
typedef int ISTDINT32;


typedef char IINT8;


typedef unsigned char IUINT8;

typedef unsigned short IUINT16;

typedef short IINT16;

typedef ISTDINT32 IINT32;

typedef ISTDUINT32 IUINT32;


typedef long long IINT64;

typedef unsigned long long IUINT64;


struct IQUEUEHEAD {
    struct IQUEUEHEAD *next, *prev;
};

typedef struct IQUEUEHEAD iqueue_head;


//---------------------------------------------------------------------
// queue init                                                         
//---------------------------------------------------------------------
#define IQUEUE_HEAD_INIT(name) { &(name), &(name) }
#define IQUEUE_HEAD(name) \
    struct IQUEUEHEAD name = IQUEUE_HEAD_INIT(name)

#define IQUEUE_INIT(ptr) ( \
    (ptr)->next = (ptr), (ptr)->prev = (ptr))

#define IOFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define ICONTAINEROF(ptr, type, member) ( \
        (type*)( ((char*)((type*)ptr)) - IOFFSETOF(type, member)) )

#define IQUEUE_ENTRY(ptr, type, member) ICONTAINEROF(ptr, type, member)


//---------------------------------------------------------------------
// queue operation
//---------------------------------------------------------------------
#define IQUEUE_ADD(node, head) ( \
    (node)->prev = (head), (node)->next = (head)->next, \
    (head)->next->prev = (node), (head)->next = (node))

#define IQUEUE_ADD_TAIL(node, head) ( \
    (node)->prev = (head)->prev, (node)->next = (head), \
    (head)->prev->next = (node), (head)->prev = (node))

#define IQUEUE_DEL_BETWEEN(p, n) ((n)->prev = (p), (p)->next = (n))

#define IQUEUE_DEL(entry) (\
    (entry)->next->prev = (entry)->prev, \
    (entry)->prev->next = (entry)->next, \
    (entry)->next = 0, (entry)->prev = 0)

#define IQUEUE_DEL_INIT(entry) do { \
    IQUEUE_DEL(entry); IQUEUE_INIT(entry); } while (0)

#define IQUEUE_IS_EMPTY(entry) ((entry) == (entry)->next)

#define iqueue_init        IQUEUE_INIT
#define iqueue_entry    IQUEUE_ENTRY
#define iqueue_add        IQUEUE_ADD
#define iqueue_add_tail    IQUEUE_ADD_TAIL
#define iqueue_del        IQUEUE_DEL
#define iqueue_del_init    IQUEUE_DEL_INIT
#define iqueue_is_empty IQUEUE_IS_EMPTY

#define IQUEUE_FOREACH(iterator, head, TYPE, MEMBER) \
    for ((iterator) = iqueue_entry((head)->next, TYPE, MEMBER); \
        &((iterator)->MEMBER) != (head); \
        (iterator) = iqueue_entry((iterator)->MEMBER.next, TYPE, MEMBER))

#define iqueue_foreach(iterator, head, TYPE, MEMBER) \
    IQUEUE_FOREACH(iterator, head, TYPE, MEMBER)

#define iqueue_foreach_entry(pos, head) \
    for( (pos) = (head)->next; (pos) != (head) ; (pos) = (pos)->next )


#define __iqueue_splice(list, head) do {    \
        iqueue_head *first = (list)->next, *last = (list)->prev; \
        iqueue_head *at = (head)->next; \
        (first)->prev = (head), (head)->next = (first);        \
        (last)->next = (at), (at)->prev = (last); }    while (0)

#define iqueue_splice(list, head) do { \
    if (!iqueue_is_empty(list)) __iqueue_splice(list, head); } while (0)

#define iqueue_splice_init(list, head) do {    \
    iqueue_splice(list, head);    iqueue_init(list); } while (0)



#ifndef IWORDS_BIG_ENDIAN
#ifdef _BIG_ENDIAN_
#if _BIG_ENDIAN_
#define IWORDS_BIG_ENDIAN 1
#endif
#endif
#ifndef IWORDS_BIG_ENDIAN
#if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
#define IWORDS_BIG_ENDIAN 1
#endif
#endif
#ifndef IWORDS_BIG_ENDIAN
#define IWORDS_BIG_ENDIAN  0
#endif
#endif

#ifndef IWORDS_MUST_ALIGN
#if defined(__i386__) || defined(__i386) || defined(_i386_)
#define IWORDS_MUST_ALIGN 0
#elif defined(_M_IX86) || defined(_X86_) || defined(__x86_64__)
#define IWORDS_MUST_ALIGN 0
#elif defined(__amd64) || defined(__amd64__)
#define IWORDS_MUST_ALIGN 0
#else
#define IWORDS_MUST_ALIGN 1
#endif
#endif


struct IKCPSEG {
    struct IQUEUEHEAD node;
    IUINT32 conv;
    IUINT32 cmd;
    IUINT32 frg;
    IUINT32 wnd;
    IUINT32 ts;
    IUINT32 sn;
    IUINT32 una;
    IUINT32 len;
    IUINT32 resendts;
    IUINT32 rto;
    IUINT32 fastack;
    IUINT32 xmit;
    char data[1];
};


//---------------------------------------------------------------------
// IKCPCB
//---------------------------------------------------------------------
struct IKCPCB {
    IUINT32 conv, mtu, mss, state;
    IUINT32 snd_una, snd_nxt, rcv_nxt;
    IUINT32 ts_recent, ts_lastack, ssthresh;
    IINT32 rx_rttval, rx_srtt, rx_rto, rx_minrto;
    IUINT32 snd_wnd, rcv_wnd, rmt_wnd, cwnd, probe;
    IUINT32 current, interval, ts_flush, xmit;
    IUINT32 nrcv_buf, nsnd_buf;
    IUINT32 nrcv_que, nsnd_que;
    IUINT32 nodelay, updated;
    IUINT32 ts_probe, probe_wait;
    IUINT32 dead_link, incr;
    struct IQUEUEHEAD snd_queue;
    struct IQUEUEHEAD rcv_queue;
    struct IQUEUEHEAD snd_buf;
    struct IQUEUEHEAD rcv_buf;
    IUINT32 *acklist;
    IUINT32 ackcount;
    IUINT32 ackblock;
    void *user;
    char *buffer;
    int fastresend;
    int fastlimit;
    int nocwnd, stream;
    int logmask;

    int (*output)(const char *buf, int len, struct IKCPCB *kcp, void *user);

    void (*writelog)(const char *log, struct IKCPCB *kcp, void *user);
};


typedef struct IKCPCB ikcpcb;

#define IKCP_LOG_OUTPUT            1
#define IKCP_LOG_INPUT            2
#define IKCP_LOG_SEND            4
#define IKCP_LOG_RECV            8
#define IKCP_LOG_IN_DATA        16
#define IKCP_LOG_IN_ACK            32
#define IKCP_LOG_IN_PROBE        64
#define IKCP_LOG_IN_WINS        128
#define IKCP_LOG_OUT_DATA        256
#define IKCP_LOG_OUT_ACK        512
#define IKCP_LOG_OUT_PROBE        1024
#define IKCP_LOG_OUT_WINS        2048

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// interface
//---------------------------------------------------------------------

// create a new kcp control object, 'conv' must equal in two endpoint
// from the same connection. 'user' will be passed to the output callback
// output callback can be setup like this: 'kcp->output = my_udp_output'
ikcpcb *ikcp_create(IUINT32 conv, void *user);

// release kcp control object
void ikcp_release(ikcpcb *kcp);

void ikcp_setoutput(ikcpcb *kcp, int (*output)(const char *buf, int len,
                                               ikcpcb *kcp, void *user));

int ikcp_recv(ikcpcb *kcp, char *buffer, int len);

int ikcp_send(ikcpcb *kcp, const char *buffer, int len);

void ikcp_update(ikcpcb *kcp, IUINT32 current);

IUINT32 ikcp_check(const ikcpcb *kcp, IUINT32 current);

int ikcp_input(ikcpcb *kcp, const char *data, long size);

// flush pending data
void ikcp_flush(ikcpcb *kcp);

// check the size of next message in the recv queue
int ikcp_peeksize(const ikcpcb *kcp);

// change MTU size, default is 1400
int ikcp_setmtu(ikcpcb *kcp, int mtu);

int ikcp_wndsize(ikcpcb *kcp, int sndwnd, int rcvwnd);

int ikcp_waitsnd(const ikcpcb *kcp);

int ikcp_nodelay(ikcpcb *kcp, int nodelay, int interval, int resend, int nc);


void ikcp_log(ikcpcb *kcp, int mask, const char *fmt, ...);

void ikcp_allocator(void *(*new_malloc)(size_t), void (*new_free)(void *));

IUINT32 ikcp_getconv(const void *ptr);




