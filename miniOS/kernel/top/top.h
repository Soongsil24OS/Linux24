/*  ***********************Include source**************************/
// include 이유와 사용부분, 이름을 꼭 ! 기입해주세요.
#include <stdio.h>
#include <string.h> // str을 읽어오기 위해 include 2행 - 지수
#include <math.h> // isnan 사용하기 위해 include - 3행, 민석
#include <ctype.h> // isdigit 사용하기 위해 include - 3행, 민석
#include <curses.h> // mvprintw 사용하기 위해 include - 3행, 민석
#include <unistd.h> // hertz 구하기 위해 include - 3행, 민석
#include <dirent.h> // DIR 사용해 /proc 내부 탐색 - get_procDIR, 민석
#include <sys/stat.h> // User name 읽어오기 위해 사용

#define CPUTicks 8
#define BUFFER_SIZE 1024

unsigned long uptime;			//os 부팅 후 지난 시간이 저장될 변수 - 민석
unsigned long beforeUptime = 0;	//직전 부팅 이후 지난 시각 - 민석
long double beforeTicks[CPUTicks] = {0, };	//이전의 cpu ticks 저장하기 위한 배열 - 민석
unsigned long memTotal;			//전체 물리 메모리 크기
unsigned int hertz;	 			//os의 hertz값이 저장된 변수 - 민석 [구현 필요]

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
#define TASK_ROW 1		//process state를 출력하고자하는 행 - 지수

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


#define PROCESS_MAX 4096 //process 최대 크기 정의 - 지수

/********************* 길이 설정************************/ //2행, erase 구조체에서 사용
#define PATH_LEN 1024
#define UNAME_LEN 32
#define TTY_LEN 32
#define TOKEN_LEN 32
#define STAT_LEN 8
#define TIME_LEN 16
#define CMD_LEN 1024

/***************2행 /proc/pid/stat에서의 idx*************/
#define STAT_PID_IDX 0
#define STAT_TPGID_IDX 7
#define STAT_NICE_IDX 18
#define STAT_N_THREAD_IDX 19

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

// column에 출력할 문자열
#define PID_STR "PID"
#define PR_STR "PR"
#define NI_STR "NI"
#define VSZ_STR "VSZ"
#define VIRT_STR "VIRT"
#define RSS_STR "RSS"
#define RES_STR "RES"
#define SHR_STR "SHR"
#define S_STR "S"
#define STAT_STR "STAT"
#define START_STR "START"
#define TTY_STR "TTY"
#define CPU_STR "%CPU"
#define MEM_STR "MEM"
#define TIME_STR "TIME"
#define TIME_P_STR "TIME+"
#define CMD_STR "CMD"
#define COMMAND_STR "COMMAND"

