#include "hash.h"

void expand(List &list) {
    //int i = 1;

    //list.length += 10;
    //ASNode* newlist = (ASNode*)malloc(list.length * sizeof(ASNode));
    //for(; i <= list.length; i++) {
    //    strcpy(newlist[i].ip, " ");
    //}
    //double minValue = list.nodes[list.min].weight;
    //ASNode* oldlist = list.nodes;
    //list.nodes = newlist;
    //for(i = 1; i<= list.length - 10; i++) {
    //    if(strcmp(oldlist[i].ip, " ") != 0) 
    //        insert(list, oldlist[i]);
    //}
    //free(oldlist);
}

//void insert(List &list, ASNode node) {
void insert(ASNode node) {
    struct in_addr addr;
    inet_aton(node.ip, &addr);
    int fd = shm_open("asinfo", O_RDWR, 0);
    struct stat stat;
    fstat(fd, &stat);
    ASNode* list = (ASNode *)mmap(NULL, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    int fd_length = shm_open("length", O_RDWR, 0);
    int* length = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_length, 0);
    int fd_min = shm_open("min", O_RDWR, 0);
    int* min = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_min, 0);
    int fd_number = shm_open("number", O_RDWR, 0);
    int* number = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_number, 0);
    
    if((double)*number / (double)*length >= 0.75)
     {
         int new_length = (*length + 10) * sizeof(ASNode);
         int new_fd = shm_open("asinfo", O_RDWR | O_CREAT, 0644);
         list = (ASNode *)mmap(NULL, new_length, PROT_READ | PROT_WRITE, MAP_SHARED, new_fd, 0);
         *length += 10;
         int i = 0;
         for(; i < *length ; i++) {
             strcpy(list[i].ip, " ");
         }
         *min = -1;
         *number = 0;
     }
    unsigned int num = addr.s_addr;
    unsigned int k = num % *length;
    printf("%d\n", k);
    while(strcmp(list[k].ip, " ") != 0) {
        k++;
    }
    printf("%d\n", k);
    list[k].cpuload = node.cpuload;
    list[k].memload = node.memload;
    list[k].weight = node.weight;
    strcpy(list[k].ip, node.ip);
    *number ++;
    if(*min == -1 || list[k].weight < list[*min].weight)
        *min = k;

    //if((double)list.number / (double)list.length >= 0.75)
	//    expand(list);
 
    //unsigned int num = addr.s_addr;
    //unsigned int k = num % list.length;
    //while(strcmp(list.nodes[k].ip, " ") != 0) {
    //    k++;
    //}
    //strcpy(list.nodes[k].ip, node.ip);
    //list.nodes[k].cpuload = node.cpuload;
    //list.nodes[k].memload = node.memload;
    //list.nodes[k].weight = node.weight;
    //list.number++;
    //if(list.nodes[k].weight < list.nodes[list.min].weight) {
    //    list.min = k;
    //}
}
//int find(List list, const char* ip) {
int find(ASNode* list, const char* ip, int length) {
    struct in_addr addr;
    inet_aton(ip, &addr);
    unsigned int f = addr.s_addr;
    int l = f % 10;
    //while(strcmp(list.nodes[l].ip, ip) != 0) {
    while(strcmp(list[l].ip, ip) != 0) {
        //if(l < list.length)
        if(l < length)
            l++;
        else{
            l = -1;
            break;
        }
    }
    return l;
}
//void update(List &list, ASNode node, const char* ip) {
void update(ASNode node, const char* ip) {
    int fd = shm_open("asinfo", O_RDWR, 0);
    struct stat stat;
    fstat(fd, &stat);
    ASNode* list = (ASNode *)mmap(NULL, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    int fd_length = shm_open("length", O_RDWR, 0);
    int* length = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_length, 0);
    int fd_min = shm_open("min", O_RDWR, 0);
    int* min = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_min, 0);

    int index = find(list, ip, *length);
    strcpy(list[index].ip, node.ip);
    list[index].cpuload = node.cpuload;
    list[index].memload = node.memload;
    list[index].weight = node.weight;

    if(*min == -1 || list[index].weight < list[*min].weight) {
        *min = index;
    }

    //int index = find(list, ip);
    //strcpy(list.nodes[index].ip, node.ip);
    //list.nodes[index].cpuload = node.cpuload;
    //list.nodes[index].memload = node.memload;
    //list.nodes[index].weight = node.weight;

    //if(list.nodes[index].weight < list.nodes[list.min].weight) {
    //    list.min = index;
    //}
}
//void init(List &list) 
void init()
{
    //list.length = 10;
    //list.number = 0;
    //list.min = 0;
    //list.nodes = (ASNode *)malloc(10 * sizeof(ASNode));
    //int i = 0;
    //for(; i < 10; i++) 
    //{
    //    list.nodes[i].cpuload = 0;
    //    list.nodes[i].memload = 0;
    //    list.nodes[i].weight = 0;
    //    strcpy(list.nodes[i].ip, " ");
    //}
    char *name = "asinfo";
    int length = sizeof(ASNode) * 10;
    int number = 0;
    char *lname = "length";
    char *nname = "number";
    char *mname = "min";

    int fd = shm_open(name, O_RDWR | O_CREAT, 0644);
    ftruncate(fd, length);
    ASNode *ptr = (ASNode *)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    int fd_length = shm_open(lname, O_RDWR | O_CREAT, 0644); 
    ftruncate(fd_length, sizeof(int));
    int *len = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_length, 0);
    close(fd_length);
    int fd_number = shm_open(nname, O_RDWR | O_CREAT, 0644);
    ftruncate(fd_number, sizeof(int));
    int *num = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_number, 0);
    close(fd_number);
    int fd_min = shm_open(mname, O_RDWR | O_CREAT, 0644);
    ftruncate(fd_min, sizeof(int));
    int *min = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_min, 0);
    close(fd_min);

    *len = 10;
    *num = 0;
    *min = -1;
    int i;
    for(i = 0 ; i < *len; i++) {
        strcpy(ptr[i].ip, " ");
    }
}
