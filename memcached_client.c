/* http://l3net.wordpress.com/2012/12/09/a-simple-telnet-client/ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>
 
#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe
#define CMD 0xff
#define CMD_ECHO 1
#define CMD_WINDOW_SIZE 31


typedef struct _item {
    char *key;
    int size;
    int count;
} item;


static item pack_item(const char *key, int len, int size, int count){
    item it;
    it.count = count;
    it.size = size;
    it.key = malloc(len+1);
    strcpy(it.key, key);
    it.key[len] = '\0';
    return it;
}

 
#define BUFLEN 20
#define INIT_SIZE 10
#define ITEM_PACK_SIZE 1024


int main(int argc , char *argv[]) {
    int sock;
    struct sockaddr_in server;
    unsigned char buf[BUFLEN + 1];
    char read_buf[BUFLEN];
    char send_buf[ITEM_PACK_SIZE];
    int size, count;
    FILE *f;
    char *key;
    char *word;
    item *items;
    int item_count, item_limit = INIT_SIZE;
    int len;
    int i;
    char *prefix;
    char *value;
   
    if (argc != 2) {
        printf("Usage: %s configfile\n", argv[0]);
        return 1;
    }
    int port = 11211;
    

    items = (item *)malloc(item_limit*sizeof(item));
    item_count = 0;
    f = fopen("config", "r");
    while (fgets(read_buf, BUFLEN, f) != NULL){
        key = strtok(read_buf, ",");
        word = strtok(NULL, ",");
        size = atoi(word);
        word = strtok(NULL, ",");
        count = atoi(word);
        if(item_count < item_limit){            
            items[item_count++] = pack_item(key, strlen(key), size, count);
        } else{
            int newlimit = item_limit * 2;
            item *new_items = realloc(items, newlimit*sizeof(item));
            if (new_items == NULL){
                perror("Could not realloc memory. Error");
                return 1;
            }
            items = new_items;
            item_limit = newlimit;            
            items[item_count++] = pack_item(key, strlen(key), size, count);           
        }
    }
    fclose(f);
    for (i=0; i<item_count; i++){
        item it = items[i];
        printf("key = %s, keylen = %lu, sz = %d, cnt = %d \n", it.key, 
               strlen(it.key), it.size, it.count);
    }
    
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        perror("Could not create socket. Error");
        return 1;
    }
 
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }
    puts("Connected...\n");
 
    int global_counter = 0;  
    for (i=0; i<item_count; i++){
        item it = items[i];
        prefix = it.key;
        size = it.size;
        count = it.count;
        value = (char *)malloc(size*sizeof(char)+1);
        memset(value, 'x', size);
        value[size] = '\0';
        
        for(int j=0; j < count; j++){
            if(sprintf(send_buf, "set %s%d 0 0 %d noreply\r\n%s\r\n", 
                prefix, global_counter, size, value) > 0){
                if (send(sock, send_buf, strlen(send_buf), 0) < 0)
                    exit(1);
                global_counter++;     
            }
        }
        free(value);
    }
    
    for (i=0; i<item_count; i++){
        item it = items[i];
        free(it.key); 
    }
    free(items);
    close(sock);
    return 0;
}