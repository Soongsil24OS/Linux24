#include "top.h"

/* --------------------printtop()--------------start */
void print_top(void){
    /*1. Uptime 가져오기*/
    uptime = get_uptime();			//os 부팅 후 지난 시각
    char buf[BUFFER_SIZE];

	/*****	1행 UPTIME 출력	*****/

	/*2. 현재 시각 문자열 생성*/
	char nowStr[128] = {0};       // 현재 시각 문자열을 초기화
    time_t now = time(NULL);      // 현재 시각을 얻기
    struct tm *tmNow = localtime(&now);  // 현재 시각을 struct tm으로 변환

    // 현재 시각을 "top - HH:MM:SS " 형식으로 nowStr에 저장
    strftime(nowStr, sizeof(nowStr), "top - %H:%M:%S ", tmNow);

    /*3. Uptime 문자열 생성*/
	struct tm *tmUptime = localtime(&uptime);

    char upStr[128] = {0};  // uptime 문자열 초기화
    if (uptime < 60 * 60) {
        snprintf(upStr, sizeof(upStr), "%2d min", tmUptime->tm_min);
    } else if (uptime < 60 * 60 * 24) {
        snprintf(upStr, sizeof(upStr), "%2d:%02d", tmUptime->tm_hour, tmUptime->tm_min);
    } else {
        snprintf(upStr, sizeof(upStr), "%3d days, %02d:%02d", tmUptime->tm_yday, tmUptime->tm_hour, tmUptime->tm_min);
    }

    /* 4. Load Average 가져오기 */
    FILE *loadAvgFp;
    long double loadAvg[3];
    if ((loadAvgFp = fopen(LOADAVG, "r")) == NULL) {
        fprintf(stderr, "fopen error for %s\n", LOADAVG);
        exit(1);
    }

    if (fgets(buf, BUFFER_SIZE, loadAvgFp) != NULL) {
        sscanf(buf, "%Lf%Lf%Lf", &loadAvg[0], &loadAvg[1], &loadAvg[2]);
    }
    fclose(loadAvgFp);


    /*5. 출력*/
	mvprintw(TOP_ROW, 0, "%sup %s, %d users, load average: %4.2Lf, %4.2Lf, %4.2Lf", nowStr, upStr, users, loadAvg[0], loadAvg[1], loadAvg[2]);




    hertz = (unsigned int)sysconf(_SC_CLK_TCK);	//os의 hertz값 얻기(초당 context switching 횟수)
    char buffer[BUFFER_SIZE]; // 0행
    uptime = get_uptime();

    /* 3행-------------------------------------------------------2------------------- */
    char * CPUptr;
    long double us, sy, ni, id, wa, hi ,se, st;

    FILE *cpuStatFP;
    if((cpuStatFP = fopen(CPUSTAT, "r")) == NULL){
        fprintf(stderr, "CPU 사용 관련 파일 ( %s ) 을 읽어오는데 실패했습니다.\n", 4);
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




void update_cpu(void)
{
	return;
}

// 화면 출력을 모두 초기화하는 함수
void clear_scr(void)
{
    for (int i = 0; i < LINES; i++) {
        move(i, 0);          // 커서를 각 줄의 처음으로 이동
        for (int j = 0; j < COLS; j++) {
            addch(' ');
        }
    }
    return;
}


int main(int argc, char *argv[])
{
/*1.메모리 및 시스템 정보 초기화*/
	memTotal = get_mem_total();                    // 전체 물리 메모리 크기
    hertz = (unsigned int)sysconf(_SC_CLK_TCK);    // OS의 hertz값 얻기(초당 context switching 횟수)
    now = time(NULL);   
    memset(cpuTimeTable, 0, sizeof(cpuTimeTable)); // CPU 시간 테이블 초기화

/*프로세스 및 TTY 정보 가져오기*/
	myPid = getpid();            // 자기 자신의 pid
    snprintf(pidPath, sizeof(pidPath), "/%d", myPid);
    snprintf(myPath, sizeof(myPath), "%s%s", PROC, pidPath); // 자기 자신의 /proc 경로 획득
    getTTY(myPath, myTTY);       // 자기 자신의 tty 획득
    for (int i = strlen(PTS); i < strlen(myTTY); i++) {
        if (!isdigit(myTTY[i])) {
            myTTY[i] = '\0';
            break;
        }
    }
    myUid = getuid();            // 자기 자신의 uid

/*3.출력 환경 설정*/
	initscr();				//출력 윈도우 초기화
	halfdelay(10);			//0.1초마다 입력 새로 갱신
	noecho();				//echo 제거
	keypad(stdscr, TRUE);	//특수 키 입력 허용
	curs_set(0);			//curser invisible

/*4.프로세스 정보 초기화 및 첫 출력*/
	search_proc(false, false, false, false, cpuTimeTable);

	row = 0;
	col = 0;

	ch = 0;

	before = time(NULL);

	sort_by_cpu();			//cpu 순으로 정렬
	print_ttop();			//초기 출력
	refresh();

/*5.무한 루프를 통한 주기적 화면 갱신*/
	do{						//무한 반복
		now = time(NULL);	//현재 시각 갱신

		if(print || now - before >= 3){	//3초 경과 시 화면 갱신
			erase();
			erase_proc_list();
			search_proc(false, false, false, false, cpuTimeTable);
			sort_by_cpu();			//cpu 순으로 정렬
			print_ttop();
			refresh();
			before = now;
			print = false;
		}

	}while((ch = getch()) != 'q');	//q 입력 시 종료

	endwin();

	return 0;
}

//