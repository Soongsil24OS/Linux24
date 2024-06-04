/*  ***********************Include source**************************/
// include 이유와 사용부분, 이름을 꼭 ! 기입해주세요.
#include <stdio.h>
#include <string.h>
#include <math.h> // isnan 사용하기 위해 include - 3행, 민석
#include <ctype.h> // isdigit 사용하기 위해 include - 3행, 민석
#include <curses.h> // mvprintw 사용하기 위해 include - 3행, 민석
#include <unistd.h> // hertz 구하기 위해 include - 3행, 민석
#include <dirent.h> // DIR 사용해 /proc 내부 탐색 - get_procDIR, 민석
#include <sys/stat.h> // User name 읽어오기 위해 사용

#define CPUTicks 8
#define BUFFER_SIZE 1024

unsigned long uptime;			//os 부팅 후 지난 시간이 저장될 변수 - 민석 -시운
unsigned long beforeUptime = 0;	//직전 부팅 이후 지난 시각 - 민석
long double beforeTicks[CPUTicks] = {0, };	//이전의 cpu ticks 저장하기 위한 배열 - 민석
unsigned long memTotal;			//전체 물리 메모리 크기 -시운
unsigned int hertz;	 			//os의 hertz값이 저장된 변수 - 민석 [구현 필요] -시운

/*  ***********************경로 모음**************************/
#define CPUSTAT "/proc/stat" // 3행
#define UPTIME "/proc/uptime" // 3행
#define MEMINFO "/proc/meminfo" // 4행

/*  ***********************터미널 출력을 위한 행**************************/
#define a 0
#define MEM_row 1
#define CPU_row 2
#define b 3
#define c 4
/*  ***********************프로세스 구조체**************************/
typedef struct{
	unsigned long pid;
	unsigned long uid;			//USER 구하기 위한 uid
	char user[32];		//user명
	long double cpu;			//cpu 사용률
	long double mem;			//메모리 사용률
	unsigned long vsz;			//가상 메모리 사용량
	unsigned long rss;			//실제 메모리 사용량
	unsigned long shr;			//공유 메모리 사용량
	int priority;				//우선순위
	int nice;					//nice 값
	char tty[32];			//터미널
	char stat[16];		//상태
	char start[32];		//프로세스 시작 시각
	char time[32];		//총 cpu 사용 시간
	char cmd[1024];			//option 없을 경우에만 출력되는 command (short)
	char command[1024];		//option 있을 경우에 출력되는 command (long)
	
}myProc;








time_t before;
time_t now;

unsigned long uptime;			//os 부팅 후 지난 시간 -1행
unsigned long beforeUptime = 0;	//이전 실행 시의 os 부팅 후 지난 시각
unsigned long memTotal;			//전체 물리 메모리 크기
unsigned int hertz;	 			//os의 hertz값 얻기(초당 context switching 횟수)

pid_t myPid;					//자기 자신의 pid
uid_t myUid;					//자기 자신의 uid
char myPath[PATH_LEN];			//자기 자신의 path
char myTTY[TTY_LEN];			//자기 자신의 tty

int ch;
int row, col;
int procCnt = 0;				//현재까지 완성한 myProc 갯수


#include <time.h> // -tm 구조체 사용/localtime()
#include <ctype.h>
#include <utmp.h> //setutent(), getutunt()
#include <ncurses.h>

#define BUFFER_SIZE 1024 // -1행
#define LOADAVG "/proc/loadavg"		// /proc/loadavg 절대 경로 -1행



// procList 내용 지우는 함수
void erase_proc_list(void);

//	/proc 디렉터리 탐색하는 함수
void search_proc(bool isPPS, bool aOption, bool uOption, bool xOption, unsigned long cpuTimeTable[]);

//path에 대한 tty 얻는 함수
void getTTY(char path[PATH_LEN], char tty[TTY_LEN]);

// /proc/uptime에서 OS 부팅 후 지난 시간 얻는 함수 -1행
unsigned long get_uptime(void);

// /proc/meminfo에서 전체 물리 메모리 크기 얻는 함수
unsigned long get_mem_total(void);





//procList를 cpu 순으로 sorting해 sorted 배열을 완성하는 함수
void sort_by_cpu(void)
{
	for(int i = 0; i < procCnt; i++)		//포인터 복사
		sorted[i] = procList + i;
	for(int i = procCnt - 1; i > 0; i--){
		for(int j = 0; j < i; j++){
			if(isGreater(sorted[j], sorted[j+1])){
				myProc *tmp = sorted[j];
				sorted[j] = sorted[j+1];
				sorted[j+1] = tmp;
			}
		}
	}
	return;
}

// procList 내용 지우는 함수
void erase_proc_list(void)
{
	for(int i = 0; i < procCnt; i++)
		erase_proc(procList + i);
	procCnt = 0;
	return;
}

//	/proc 디렉터리 탐색하는 함수
void search_proc(bool isPPS, bool aOption, bool uOption, bool xOption, unsigned long cpuTimeTable[PID_MAX])
{
	uptime = get_uptime();
	DIR *dirp;
	if((dirp = opendir(PROC)) == NULL){	// /proc 디렉터리 open
		fprintf(stderr, "dirp error for %s\n", PROC);
		exit(1);
	}
	struct dirent *dentry;
	while((dentry = readdir(dirp)) != NULL){	// /proc 디렉터리 내 하위 파일들 탐색 시작

		char path[PATH_LEN];			//디렉터리의 절대 경로 저장
		memset(path, '\0', PATH_LEN);
		strcpy(path, PROC);
		strcat(path, "/");
		strcat(path, dentry->d_name);

		struct stat statbuf;
		if(stat(path, &statbuf) < 0){	//디렉터리의 stat 획득
			fprintf(stderr, "stat error for %s\n", path);
			exit(1);
		}

		if(!S_ISDIR(statbuf.st_mode))	//디렉터리가 아닐 경우 skip
			continue;
		
		int len = strlen(dentry->d_name);
		bool isPid = true;
		for(int i = 0; i < len; i++){	//디렉터리가 PID인지 찾기
			if(!isdigit(dentry->d_name[i])){	//디렉터리명 중 숫자 아닌 문자가 있을 경우
				isPid = false;
				break;
			}
		}
		if(!isPid)				//PID 디렉터리가 아닌 경우 skip
			continue;
		if(isPPS && !aOption)			//aOption이 없을 경우 자기 자신의 process만 보여줌
			if(statbuf.st_uid != myUid)		//uid가 자기 자신과 다를 경우 skip
				continue;
		if(isPPS && !xOption){			//xOption이 없을 경우 nonTerminal process는 생략함
			char tty[TTY_LEN];
			memset(tty, '\0', TTY_LEN);
			getTTY(path, tty);		//TTY 획득
			if(!strlen(tty) || !strcmp(tty, "?"))	//nonTerminal일 경우
				continue;
		}
		if(isPPS && !aOption && !uOption && !xOption){	//모든 Option 없을 경우
			char tty[TTY_LEN];
			memset(tty, '\0', TTY_LEN);
			getTTY(path, tty);		//TTY 획득
			if(strcmp(tty, myTTY))	//자기 자신과 tty 다를 경우
				continue;
		}
		add_proc_list(path, isPPS, aOption, uOption, xOption, cpuTimeTable);	//PID 디렉터리인 경우 procList에 추가
	}
	closedir(dirp);
	return;
}

//path에 대한 tty 얻는 함수
void getTTY(char path[PATH_LEN], char tty[TTY_LEN])
{
	char fdZeroPath[PATH_LEN];			//0번 fd에 대한 절대 경로
	memset(tty, '\0', TTY_LEN);
	memset(fdZeroPath, '\0', TTY_LEN);
	strcpy(fdZeroPath, path);
	strcat(fdZeroPath, FD__ZERO);

	if(access(fdZeroPath, F_OK) < 0){	//fd 0이 없을 경우

		char statPath[PATH_LEN];		// /proc/pid/stat에 대한 절대 경로
		memset(statPath, '\0', PATH_LEN);
		strcpy(statPath, path);
		strcat(statPath, STAT);

		FILE *statFp;
		if((statFp = fopen(statPath, "r")) == NULL){	// /proc/pid/stat open
			fprintf(stderr, "fopen error %s %s\n", strerror(errno), statPath);
			sleep(1);
			return;
		}

		char buf[BUFFER_SIZE];
		for(int i = 0; i <= STAT_TTY_NR_IDX; i++){		// 7행까지 read해 TTY_NR 획득
			memset(buf, '\0', BUFFER_SIZE);
			fscanf(statFp, "%s", buf);
		}
		fclose(statFp);

		int ttyNr = atoi(buf);		//ttyNr 정수 값으로 저장

		DIR *dp;
		struct dirent *dentry;
		if((dp = opendir(DEV)) == NULL){		// 터미널 찾기 위해 /dev 디렉터리 open
			fprintf(stderr, "opendir error for %s\n", DEV);
			exit(1);
		}
		char nowPath[PATH_LEN];

		while((dentry = readdir(dp)) != NULL){	// /dev 디렉터리 탐색
			memset(nowPath, '\0', PATH_LEN);	// 현재 탐색 중인 파일 절대 경로
			strcpy(nowPath, DEV);
			strcat(nowPath, "/");
			strcat(nowPath, dentry->d_name);

			struct stat statbuf;
			if(stat(nowPath, &statbuf) < 0){	// stat 획득
				fprintf(stderr, "stat error for %s\n", nowPath);
				exit(1);
			}
			if(!S_ISCHR(statbuf.st_mode))		//문자 디바이스 파일이 아닌 경우 skip
				continue;
			else if(statbuf.st_rdev == ttyNr){	//문자 디바이스 파일의 디바이스 ID가 ttyNr과 같은 경우
				strcpy(tty, dentry->d_name);	//tty에 현재 파일명 복사
				break;
			}
		}
		closedir(dp);

		if(!strlen(tty))					// /dev에서도 찾지 못한 경우
			strcpy(tty, "?");				//nonTerminal
	}
	else{
		char symLinkName[FNAME_LEN];
		memset(symLinkName, '\0', FNAME_LEN);
		if(readlink(fdZeroPath, symLinkName, FNAME_LEN) < 0){
			fprintf(stderr, "readlink error for %s\n", fdZeroPath);
			exit(1);
		}
		if(!strcmp(symLinkName, DEVNULL))		//symbolic link로 가리키는 파일이 /dev/null일 경우
			strcpy(tty, "?");					//nonTerminal
		else
			sscanf(symLinkName, "/dev/%s", tty);	//그 외의 경우 tty 획득

	}
	return;
}

// /proc/uptime에서 OS 부팅 후 지난 시간 얻는 함수
unsigned long get_uptime(void)
{
    FILE *fp;
    char buf[BUFFER_SIZE];
    long double time;

	memset(buf, '\0', BUFFER_SIZE);

    if ((fp = fopen(UPTIME, "r")) == NULL){	// /proc/uptime open
		fprintf(stderr, "fopen error for %s\n", UPTIME);
        exit(1);
    }
    fgets(buf, BUFFER_SIZE, fp);
    sscanf(buf, "%Lf", &time);	// /proc/uptime의 첫번째 double 읽기
    fclose(fp);

    return (unsigned long)time;
}

// /proc/meminfo에서 전체 물리 메모리 크기 얻는 함수
unsigned long get_mem_total(void)
{
    FILE *fp;
    char buf[BUFFER_SIZE];
	unsigned long memTotal;

    if ((fp = fopen(MEMINFO, "r")) == NULL){	// /proc/meminfo open
		fprintf(stderr, "fopen error for %s\n", MEMINFO);
        exit(1);
    }
	int i = 0;
	while(i < MEMINFO_MEM_TOTAL_ROW){	// memTotal read
		memset(buf, '\0', BUFFER_SIZE);
		fgets(buf, BUFFER_SIZE, fp);
		i++;
	}
	char *ptr = buf;
	while(!isdigit(*ptr)) ptr++;
    sscanf(ptr, "%lu", &memTotal);	// /proc/meminfo의 1행에서 memTotal read
    fclose(fp);

	memTotal = kib_to_kb(memTotal);	//Kib 단위를 Kb로 변환

    return memTotal;
}


