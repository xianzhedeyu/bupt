#include <stdio.h>
#include <string.h>

int MIN = 10;
typedef struct ASNode {
    double load;
    double memfree;
    double weight;
    char ip[40];
    int port;
}ASNode;

typedef struct List {
    ASNode* nodes;
    int length;
}List;

void HeapAdjust(List &list, int s, int m) {
    ASNode rc = list.nodes[s];
    int j = 0;
    for(j = 2 * s; j <= m; j *= 2) {
        if(j < m && list.nodes[j].weight < list.nodes[j + 1].weight)
            j++;
        if(rc.weight >= list.nodes[j].weight) 
            break;
        list.nodes[s].load = list.nodes[j].load;
        list.nodes[s].memfree = list.nodes[j].memfree;
        list.nodes[s].weight = list.nodes[j].weight;
        strcpy(list.nodes[s].ip, list.nodes[j].ip);
        list.nodes[s].port = list.nodes[j].port;
        s = j;
    }
    list.nodes[s].load = rc.load;
    list.nodes[s].memfree = rc.memfree;
    list.nodes[s].weight = rc.weight;
    strcpy(list.nodes[s].ip, rc.ip);
    list.nodes[s].port = rc.port;
}

void HeapSort(List &list) {
    int i = 0;
    for(i = list.length / 2; i > 0; i--) 
        HeapAdjust(list, i, list.length);
    for(i = list.length; i > 1; i--) {
        list.nodes[1].load = list.nodes[i].load;
        list.nodes[1].memfree = list.nodes[i].memfree;
        list.nodes[1].weight = list.nodes[i].weight;
        strcpy(list.nodes[1].ip, list.nodes[i].ip);
        list.nodes[1].port = list.nodes[i].port;
        HeapAdjust(list, 1, i - 1);
    }
}
