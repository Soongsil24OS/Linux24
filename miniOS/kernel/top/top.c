#include "top.h"

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

	mvprintw(CPU_row, 0, "%%Cpu(s):  %4.1Lf us, %4.1Lf sy, %4.1Lf ni, %4.1Lf id, %4.1Lf wa, %4.1Lf hi, %4.1Lf si, %4.1Lf st",
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
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while(!isdigit(*MEMptr)){
        MEMptr++;}
    sscanf(MEMptr, "%lu", & memTotal);
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while(!isdigit(*MEMptr)){
        MEMptr++;}
    sscanf(MEMptr, "%lu", & memFree);
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while(!isdigit(*MEMptr)){
        MEMptr++;}
    sscanf(MEMptr, "%lu", & memAvailable);
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while(!isdigit(*MEMptr)){
        MEMptr++;}
    sscanf(MEMptr, "%lu",& buffers);
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

	MEMptr = buffer;
	while(!isdigit(*MEMptr)){
        MEMptr++;}
    sscanf(MEMptr, "%lu", & cached);
    /* ---------------------------------------------------------------------------- */

    memUsed = memTotal - memFree - buffers - cached; // 사용 메모리 구하기

    mvprintw(MEM_row, 0, "Kib Mem : %8lu total,  %8lu free,  %8lu used,  %8lu buff/cache", memTotal, memFree, memUsed, buffers+cached); // 출력

    fclose(meminfoFP);
/* --------------------printtop()----------------end */
}