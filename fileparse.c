#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>

#include "hash.h"

#define NUM_THREADS 25

int numIP;
hashtable_t *hashtable;
pthread_mutex_t mutexhash;

typedef struct thread_block{
    long tid;
    char filename[256];
} thread_block;


void *ReadFile(void *arg){
    
    thread_block* tb = (thread_block*) arg;
    size_t len = 1000;
    char *line = (char*)malloc(sizeof(char)*len);
    ssize_t read;
    
    FILE *file;
    
    file = fopen(tb->filename, "r");
    int line_total = 0;
    
    
    if(file != NULL){
        
        char IP[50];
        char *start;
        char *end;
        
        while((read = (getline(&line, &len, file))) != -1){
            start = line;
            end = strchr(line, ' ');
            strncpy(IP, line, end - start);
            IP[end - start] = '\0';
            
            /* enter critical section */
            pthread_mutex_lock(&mutexhash);
            if(ht_get(hashtable, IP) == NULL){
                ht_set(hashtable, IP, "1");
                numIP++;
            }
            
            /* leave critical section */
            pthread_mutex_unlock(&mutexhash);
            line_total++;
        }
        printf("tid = %-3ld file = %-28s line total = %d\n", tb->tid, tb->filename, line_total);
        fclose(file);
    }
    
    else{
        printf("open %s fail\n", tb->filename);
    }
    
    
    pthread_exit((NULL));
}



int main(int argc, char *argv[])
{
    DIR *dir;	//directory stream
    struct dirent *ent;	// directory entry structure
    thread_block* tb[NUM_THREADS]; // hold the data of thread
    
    pthread_t thread[NUM_THREADS];
    pthread_attr_t attr;
    int rc;
    long t = 0; //tid of thread
    void *status;
    
    
    // create the size of hashtable
    hashtable = ht_create(10000);
    
    // initialize and set thread detached attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutexhash, NULL);
    numIP = 0;
    
    
    // check the arguments
    
    if(argc < 2)
    {
        printf("Not enough arguments supplied\n");
        return -1;
    }
    
    if(argc > 2)
    {
        printf("Too many arguments supplied\n");
        return -1;
    }
    
    printf("Main: program started\n");
    printf("======================================================================\n");
    
    if ((dir = opendir (argv[1])) != NULL)
    {
        
        /*t ry to open the directory given by the argument */
        while ((ent = readdir (dir)) != NULL)
        {
            
            /* check the file whether is the .log file or not*/
            size_t slen = strlen(ent->d_name);
            if (strncmp(ent->d_name + slen - 4, ".log", 4) != 0) continue;
            
            
            /* Check if the list is a regular file */
            if(ent->d_type == DT_REG)
            {
                
                /* Create the absolute path of the filename */
                tb[t] = (thread_block*)malloc(sizeof(thread_block));
                snprintf(tb[t]->filename, sizeof tb[t]->filename, "./%s%s\0", argv[1], ent->d_name);
                tb[t]->tid = t;
                
                
                /* Create new thread to read file*/
                printf("create new thread, tid = %-2ld, file = %s\n", tb[t]->tid, tb[t]->filename);
                rc = pthread_create(&thread[t], &attr, ReadFile, (void*) tb[t]);

                if(rc){
                    printf("ERROR; return code from pthread_create is %d\n", rc);
                    exit(-1);
                }
                t++;
            }
        }
        
        pthread_attr_destroy(&attr);
        
        /* wait for threads dones*/
        for(long j = 0; j < t; j++){
            rc = pthread_join(thread[j], &status);
            if(rc){
                printf("ERROR\n");
            }
        }
        
        // Close the directory structure
        closedir (dir);
    }
    else
    {
        /* could not open directory */
        perror ("");
        return -1;
    }
    
    printf("======================================================================\n");
    printf("Total number of unique IP = %d\n", numIP);
    printf("Main: program completed\n");
    
    /* */
    pthread_mutex_destroy(&mutexhash);
    pthread_exit(NULL);

}
