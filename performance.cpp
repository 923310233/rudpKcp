//
// Created by wushuohan on 2019-11-09.
//Descriptions:
//

#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "rudp.c"


LatencySimulator *vnet;

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
    union {
        int id;
        void *ptr;
    } parameter;
    parameter.ptr = user;
    vnet->send(parameter.id, buf, len);
    return 0;
}


void test(int mode) {
    vnet = new LatencySimulator(10, 60, 125);
    ikcpcb *kcp1 = ikcp_create(0x11223344, (void *) 0);
    ikcpcb *kcp2 = ikcp_create(0x11223344, (void *) 1);
    
    kcp1->output = udp_output;
    kcp2->output = udp_output;

    IUINT32 current = iclock();
    IUINT32 slap = current + 20;
    IUINT32 index = 0;
    IUINT32 next = 0;
    IINT64 sumrtt = 0;
    int count = 0;
    int maxrtt = 0;

    ikcp_wndsize(kcp1, 128, 128);
    ikcp_wndsize(kcp2, 128, 128);
    
    if (mode == 0) {
        ikcp_nodelay(kcp1, 0, 10, 0, 0);
        ikcp_nodelay(kcp2, 0, 10, 0, 0);
    } else if (mode == 1) {
        ikcp_nodelay(kcp1, 0, 10, 0, 1);
        ikcp_nodelay(kcp2, 0, 10, 0, 1);
    } else {
        ikcp_nodelay(kcp1, 1, 10, 2, 1);
        ikcp_nodelay(kcp2, 1, 10, 2, 1);
        kcp1->rx_minrto = 10;
        kcp1->fastresend = 1;
    }


    char buffer[2000];
    int hr;

    IUINT32 ts1 = iclock();

    while (1) {
        isleep(1);
        current = iclock();
        ikcp_update(kcp1, iclock());
        ikcp_update(kcp2, iclock());
        
        for (; current >= slap; slap += 20) {
            ((IUINT32 *) buffer)[0] = index++;
            ((IUINT32 *) buffer)[1] = current;
            
            ikcp_send(kcp1, buffer, 8);
        }
        
        while (1) {
            hr = vnet->recv(1, buffer, 2000);
            if (hr < 0) break;
            ikcp_input(kcp2, buffer, hr);
        }
        
        while (1) {
            hr = vnet->recv(0, buffer, 2000);
            if (hr < 0) break;
            ikcp_input(kcp1, buffer, hr);
        }
        
        while (1) {
            hr = ikcp_recv(kcp2, buffer, 10);
            if (hr < 0) break;
            ikcp_send(kcp2, buffer, hr);
        }
        
        while (1) {
            hr = ikcp_recv(kcp1, buffer, 10);
            if (hr < 0) break;
            IUINT32 sn = *(IUINT32 *) (buffer + 0);
            IUINT32 ts = *(IUINT32 *) (buffer + 4);
            IUINT32 rtt = current - ts;

            if (sn != next) {
                printf("ERROR sn %d<->%d\n", (int) count, (int) next);
                return;
            }

            next++;
            sumrtt += rtt;
            count++;
            if (rtt > (IUINT32) maxrtt) maxrtt = rtt;
            printf("[RECV] mode=%d sn=%d rtt=%d\n", mode, (int) sn, (int) rtt);
        }
        if (next > 100) break;
    }

    ts1 = iclock() - ts1;

    ikcp_release(kcp1);
    ikcp_release(kcp2);

    const char *names[3] = {"default", "normal", "fast"};
    printf(" result (%dms):\n", (int) ts1);
    printf("press enter to next ...\n");
    char ch;
    scanf("%c", &ch);
}

int main() {
    test(2);   
    return 0;
}