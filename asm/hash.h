#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

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
    int number;
    int min;
}List;

int find(List list, const char* ip);
void insert(List &list, ASNode node);
void update(List &list, ASNode node, const char* ip);
