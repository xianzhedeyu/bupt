#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct ASNode {
    double cpuload;
    double memload;
    double weight;
    char ip[40];
}ASNode;

typedef struct List {
    ASNode* nodes;
    int length;
    int number;
    int min;
}List;

int find(List list, const char* ip);
//void insert(List &list, ASNode node);
void insert(ASNode node);
//void update(List &list, ASNode node, const char* ip);
void update(ASNode node, const char* ip);
//void init(List &list);
void init();
void init_set();
