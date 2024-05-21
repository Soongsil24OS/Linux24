/*  ***********************Include source**************************/
// include 이유와 사용부분, 이름을 꼭 ! 기입해주세요.
#include <stdio.h>
#include <string.h>
#include <math.h> // isnan 사용하기 위해 include - 3행, 민석
#include <ctype.h> // isdigit 사용하기 위해 include - 3행, 민석
#include <curses.h> // mvprintw 사용하기 위해 include - 3행, 민석
#include <unistd.h> // hertz 구하기 위해 include - 3행, 민석

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
