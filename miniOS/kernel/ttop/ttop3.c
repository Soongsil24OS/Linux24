#include <stdio.h>
#include <string.h>
#include <math.h> // isnan 사용하기 위해 include
#include <ctype.h> // isdigit 사용하기 위해 include
#include <curses.h> // mvprintw 사용하기 위해 include
#include <unistd.h> // hertz 구하기 위해 include
#define MEM_ROW 3	
/* --------------------header.h--------------start */
/* 3행 */
#define CPUSTAT "/proc/stat" 
#define CPUTicks 8
#define BUFFER_SIZE 1024
#define CPU_ROW 2				// TUI) cpu 출력할 2행
unsigned long uptime;			//os 부팅 후 지난 시간
unsigned long beforeUptime = 0;	//이전 실행 시의 os 부팅 후 지난 시각
unsigned long memTotal;			//전체 물리 메모리 크기
unsigned int hertz;	 			//os의 hertz값 얻기(초당 context switching 횟수)
long double beforeTicks[CPUTicks] = {0, };	//이전의 cpu ticks 저장하기 위한 배열
/* 4,5 행*/
#define MEMINFO "/proc/meminfo"		// /proc/meminfo 절대 경로, header.h
// /proc/meminfo 에서의 ROW
#define MEMINFO_MEM_TOTAL 1
#define MEMINFO_MEM_FREE 2
#define MEMINFO_MEM_AVAILABLE 3
#define MEMINFO_BUFFERS 4
#define MEMINFO_CACHED 5
/* --------------------header.h----------------end */

#define UPTIME "/proc/uptime"		// /proc/uptime 절대 경로
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


/* --------------------printtop()--------------start */
void print_top(void){

    hertz = (unsigned int)sysconf(_SC_CLK_TCK);	//os의 hertz값 얻기(초당 context switching 횟수)
    char buffer[BUFFER_SIZE]; // 0행
    uptime = get_uptime();




    /* 3행-------------------------------------------------------------------------- */
    char * CPUptr;
    long double us, sy, ni, id, wa, hi ,se, st;

    FILE *cpuStatFP;
    if((cpuStatFP = fopen(CPUSTAT, "r")) == NULL){
        fprintf(stderr, "CPU 사용 관련 파일 ( %s ) 을 읽어오는데 실패했습니다.\n", CPUSTAT);
        exit(1);
    }
    memset(buffer, '\0', BUFFER_SIZE);
    fclose(cpuStatFP);

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

    /* 4행-------------------------------------------------------------------------- */
    char * MEMptr;
    unsigned long memTotal, memFree, memUsed, memAvailable, buffers, cached;

    FILE * meminfoFP;

    if ((meminfoFP = fopen(MEMINFO, "r") == NULL)){
        fprintf(stderr, "MEM 사용 관련 파일 ( %s ) 을 읽어오는데 실패했습니다.\n", MEMINFO);
        exit(1);
    }

    int i = 0;
    /* ---------------------------------------------------------------------------- */
    while(i < MEMINFO_MEM_TOTAL){
        memset(buffer, '\0', BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, meminfoFP);
        i++;
    }
    MEMptr = buffer;
    while(!isdigit(*MEMptr)){
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", & memTotal);
    /* ---------------------------------------------------------------------------- */
    while(i < MEMINFO_MEM_FREE){
        memset(buffer, '\0', BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, meminfoFP);
        i++;
    }
    MEMptr = buffer;
    while(!isdigit(*MEMptr)){
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", & memFree);
    /* ---------------------------------------------------------------------------- */
    while(i < MEMINFO_MEM_AVAILABLE){
        memset(buffer, '\0', BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, meminfoFP);
        i++;
    }
    MEMptr = buffer;
    while(!isdigit(*MEMptr)){
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", & memAvailable);
    /* ---------------------------------------------------------------------------- */
    while(i < MEMINFO_BUFFERS){
        memset(buffer, '\0', BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, meminfoFP);
        i++;
    }
    MEMptr = buffer;
    while(!isdigit(*MEMptr)){
        MEMptr++;
    }
    sscanf(MEMptr, "%lu",& buffers);
    /* ---------------------------------------------------------------------------- */
	while(i < MEMINFO_CACHED){
		memset(buffer, '\0', BUFFER_SIZE);
    	fgets(buffer, BUFFER_SIZE, meminfoFP);
		i++;
	}
	MEMptr = buffer;
	while(!isdigit(*MEMptr)){
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", & cached);
    /* ---------------------------------------------------------------------------- */

    memUsed = memTotal - memFree - buffers - cached; // 사용 메모리 구하기

    mvprintw(MEM_ROW, 0, "Kib Mem : %8lu total,  %8lu free,  %8lu used,  %8lu buff/cache", memTotal, memFree, memUsed, buffers+cached); // 출력

    fclose(meminfoFP);
/* --------------------printtop()----------------end */
}