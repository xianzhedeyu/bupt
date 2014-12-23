#include "hash.h"

void expand(List &list) {
    int i = 1;

    list.length += 10;
    ASNode* newlist = (ASNode*)malloc(list.length * sizeof(ASNode));
    for(; i <= list.length; i++) {
        strcpy(newlist[i].ip, " ");
    }
    double minValue = list.nodes[list.min].weight;
    ASNode* oldlist = list.nodes;
    list.nodes = newlist;
    for(i = 1; i<= list.length - 10; i++) {
        if(strcmp(oldlist[i].ip, " ") != 0) 
            insert(list, oldlist[i]);
    }
    free(oldlist);
}

void insert(List &list, ASNode node) {
    struct in_addr addr;
    inet_aton(node.ip, &addr);
    if((double)list.number / (double)list.length >= 0.75)
	expand(list);
 
    unsigned int num = addr.s_addr;
    unsigned int k = num % list.length;
    while(strcmp(list.nodes[k].ip, " ") != 0) {
        k++;
    }
    strcpy(list.nodes[k].ip, node.ip);
    list.nodes[k].load = node.load;
    list.nodes[k].memfree = node.memfree;
    list.nodes[k].weight = node.weight;
    list.nodes[k].port = node.port;
    list.number++;
    if(list.nodes[k].weight < list.nodes[list.min].weight) {
        list.min = k;
    }
}
int find(List list, const char* ip) {
    struct in_addr addr;
    inet_aton(ip, &addr);
    unsigned int f = addr.s_addr;
    unsigned int l = f % 10;
    while(strcmp(list.nodes[l].ip, ip) != 0) {
        if(l < list.length)
            l++;
        else
            l = 0;
    }
    return l;
}
void update(List &list, ASNode node, const char* ip) {
    int index = find(list, ip);
    strcpy(list.nodes[index].ip, node.ip);
    list.nodes[index].load = node.load;
    list.nodes[index].memfree = node.memfree;
    list.nodes[index].weight = node.weight;
    list.nodes[index].port = node.port;

    if(list.nodes[index].weight < list.nodes[list.min].weight) {
        list.min = index;
    }
}


