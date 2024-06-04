/*  ***********************Include source**************************/
// include 이유와 사용부분, 이름을 꼭 ! 기입해주세요.
#include <stdio.h>
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
#include <sys/sysinfo.h>

#define CPUTicks 8
#define BUFFER_SIZE 1024

unsigned long uptime;			//os 부팅 후 지난 시간이 저장될 변수 - 민석
unsigned long beforeUptime = 0;	//직전 부팅 이후 지난 시각 - 민석
long double beforeTicks[CPUTicks] = {0, };	//이전의 cpu ticks 저장하기 위한 배열 - 민석
unsigned long memTotal;			//전체 물리 메모리 크기
unsigned int hertz;	 			//os의 hertz값이 저장된 변수 - 민석 [구현 필요]

/*  ***********************경로 모음**************************/
#define PROC "/proc"				// /proc 절대 경로
#define CPUSTAT "/proc/stat"		// /proc/stat 절대 경로
#define STATUS "/status"	
#define UPTIME "/proc/uptime" // 3행
#define MEMINFO "/proc/meminfo" // 4행

#define PATH_LEN 1024
#define UNAME_LEN 32
#define TOKEN_LEN 32
#define MAX_TOKEN 22				// /proc/pid/stat에서 읽어들일 token 갯수
#define STAT_LEN 8
#define PROCESS_MAX 4096
#define STAT_STATE_IDX 2
#define PID_MAX 32768				//pid 최대 갯수

/*  ***********************터미널 출력을 위한 행**************************/
#define a 0
#define MEM_row 1
#define CPU_row 2
#define b 3
#define c 4

#define COLUMN_CNT 12 //출력할 column 최대 갯수
#define PID_IDX 0
#define USER_IDX 1
#define PR_IDX 2
#define NI_IDX 3
#define VIRT_IDX 4
#define RES_IDX 5
#define SHR_IDX 6
#define S_IDX 7
#define CPU_IDX 8
#define MEM_IDX 9
#define TIME_P_IDX 10
#define COMMAND_IDX 11
#define COLUMN_ROW 6			//column 출력할 행

/*  *************************************************/
#define MAX_PROC 1024
#define UNAME_LEN 32
#define TTY_LEN 16
#define STAT_LEN 16
#define TIME_LEN 16
#define CMD_LEN 256

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
    char tty[TTY_LEN];
    char stat[STAT_LEN];
    char start[TIME_LEN];
    char time[TIME_LEN];
    char cmd[CMD_LEN];
    char command[CMD_LEN];
} myProc;