/*  ***********************Include source**************************/
// include 이유와 사용부분, 이름을 꼭 ! 기입해주세요.
#include <stdio.h>
#include <string.h> // str을 읽어오기 위해 include 2행 - 지수
#include <stdlib.h>
#include <string.h>
#include <math.h> // isnan 사용하기 위해 include - 3행, 민석
#include <ctype.h> // isdigit 사용하기 위해 include - 3행, 민석
#include <curses.h> // mvprintw 사용하기 위해 include - 3행, 민석
#include <unistd.h> // hertz 구하기 위해 include - 3행, 민석
#include <dirent.h> // DIR 사용해 /proc 내부 탐색 - get_procDIR, 민석
#include <sys/stat.h> // User name 읽어오기 위해 사용
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <ncurses.h> // ncurses

#define CPUTicks 8
#define BUFFER_SIZE 1024
	//이전의 cpu ticks 저장하기 위한 배열 - 민석

/*  ***********************경로 모음**************************/
#define PROC "/proc"				// /proc 절대 경로
#define CPUSTAT "/proc/stat"		// /proc/stat 절대 경로
#define UPTIME "/proc/uptime" // 3행
#define MEMINFO "/proc/meminfo" // 4행
#define LOADAVG "/proc/loadavg"
#define PROCESS_MAX 4096
#define PID_MAX 32768				//pid 최대 갯수

/*  ***********************터미널 출력을 위한 행**************************/
#define TOP_ROW 0
#define TASK_ROW 1
#define MEM_ROW 2
#define CPU_ROW 3

/********************* 길이 설정************************/ //2행, erase 구조체에서 사용
#define PATH_LEN 1024
#define UNAME_LEN 32
#define TOKEN_LEN 32
#define STAT_LEN 8
#define TIME_LEN 16
#define CMD_LEN 1024
/*  ***********************프로세스 구조체**************************/
typedef struct {
    unsigned long pid;
    unsigned long uid;
    char user[UNAME_LEN];
    long double cpu;
    long double mem;
    unsigned long vsz;
    unsigned long rss;
    unsigned long shr;
    int priority;
    int nice;
    char stat[STAT_LEN];
    char start[TIME_LEN];
    char time[TIME_LEN];
    char cmd[CMD_LEN];
    char command[CMD_LEN];
}myProc;

/*  ***********************함수 정의**************************/
void add_proc_list(char path[1024], unsigned long cpuTimeTable[8192]);
extern unsigned long get_uptime(void);
extern unsigned long kib_to_kb(unsigned long kib);
extern unsigned long get_mem_total(void);
extern void get_procPath(unsigned long cpuTimeTable[8192]);
extern void erase_proc_list(void);
extern void erase_proc(myProc* proc);

/******************6행 column index*******************/
#define COLUMN_CNT 11 //출력할 column 최대 갯수
#define PID_IDX 0
#define PR_IDX 1
#define NI_IDX 2
#define VIRT_IDX 3
#define RES_IDX 4
#define SHR_IDX 5
#define S_IDX 6
#define CPU_IDX 7
#define MEM_IDX 8
#define TIME_P_IDX 9
#define COMMAND_IDX 10
#define PID_STR "PID"
#define PR_STR "PR"
#define NI_STR "NI"
#define VIRT_STR "VIRT"
#define RES_STR "RES"
#define SHR_STR "SHR"
#define S_STR "S"
#define CPU_STR "%CPU"
#define MEM_STR "MEM"
#define TIME_P_STR "TIME+"
#define COMMAND_STR "COMMAND"
#define COLUMN_ROW 6			//column 출력할 행
#define TAB_WIDTH 8
