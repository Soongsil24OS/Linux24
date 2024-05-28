#include "ttopsource2.h"

#define TASK_ROW 1

myProc procList[PROCESS_MAX]; // 외부 정의 및 초기화
int procCnt = 0;               // 외부 정의 및 초기화

// 파일이 성공적으로 열린 경우에만 아래의 코드 실행    
void calculate_and_print_stats() {
	add_proc_list(PROC, true);
    unsigned int total = 0, running = 0, sleeping = 0, stopped = 0, zombie = 0, uninterruptible_sleep=0, tracedORstopped=0;
    total = procCnt;
    for(int i = 0; i < procCnt; i++){
        if(!strcmp(procList[i].stat, "R")) //실행 중인 프로세스
            running++;
        else if(!strcmp(procList[i].stat, "D")) //불가피하게 대기 중인 프로세스
            uninterruptible_sleep++;
        else if(!strcmp(procList[i].stat, "S")) //대기 중인 프로세스
            sleeping++;
        else if(!strcmp(procList[i].stat, "T")) //정지된 프로세스
            stopped++;
        else if(!strcmp(procList[i].stat, "t")) //추적 중인 프로세스 또는 멈춤 상태인 프로세스
            tracedORstopped++;
        else if(!strcmp(procList[i].stat, "Z")) //좀비 상태인 프로세스
            zombie++;
    }

    mvprintw(TASK_ROW, 0, "Tasks:  %4u total,  %4u running, %4u uninterruptible_sleep, %4u sleeping,  %4u stopped, %4u tracedORstopped, %4u zombie", total, running, uninterruptible_sleep, sleeping, stopped, tracedORstopped, zombie);
}