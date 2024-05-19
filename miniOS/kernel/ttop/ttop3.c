#include <stdio.h>
#include <string.h>
#include <math.h> // isnan 사용하기 위해 include
#include <ctype.h> // isdigit 사용하기 위해 include
#include <curses.h> // mvprintw 사용하기 위해 include

#define CPUSTAT "/proc/stat"
#define CPUTicks 8
#define BUFFER_SIZE 1024

#define CPU_ROW 2				// TUI) cpu 출력할 2행


unsigned long uptime;			//os 부팅 후 지난 시간
unsigned long beforeUptime = 0;	//이전 실행 시의 os 부팅 후 지난 시각
unsigned long memTotal;			//전체 물리 메모리 크기
unsigned int hertz;	 			//os의 hertz값 얻기(초당 context switching 횟수)
long double beforeTicks[CPUTicks] = {0, };	//이전의 cpu ticks 저장하기 위한 배열
// 3행을 위한 변수들이다. header.h에서 가져왔다.



void main(void){
    char buffer[BUFFER_SIZE]; // 0행
    uptime = get_uptime();

    /* 3행 */
    char * CPUptr;
    long double us, sy, ni, id, wa, hi ,se, st;

    FILE *cpuStatFp;
    if((cpuStatFp = fopen(CPUSTAT, "r")) == NULL){
        fprintf(stderr, "CPU 사용 관련 파일 ( %s ) 을 읽어오는데 실패했습니다.\n", CPUSTAT);
        exit(1);
    }
    memset(buffer, '\0', BUFFER_SIZE);
    fclose(cpuStatFp);

    CPUptr = buffer;

    while(!isdigit(*CPUptr)) CPUptr++;

    long double ticks[CPUTicks] = {0.0, }; // 배열을 0.0 으로 초기화
    sscanf(CPUptr, "%Lf%Lf%Lf%Lf%Lf%Lf%Lf%Lf",
    &ticks[0], &ticks[1], &ticks[2], &ticks[3], &ticks[4], &ticks[5], &ticks[6], &ticks[7]);

    //sscanf를 사용해 CPUptr에서 읽은 값들을 각 Ticks의 주소에 저장한다.

    unsigned long nowTicks = 0; // 현재 틱을 계산할 변수이다.
    long double results[CPUTicks] = {0.0, }; // 결과를 출력할 results 배열을 0.0으로 초기화한다.

    if(beforeUptime == 0){
        nowTicks = uptime * hertz; // doit) uptime과 hertz 구해주는 함수 만들어야함.
        for (int i = 0; i < CPUTicks; i++){
            results[i] = ticks[i]; // 아까 CPUptr을 통해 저장한 ticks를 불러옴.
        }
    }
    else{
        nowTicks = (uptime - beforeUptime) * hertz;
        for (int i = 0; i < CPUTicks; i++){
            results[i] = ticks[i] - beforeTicks[i]; // 최초실행과 달리 이전 ticks를 빼서 출력해야 한다.
        }
    }
    
    for (int i = 0; i < CPUTicks; i++){
        results[i] = (results[i]/nowTicks) * 100; // 퍼센트로 구하기. 뭘 구할까? results 배열의 ticks value를 현재 실행된 ticks로 나누고, 100을 곱하면 해당 results 배열의 ticks 수가 전체 ticks 수 중 몇 % 정도 차지하는 지 알 수 있다.
        if (isnan(results[i]) || isinf(results[i])){
            results[i] = 0;
        }
    }

	mvprintw(CPU_ROW, 0, "%%Cpu(s):  %4.1Lf us, %4.1Lf sy, %4.1Lf ni, %4.1Lf id, %4.1Lf wa, %4.1Lf hi, %4.1Lf si, %4.1Lf st",
    results[0], results[2], results[1], results[3], results[4], results[5], results[6], results[7]);

	beforeUptime = uptime;
	for(int i = 0; i < CPUTicks; i++)					
		beforeTicks[i] = ticks[i];

}