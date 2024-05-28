#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define PROC "/proc"				// /proc 절대 경로
#define CPUSTAT "/proc/stat"		// /proc/stat 절대 경로
#define STATUS "/status"			// /proc/pid에서의 status 경로

#define PATH_LEN 1024
#define UNAME_LEN 32
#define TOKEN_LEN 32
#define MAX_TOKEN 22				// /proc/pid/stat에서 읽어들일 token 갯수
#define STAT_LEN 8
#define PROCESS_MAX 4096
#define STAT_STATE_IDX 2

#define PID_MAX 32768				//pid 최대 갯수

// /proc/pid/stat에서의 idx
#define STAT_PID_IDX 0
#define STAT_TPGID_IDX 7
#define STAT_NICE_IDX 18
#define STAT_N_THREAD_IDX 19

//process를 추상화 한 myProc 구조체
typedef struct{
	unsigned long pid;              // 프로세스 ID
    char user[UNAME_LEN];           // 사용자 이름
    char stat[STAT_LEN];            // 프로세스 상태
	int nice;					//nice 값
}myProc;


// pid 디렉터리 내의 파일들을 이용해 myProc 완성하는 함수
void add_proc_list(char path[PATH_LEN], bool isPPS, bool aOption, bool uOption, bool xOption, unsigned long cpuTimeTable[]);