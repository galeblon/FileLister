#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <linux/limits.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

char** parseDirectory(const char* str, char* arr[], int* start, int* end);
void parseFile(const char* name, struct stat filestat);

int main (int argc, char *argv[]){
   char** directories = malloc(sizeof(const char*)*20);
   int start = 0;
   int end = 20;
   if(argc > 1){
	   printf("\n%s:\n", argv[1]);
	   directories = parseDirectory(argv[1], directories, &start, &end);
   } else {
	   printf(".:\n");
	   directories = parseDirectory(".", directories, &start, &end);
   }
   int i=0;
   while(i<start){
	   printf("\n%s:\n", directories[i]);
	   directories = parseDirectory(directories[i], directories, &start, &end);
	   i++;
   }
   free(directories);
   return 0;
}

char** parseDirectory(const char* path, char* arr[], int* start, int* end){
	DIR *pDIR;
	struct dirent *pDirEnt;
	pDIR = opendir(path);
	if(pDIR == NULL){
		fprintf(stderr, "%s %d : opendir() failed (%s)\n", __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
	pDirEnt = readdir(pDIR);
	while(pDirEnt != NULL){
		if(pDirEnt->d_name[0] != '.'){
			struct stat fileStat;
			char* path_tmp = malloc(PATH_MAX);
			path_tmp[0] = '\0';
			strcat(path_tmp, path);
			strcat(path_tmp, "/");
			strcat(path_tmp, pDirEnt->d_name);
			stat(path_tmp, &fileStat);
			if(pDirEnt->d_type == DT_DIR){
				if(*start == *end){
					arr = realloc(arr, 2*(*end)*sizeof(const char*));
					*end = (*end)*2;
					if(arr == NULL){
						fprintf(stderr, "%s %d : realloc() failed (%s)\n", __FILE__, __LINE__, strerror(errno));
						exit(-1);
						return arr;
					}
				}
				arr[*start] = path_tmp;
				*start = *start + 1;
			}
			parseFile(pDirEnt->d_name, fileStat);
		}
		pDirEnt = readdir(pDIR);
	}
	closedir(pDIR);
	return arr;
}

void parseFile(const char* name, struct stat filestat){
	if(S_ISREG(filestat.st_mode))
		printf("-");
	else if(S_ISDIR(filestat.st_mode))
		printf("d");
	else if(S_ISCHR(filestat.st_mode))
		printf("c");
	else if(S_ISBLK(filestat.st_mode))
		printf("b");
	else if(S_ISFIFO(filestat.st_mode))
		printf("f");
	else if(S_ISLNK(filestat.st_mode))
		printf("l");
	else if(S_ISSOCK(filestat.st_mode))
		printf("s");
	struct group *grp;
	grp = getgrgid(filestat.st_gid);
	struct passwd *pwd;
	pwd = getpwuid(filestat.st_uid);
	char* date;
	date = ctime(&(filestat.st_mtime));
	date[16] = '\0';
	printf("rw-rw-r-- 1 %s %s %8ld %s %s\n", pwd->pw_name, grp->gr_name, filestat.st_size, date+4 ,name);
}





