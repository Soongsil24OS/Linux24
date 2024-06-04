#include "top.h"

/* --------------------printtop()--------------start */
void print_top(void){

    hertz = (unsigned int)sysconf(_SC_CLK_TCK);	//os의 hertz값 얻기(초당 context switching 횟수)
    char buffer[BUFFER_SIZE]; // 0행
    uptime = get_uptime();

/*2행*/
int procCnt = 0;               // 외부 정의 및 초기화
// 파일이 성공적으로 열린 경우에만 아래의 코드 실행    
void calculate_and_print_stats() {
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
    int columnWidth[COLUMN_CNT] = {					//column의 x축 길이 저장하는 배열
		strlen(PID_STR), strlen(PR_STR), strlen(NI_STR),
		strlen(VIRT_STR), strlen(RES_STR), strlen(SHR_STR), strlen(S_STR),
		strlen(CPU_STR), strlen(MEM_STR), strlen(TIME_P_STR), strlen(COMMAND_STR) };

	for(int i = 0; i < procCnt; i++){			//PID 최대 길이 저장
		sprintf(buf, "%lu", procList[i].pid);
		if(columnWidth[PID_IDX] < strlen(buf))
			columnWidth[PID_IDX] = strlen(buf);
	}

	for(int i = 0; i < procCnt; i++){			//PR 최대 길이 저장
		sprintf(buf, "%d", procList[i].priority);
		if(columnWidth[PR_IDX] < strlen(buf))
			columnWidth[PR_IDX] = strlen(buf);
	}

	for(int i = 0; i < procCnt; i++){			//NI 최대 길이 저장
		sprintf(buf, "%d", procList[i].nice);
		if(columnWidth[NI_IDX] < strlen(buf))
			columnWidth[NI_IDX] = strlen(buf);
	}

	for(int i = 0; i < procCnt; i++){			//VIRT 최대 길이 저장
		sprintf(buf, "%lu", procList[i].vsz);
		if(columnWidth[VIRT_IDX] < strlen(buf))
			columnWidth[VIRT_IDX] = strlen(buf);
	}

	for(int i = 0; i < procCnt; i++){			//RES 최대 길이 저장
		sprintf(buf, "%lu", procList[i].rss);
		if(columnWidth[RES_IDX] < strlen(buf))
			columnWidth[RES_IDX] = strlen(buf);
	}

	for(int i = 0; i < procCnt; i++){			//SHR 최대 길이 저장
		sprintf(buf, "%lu", procList[i].shr);
		if(columnWidth[SHR_IDX] < strlen(buf))
			columnWidth[SHR_IDX] = strlen(buf);
	}

	for(int i = 0; i < procCnt; i++){			//S 최대 길이 저장
		if(columnWidth[S_IDX] < strlen(procList[i].stat))
			columnWidth[S_IDX] = strlen(procList[i].stat);
	}


	for(int i = 0; i < procCnt; i++){			//CPU 최대 길이 저장
		sprintf(buf, "%3.1Lf", procList[i].cpu);
		if(columnWidth[CPU_IDX] < strlen(buf))
			columnWidth[CPU_IDX] = strlen(buf);
	}

	for(int i = 0; i < procCnt; i++){			//MEM 최대 길이 저장
		sprintf(buf, "%3.1Lf", procList[i].mem);
		if(columnWidth[MEM_IDX] < strlen(buf))
			columnWidth[MEM_IDX] = strlen(buf);
	}

	for(int i = 0; i < procCnt; i++){			//TIME 최대 길이 저장
		if(columnWidth[TIME_P_IDX] < strlen(procList[i].time))
			columnWidth[TIME_P_IDX] = strlen(procList[i].time);
	}

	for(int i = 0; i < procCnt; i++){			//COMMAND 최대 길이 저장
		if(columnWidth[COMMAND_IDX] < strlen(procList[i].command))
			columnWidth[COMMAND_IDX] = strlen(procList[i].command);
	}

int startX[COLUMN_CNT] = {0, };				//각 column의 시작 x좌표

int startCol = 0, endCol = 0;
int maxCmd = -1;							//COMMAND 출력 가능한 최대 길이

	if(col >= COLUMN_CNT - 1){					//COMMAND COLUMN만 출력하는 경우 (우측 화살표 많이 누른 경우)
		startCol = COMMAND_IDX;                 //col: 사용자가 선택한 열의 인덱스
		endCol = COLUMN_CNT;
		maxCmd = COLS;							//COMMAND 터미널 너비만큼 출력 가능, COLS: 현재 사용 중인 터미널 창의 가로 길이
	}
	else{
		int i;
		for(i = col + 1; i < COLUMN_CNT; i++){
			startX[i] = columnWidth[i-1] + 2 + startX[i-1];
			if(startX[i] >= COLS){				//COLUMN의 시작이 이미 터미널 너비 초과한 경우
				endCol = i;
				break;
			}
		}
		startCol = col;
		if(i == COLUMN_CNT){
			endCol = COLUMN_CNT;					//COLUMN 전부 출력하는 경우
			maxCmd = COLS - startX[COMMAND_IDX];	//COMMAND 최대 출력 길이: COMMAND 터미널 너비 - COMMAND 시작 x좌표
		}
	}

/* 6행 column 출력 시작 */

//1행
attron(A_REVERSE); //attron: 특정 속성 활성화, attron(A_REVERSE): 텍스트 반전 
	for(int i = 0; i < COLS; i++)
		mvprintw(COLUMN_ROW, i, " ");

	int gap = 0;

	//PID 출력
	if(startCol <= PID_IDX && PID_IDX < endCol){
		gap = columnWidth[PID_IDX] - strlen(PID_STR);	//PID의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[PID_IDX] + gap, "%s", PID_STR);	//우측 정렬
	}

	//PR 출력
	if(startCol <= PR_IDX && PR_IDX < endCol){
		gap = columnWidth[PR_IDX] - strlen(PR_STR);		//PR 의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[PR_IDX] + gap, "%s", PR_STR);	//우측 정렬
	}

	//NI 출력
	if(startCol <= NI_IDX && NI_IDX < endCol){
		gap = columnWidth[NI_IDX] - strlen(NI_STR);		//NI 의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[NI_IDX] + gap, "%s", NI_STR);	//우측 정렬
	}

	//VIRT 출력
	if(startCol <= VIRT_IDX && VIRT_IDX < endCol){
		gap = columnWidth[VIRT_IDX] - strlen(VIRT_STR);	//VSZ의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[VIRT_IDX] + gap, "%s", VIRT_STR);	//우측 정렬
	}

	//RES 출력
	if(startCol <= RES_IDX && RES_IDX < endCol){
		gap = columnWidth[RES_IDX] - strlen(RES_STR);	//RSS의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[RES_IDX] + gap, "%s", RES_STR);	//우측 정렬
	}

	//SHR 출력
	if(startCol <= SHR_IDX && SHR_IDX < endCol){
		gap = columnWidth[SHR_IDX] - strlen(SHR_STR);	//SHR의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[SHR_IDX] + gap, "%s", SHR_STR);	//우측 정렬
	}

	//S 출력
	if(startCol <= S_IDX && S_IDX < endCol){
		mvprintw(COLUMN_ROW, startX[S_IDX], "%s", S_STR);	//우측 정렬
	}

	//%CPU 출력
	if(startCol <= CPU_IDX && CPU_IDX < endCol){
		gap = columnWidth[CPU_IDX] - strlen(CPU_STR);	//CPU의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[CPU_IDX] + gap, "%s", CPU_STR);	//우측 정렬
	}

	//%MEM 출력
	if(startCol <= MEM_IDX && MEM_IDX < endCol){
		gap = columnWidth[MEM_IDX] - strlen(MEM_STR);	//MEM의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[MEM_IDX] + gap, "%s", MEM_STR);	//우측 정렬
	}

	//TIME+ 출력
	if(startCol <= TIME_P_IDX && TIME_P_IDX < endCol){
		gap = columnWidth[TIME_P_IDX] - strlen(TIME_P_STR);	//TIME의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[TIME_P_IDX] + gap, "%s", TIME_P_STR);	//우측 정렬
	}

	//COMMAND 출력
	mvprintw(COLUMN_ROW, startX[COMMAND_IDX], "%s", COMMAND_STR);	//좌측 정렬

	attroff(A_REVERSE);

	/*****		column 출력 종료	*****/


	/*****		process 출력 시작	*****/ 

	char token[TOKEN_LEN];
	memset(token, '\0', TOKEN_LEN); //문자열을 저장할 임시 배열 'token'을 초기화

	for(int i = row; i < procCnt; i++){

		//PID 출력
		if(startCol <= PID_IDX && PID_IDX < endCol){
			memset(token, '\0', TOKEN_LEN);
			sprintf(token, "%lu", sorted[i]->pid);
			gap = columnWidth[PID_IDX] - strlen(token);	//PID의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[PID_IDX]+gap, "%s", token);	//우측 정렬
		}

		//PR 출력
		if(startCol <= PR_IDX && PR_IDX < endCol){
			memset(token, '\0', TOKEN_LEN);
			sprintf(token, "%d", sorted[i]->priority);
			gap = columnWidth[PR_IDX] - strlen(token);	//PR의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[PR_IDX]+gap, "%s", token);	//우측 정렬
		}

		//NI 출력
		if(startCol <= NI_IDX && NI_IDX < endCol){
			memset(token, '\0', TOKEN_LEN);
			sprintf(token, "%d", sorted[i]->nice);
			gap = columnWidth[NI_IDX] - strlen(token);	//NI의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[NI_IDX]+gap, "%s", token);	//우측 정렬
		}

		//VIRT 출력
		if(startCol <= VIRT_IDX && VIRT_IDX < endCol){
			memset(token, '\0', TOKEN_LEN);
			sprintf(token, "%lu", sorted[i]->vsz);
			gap = columnWidth[VIRT_IDX] - strlen(token);	//VIRT의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[VIRT_IDX]+gap, "%s", token);	//우측 정렬
		}

		//RES 출력
		if(startCol <= RES_IDX && RES_IDX < endCol){
			memset(token, '\0', TOKEN_LEN);
			sprintf(token, "%lu", sorted[i]->rss);
			gap = columnWidth[RES_IDX] - strlen(token);	//RES의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[RES_IDX]+gap, "%s", token);	//우측 정렬
		}

		//SHR 출력
		if(startCol <= SHR_IDX && SHR_IDX < endCol){
			memset(token, '\0', TOKEN_LEN);
			sprintf(token, "%lu", sorted[i]->shr);
			gap = columnWidth[SHR_IDX] - strlen(token);	//SHR의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[SHR_IDX]+gap, "%s", token);	//우측 정렬
		}

		//S 출력
		if(startCol <= S_IDX && S_IDX < endCol){
			gap = columnWidth[S_IDX] - strlen(sorted[i]->stat);	//S의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[S_IDX], "%s", sorted[i]->stat);	//좌측 정렬
		}

		//%CPU 출력
		if(startCol <= CPU_IDX && CPU_IDX < endCol){
			memset(token, '\0', TOKEN_LEN);
			sprintf(token, "%3.1Lf", sorted[i]->cpu);
			gap = columnWidth[CPU_IDX] - strlen(token);	//CPU의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[CPU_IDX]+gap, "%s", token);	//우측 정렬
		}

		//%MEM 출력
		if(startCol <= MEM_IDX && MEM_IDX < endCol){
			memset(token, '\0', TOKEN_LEN);
			sprintf(token, "%3.1Lf", sorted[i]->mem);
			gap = columnWidth[MEM_IDX] - strlen(token);	//MEM의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[MEM_IDX]+gap, "%s", token);	//우측 정렬
		}

		//TIME+ 출력
		if(startCol <= TIME_P_IDX && TIME_P_IDX < endCol){
			gap = columnWidth[TIME_P_IDX] - strlen(sorted[i]->time);	//TIME의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[TIME_P_IDX]+gap, "%s", sorted[i]->time);	//우측 정렬
		}

		//COMMAND 출력
		int tap = col - COMMAND_IDX;
		if((col == COMMAND_IDX) && (strlen(sorted[i]->command) < tap*TAB_WIDTH))		//COMMAND를 출력할 수 없는 경우
			continue;
		if(col < COLUMN_CNT - 1)	//다른 column도 함께 출력하는 경우
			tap = 0;
		sorted[i]->cmd[maxCmd] = '\0';
		mvprintw(COLUMN_ROW+1+i-row, startX[COMMAND_IDX], "%s", sorted[i]->cmd + tap*TAB_WIDTH);	//좌측 정렬

	}

	/*****		process 출력 종료	*****/
/* --------------------printtop()----------------end */
}
