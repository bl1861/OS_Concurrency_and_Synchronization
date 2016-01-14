#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
	FILE *fp;
	char *filename = "fileparse.c";
	
	fp = fopen(filename, "r");

	if(fp != NULL){
		size_t len = 1000;
		char *line = (char*)malloc(sizeof(char) * len);
		int read;
		char buf[10000] = "";
		while((read = getline(&line, &len, fp)) != -1){
			strcat(buf, line);
		}
		printf("%s", buf);
		printf("=========================\nlength = %d\n", strlen(buf))	;
	}
	else{
		printf("open fail: %s\n", filename);
	}
}
