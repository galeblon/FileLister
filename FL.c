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
#define DEFAULT_BLOCK_SIZE 1024
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

char** parseDirectory(const char* str, char* arr[], int* start, int* end);
void parseFile(const char* name, struct stat filestat, unsigned char d_type);
int checkMode(mode_t st_mode, int permission);
int compFiles(const void *a, const void* b);

struct FILE{
	char* name;
	char* full_path;
	struct stat filestat;
	unsigned char d_type;
};

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
		free(directories[i]);
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

	int numOfFiles = 0;
	int total = 0;
	while(pDirEnt != NULL){
		if(pDirEnt->d_name[0] != '.'){
			numOfFiles++;
		}
		pDirEnt = readdir(pDIR);
	}
	rewinddir(pDIR);
	struct FILE* arrOfFiles = malloc(numOfFiles*sizeof(struct FILE));
	int index = 0;
	pDirEnt = readdir(pDIR);
	while(pDirEnt != NULL){
		if(pDirEnt->d_name[0] != '.'){
			char* path_tmp = malloc(PATH_MAX);
			path_tmp[0] = '\0';
			strcat(path_tmp, path);
			strcat(path_tmp, "/");
			strcat(path_tmp, pDirEnt->d_name);
			stat(path_tmp, &(arrOfFiles[index].filestat));
			arrOfFiles[index].name = malloc(strlen(pDirEnt->d_name)+1);
			strcpy(arrOfFiles[index].name, pDirEnt->d_name);
			arrOfFiles[index].full_path = path_tmp;
			arrOfFiles[index].d_type = pDirEnt->d_type;
			total += arrOfFiles[index].filestat.st_blocks*(arrOfFiles[index].filestat.st_blksize/DEFAULT_BLOCK_SIZE);
			index++;
		}
		pDirEnt = readdir(pDIR);
	}
	closedir(pDIR);
	printf("total %d\n", total);
	qsort(arrOfFiles, numOfFiles, sizeof(struct FILE), compFiles);
	index = 0;
	while(index < numOfFiles){
		parseFile(arrOfFiles[index].name, arrOfFiles[index].filestat, arrOfFiles[index].d_type);
		if(arrOfFiles[index].d_type == DT_DIR){
			if(*start == *end){
				arr = realloc(arr, 2*(*end)*sizeof(const char*));
				*end = (*end)*2;
				if(arr == NULL){
					fprintf(stderr, "%s %d : realloc() failed (%s)\n", __FILE__, __LINE__, strerror(errno));
					exit(-1);
					return arr;
				}
			}
			arr[*start] = arrOfFiles[index].full_path;
			*start = *start + 1;
		}
		free(arrOfFiles[index].name);
		index++;
	}
	free(arrOfFiles);
	return arr;
}

void parseFile(const char* name, struct stat filestat, unsigned char d_type){
	struct group *grp;
	grp = getgrgid(filestat.st_gid);
	struct passwd *pwd;
	pwd = getpwuid(filestat.st_uid);
	if(grp == NULL || pwd == NULL)
		return;
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
	if(checkMode(filestat.st_mode, S_IRUSR))
		printf("r");
	else
		printf("-");
	if(checkMode(filestat.st_mode, S_IWUSR))
		printf("w");
	else
		printf("-");
	if(checkMode(filestat.st_mode, S_IXUSR))
		printf("x");
	else
		printf("-");
	if(checkMode(filestat.st_mode, S_IRGRP))
		printf("r");
	else
		printf("-");
	if(checkMode(filestat.st_mode, S_IWGRP))
		printf("w");
	else
		printf("-");
	if(checkMode(filestat.st_mode, S_IXGRP))
		printf("x");
	else
		printf("-");
	if(checkMode(filestat.st_mode, S_IROTH))
		printf("r");
	else
		printf("-");
	if(checkMode(filestat.st_mode, S_IWOTH))
		printf("w");
	else
		printf("-");
	if(checkMode(filestat.st_mode, S_IXOTH))
		printf("x");
	else
		printf("-");

	char* date;
	date = ctime(&(filestat.st_mtime));
	date[16] = '\0';
	printf(" %lu %s %s %8ld %s ", filestat.st_nlink, pwd->pw_name, grp->gr_name, filestat.st_size, date+4);
	if(strstr(name, ".png") != NULL || strstr(name, ".jpg")
				|| strstr(name, ".gif") || strstr(name, ".jpeg") || strstr(name, ".bmp"))
			printf(BOLDMAGENTA);
	if(checkMode(filestat.st_mode, S_IXUSR))
		printf(BOLDGREEN);
	if(d_type == DT_CHR)
			printf(BOLDYELLOW);
	if(d_type == DT_DIR)
			printf(BOLDBLUE);
	printf("%s", name);
	printf(RESET "\n");
}

int checkMode(mode_t st_mode, int permission){
	if(st_mode & permission)
		return 1;
	return 0;
}

int compFiles(const void *a, const void* b){
	struct FILE first = *((struct FILE*)a);
	struct FILE second = *((struct FILE*)b);
	return strcmp(first.name, second.name);
}

