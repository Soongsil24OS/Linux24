#include "top.h"

/* --------------------printtop()--------------start */
void print_top(void) {
    /*1. Uptime 가져오기*/
    uptime = get_uptime();			//os 부팅 후 지난 시각
    char buf[BUFFER_SIZE];

    /*****	1행 UPTIME 출력	*****/

    /*2. 현재 시각 문자열 생성*/
    char nowStr[128] = { 0 };       // 현재 시각 문자열을 초기화
    time_t now = time(NULL);      // 현재 시각을 얻기
    struct tm* tmNow = localtime(&now);  // 현재 시각을 struct tm으로 변환

    // 현재 시각을 "top - HH:MM:SS " 형식으로 nowStr에 저장
    strftime(nowStr, sizeof(nowStr), "top - %H:%M:%S ", tmNow);

    /*3. Uptime 문자열 생성*/
    struct tm* tmUptime = localtime(&uptime);

    char upStr[128] = { 0 };  // uptime 문자열 초기화
    if (uptime < 60 * 60) {
        snprintf(upStr, sizeof(upStr), "%2d min", tmUptime->tm_min);
    }
    else if (uptime < 60 * 60 * 24) {
        snprintf(upStr, sizeof(upStr), "%2d:%02d", tmUptime->tm_hour, tmUptime->tm_min);
    }
    else {
        snprintf(upStr, sizeof(upStr), "%3d days, %02d:%02d", tmUptime->tm_yday, tmUptime->tm_hour, tmUptime->tm_min);
    }

    /* 4. Load Average 가져오기 */
    FILE* loadAvgFp;
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
    mvprintw(TOP_ROW, 0, "%sup %s, load average: %4.2Lf, %4.2Lf, %4.2Lf", nowStr, upStr, loadAvg[0], loadAvg[1], loadAvg[2]);

    /* 2행-------------------------------------------------------------------------- */
    add_proc_list(PROC, true);
    unsigned int total = 0, running = 0, sleeping = 0, stopped = 0, zombie = 0, uninterruptible_sleep = 0, tracedORstopped = 0;
    total = procCnt;
    for (int i = 0; i < procCnt; i++) {
        if (!strcmp(procList[i].stat, "R")) //실행 중인 프로세스
            running++;
        else if (!strcmp(procList[i].stat, "D")) //불가피하게 대기 중인 프로세스
            uninterruptible_sleep++;
        else if (!strcmp(procList[i].stat, "S")) //대기 중인 프로세스
            sleeping++;
        else if (!strcmp(procList[i].stat, "T")) //정지된 프로세스
            stopped++;
        else if (!strcmp(procList[i].stat, "t")) //추적 중인 프로세스 또는 멈춤 상태인 프로세스
            tracedORstopped++;
        else if (!strcmp(procList[i].stat, "Z")) //좀비 상태인 프로세스
            zombie++;
    }

    mvprintw(TASK_ROW, 0, "Tasks:  %4u total,  %4u running, %4u uninterruptible_sleep, %4u sleeping,  %4u stopped, %4u tracedORstopped, %4u zombie", total, running, uninterruptible_sleep, sleeping, stopped, tracedORstopped, zombie);



    hertz = (unsigned int)sysconf(_SC_CLK_TCK);	//os의 hertz값 얻기(초당 context switching 횟수)
    char buffer[BUFFER_SIZE]; // 0행
    uptime = get_uptime();

    /* 3행-------------------------------------------------------------------------- */
    char* CPUptr;
    long double us, sy, ni, id, wa, hi, se, st;

    FILE* cpuStatFP;
    if ((cpuStatFP = fopen(CPUSTAT, "r")) == NULL) {
        fprintf(stderr, "CPU 사용 관련 파일 ( %s ) 을 읽어오는데 실패했습니다.\n", CPUSTAT);
        exit(1);
    }
    memset(buffer, '\0', BUFFER_SIZE);
    fclose(cpuStatFP);

    CPUptr = buffer;

    while (!isdigit(*CPUptr)) CPUptr++;

    long double ticks[CPUTicks] = { 0.0, }; // 배열을 0.0 으로 초기화
    sscanf(CPUptr, "%Lf%Lf%Lf%Lf%Lf%Lf%Lf%Lf",
        &ticks[0], &ticks[1], &ticks[2], &ticks[3], &ticks[4], &ticks[5], &ticks[6], &ticks[7]);

    //sscanf를 사용해 CPUptr에서 읽은 값들을 각 Ticks의 주소에 저장한다.

    unsigned long nowTicks = 0; // 현재 틱을 계산할 변수이다.
    long double results[CPUTicks] = { 0.0, }; // 결과를 출력할 results 배열을 0.0으로 초기화한다.

    if (beforeUptime == 0) {
        nowTicks = uptime * hertz; // doit) uptime과 hertz 구해주는 함수 만들어야함.
        for (int i = 0; i < CPUTicks; i++) {
            results[i] = ticks[i]; // 아까 CPUptr을 통해 저장한 ticks를 불러옴.
        }
    }
    else {
        nowTicks = (uptime - beforeUptime) * hertz;
        for (int i = 0; i < CPUTicks; i++) {
            results[i] = ticks[i] - beforeTicks[i]; // 최초실행과 달리 이전 ticks를 빼서 출력해야 한다.
        }
    }

    for (int i = 0; i < CPUTicks; i++) {
        results[i] = (results[i] / nowTicks) * 100; // 퍼센트로 구하기. 뭘 구할까? results 배열의 ticks value를 현재 실행된 ticks로 나누고, 100을 곱하면 해당 results 배열의 ticks 수가 전체 ticks 수 중 몇 % 정도 차지하는 지 알 수 있다.
        if (isnan(results[i]) || isinf(results[i])) {
            results[i] = 0;
        }
    }

    mvprintw(CPU_row, 0, "%%Cpu(s):  %4.1Lf us, %4.1Lf sy, %4.1Lf ni, %4.1Lf id, %4.1Lf wa, %4.1Lf hi, %4.1Lf si, %4.1Lf st",
        results[0], results[2], results[1], results[3], results[4], results[5], results[6], results[7]);

    beforeUptime = uptime;
    for (int i = 0; i < CPUTicks; i++)
        beforeTicks[i] = ticks[i];

    /* 4행-------------------------------------------------------------------------- */
    char* MEMptr;
    unsigned long memTotal, memFree, memUsed, memAvailable, buffers, cached;

    FILE* meminfoFP;

    if ((meminfoFP = fopen(MEMINFO, "r") == NULL)) {
        fprintf(stderr, "MEM 사용 관련 파일 ( %s ) 을 읽어오는데 실패했습니다.\n", MEMINFO);
        exit(1);
    }

    int i = 0;
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while (!isdigit(*MEMptr)) {
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", &memTotal);
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while (!isdigit(*MEMptr)) {
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", &memFree);
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while (!isdigit(*MEMptr)) {
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", &memAvailable);
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while (!isdigit(*MEMptr)) {
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", &buffers);
    /* ---------------------------------------------------------------------------- */
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, meminfoFP);
    i++;

    MEMptr = buffer;
    while (!isdigit(*MEMptr)) {
        MEMptr++;
    }
    sscanf(MEMptr, "%lu", &cached);
    /* ---------------------------------------------------------------------------- */

    memUsed = memTotal - memFree - buffers - cached; // 사용 메모리 구하기

    mvprintw(MEM_row, 0, "Kib Mem : %8lu total,  %8lu free,  %8lu used,  %8lu buff/cache", memTotal, memFree, memUsed, buffers + cached); // 출력

    fclose(meminfoFP);

    myProc* sorted[PROCESS_MAX];	//procList를 cpu 순으로 sorting한 myProc 포인터 배열

    int procCnt = 0;				//현재까지 완성한 myProc 갯수
    int row, col;

    int columnWidth[COLUMN_CNT] = {					//column의 x축 길이 저장하는 배열
            strlen(PID_STR), strlen(USER_STR), strlen(PR_STR), strlen(NI_STR),
            strlen(VIRT_STR), strlen(RES_STR), strlen(SHR_STR), strlen(S_STR),
            strlen(CPU_STR), strlen(MEM_STR), strlen(TIME_P_STR), strlen(COMMAND_STR) };

    int startX[COLUMN_CNT] = { 0, };				//각 column의 시작 x좌표

    int startCol = 0, endCol = 0;
    int maxCmd = -1;							//COMMAND 출력 가능한 최대 길이

    if (col >= COLUMN_CNT - 1) {					//COMMAND COLUMN만 출력하는 경우 (우측 화살표 많이 누른 경우)
        startCol = COMMAND_IDX;                 //col: 사용자가 선택한 열의 인덱스
        endCol = COLUMN_CNT;
        maxCmd = COLS;							//COMMAND 터미널 너비만큼 출력 가능, COLS: 현재 사용 중인 터미널 창의 가로 길이
    }
    else {
        int i;
        for (i = col + 1; i < COLUMN_CNT; i++) {
            startX[i] = columnWidth[i - 1] + 2 + startX[i - 1];
            if (startX[i] >= COLS) {				//COLUMN의 시작이 이미 터미널 너비 초과한 경우
                endCol = i;
                break;
            }
        }
        startCol = col;
        if (i == COLUMN_CNT) {
            endCol = COLUMN_CNT;					//COLUMN 전부 출력하는 경우
            maxCmd = COLS - startX[COMMAND_IDX];	//COMMAND 최대 출력 길이: COMMAND 터미널 너비 - COMMAND 시작 x좌표
        }
    }

    /* 6행 column 출력 시작 */

    //1행
    attron(A_REVERSE); //attron: 특정 속성 활성화, atron(A_REVERSE): 텍스트 반전 
    for (int i = 0; i < COLS; i++)
        mvprintw(COLUMN_ROW, i, " ");

    int gap = 0;

    //PID 출력
    if (startCol <= PID_IDX && PID_IDX < endCol) {
        gap = columnWidth[PID_IDX] - strlen(PID_STR);	//PID의 길이 차 구함
        mvprintw(COLUMN_ROW, startX[PID_IDX] + gap, "%s", PID_STR);	//우측 정렬
    }

    //USER 출력
    if (startCol <= USER_IDX && USER_IDX < endCol)
        mvprintw(COLUMN_ROW, startX[USER_IDX], "%s", USER_STR);	//좌측 정렬

    //PR 출력
    if (startCol <= PR_IDX && PR_IDX < endCol) {
        gap = columnWidth[PR_IDX] - strlen(PR_STR);		//PR 의 길이 차 구함
        mvprintw(COLUMN_ROW, startX[PR_IDX] + gap, "%s", PR_STR);	//우측 정렬
    }

    //NI 출력
    if (startCol <= NI_IDX && NI_IDX < endCol) {
        gap = columnWidth[NI_IDX] - strlen(NI_STR);		//NI 의 길이 차 구함
        mvprintw(COLUMN_ROW, startX[NI_IDX] + gap, "%s", NI_STR);	//우측 정렬
    }

    //VIRT 출력
    if (startCol <= VIRT_IDX && VIRT_IDX < endCol) {
        gap = columnWidth[VIRT_IDX] - strlen(VIRT_STR);	//VSZ의 길이 차 구함
        mvprintw(COLUMN_ROW, startX[VIRT_IDX] + gap, "%s", VIRT_STR);	//우측 정렬
    }

    //RES 출력
    if (startCol <= RES_IDX && RES_IDX < endCol) {
        gap = columnWidth[RES_IDX] - strlen(RES_STR);	//RSS의 길이 차 구함
        mvprintw(COLUMN_ROW, startX[RES_IDX] + gap, "%s", RES_STR);	//우측 정렬
    }

    //SHR 출력
    if (startCol <= SHR_IDX && SHR_IDX < endCol) {
        gap = columnWidth[SHR_IDX] - strlen(SHR_STR);	//SHR의 길이 차 구함
        mvprintw(COLUMN_ROW, startX[SHR_IDX] + gap, "%s", SHR_STR);	//우측 정렬
    }

    //S 출력
    if (startCol <= S_IDX && S_IDX < endCol) {
        mvprintw(COLUMN_ROW, startX[S_IDX], "%s", S_STR);	//우측 정렬
    }

    //%CPU 출력
    if (startCol <= CPU_IDX && CPU_IDX < endCol) {
        gap = columnWidth[CPU_IDX] - strlen(CPU_STR);	//CPU의 길이 차 구함
        mvprintw(COLUMN_ROW, startX[CPU_IDX] + gap, "%s", CPU_STR);	//우측 정렬
    }

    //%MEM 출력
    if (startCol <= MEM_IDX && MEM_IDX < endCol) {
        gap = columnWidth[MEM_IDX] - strlen(MEM_STR);	//MEM의 길이 차 구함
        mvprintw(COLUMN_ROW, startX[MEM_IDX] + gap, "%s", MEM_STR);	//우측 정렬
    }

    //TIME+ 출력
    if (startCol <= TIME_P_IDX && TIME_P_IDX < endCol) {
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

    for (int i = row; i < procCnt; i++) {

        //PID 출력
        if (startCol <= PID_IDX && PID_IDX < endCol) {
            memset(token, '\0', TOKEN_LEN);
            sprintf(token, "%lu", sorted[i]->pid);
            gap = columnWidth[PID_IDX] - strlen(token);	//PID의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[PID_IDX] + gap, "%s", token);	//우측 정렬
        }

        //USER 출력 -> 여기서 memset을 안 하는 이유: 문자열로 변환하여 저장할 필요가 없음
        if (startCol <= USER_IDX && USER_IDX < endCol) {
            gap = columnWidth[USER_IDX] - strlen(sorted[i]->user);	//TIME의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[USER_IDX], "%s", sorted[i]->user);	//좌측 정렬
        }

        //PR 출력
        if (startCol <= PR_IDX && PR_IDX < endCol) {
            memset(token, '\0', TOKEN_LEN);
            sprintf(token, "%d", sorted[i]->priority);
            gap = columnWidth[PR_IDX] - strlen(token);	//PR의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[PR_IDX] + gap, "%s", token);	//우측 정렬
        }

        //NI 출력
        if (startCol <= NI_IDX && NI_IDX < endCol) {
            memset(token, '\0', TOKEN_LEN);
            sprintf(token, "%d", sorted[i]->nice);
            gap = columnWidth[NI_IDX] - strlen(token);	//NI의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[NI_IDX] + gap, "%s", token);	//우측 정렬
        }

        //VIRT 출력
        if (startCol <= VIRT_IDX && VIRT_IDX < endCol) {
            memset(token, '\0', TOKEN_LEN);
            sprintf(token, "%lu", sorted[i]->vsz);
            gap = columnWidth[VIRT_IDX] - strlen(token);	//VIRT의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[VIRT_IDX] + gap, "%s", token);	//우측 정렬
        }

        //RES 출력
        if (startCol <= RES_IDX && RES_IDX < endCol) {
            memset(token, '\0', TOKEN_LEN);
            sprintf(token, "%lu", sorted[i]->rss);
            gap = columnWidth[RES_IDX] - strlen(token);	//RES의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[RES_IDX] + gap, "%s", token);	//우측 정렬
        }

        //SHR 출력
        if (startCol <= SHR_IDX && SHR_IDX < endCol) {
            memset(token, '\0', TOKEN_LEN);
            sprintf(token, "%lu", sorted[i]->shr);
            gap = columnWidth[SHR_IDX] - strlen(token);	//SHR의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[SHR_IDX] + gap, "%s", token);	//우측 정렬
        }

        //S 출력
        if (startCol <= S_IDX && S_IDX < endCol) {
            gap = columnWidth[S_IDX] - strlen(sorted[i]->stat);	//S의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[S_IDX], "%s", sorted[i]->stat);	//좌측 정렬
        }

        //%CPU 출력
        if (startCol <= CPU_IDX && CPU_IDX < endCol) {
            memset(token, '\0', TOKEN_LEN);
            sprintf(token, "%3.1Lf", sorted[i]->cpu);
            gap = columnWidth[CPU_IDX] - strlen(token);	//CPU의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[CPU_IDX] + gap, "%s", token);	//우측 정렬
        }

        //%MEM 출력
        if (startCol <= MEM_IDX && MEM_IDX < endCol) {
            memset(token, '\0', TOKEN_LEN);
            sprintf(token, "%3.1Lf", sorted[i]->mem);
            gap = columnWidth[MEM_IDX] - strlen(token);	//MEM의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[MEM_IDX] + gap, "%s", token);	//우측 정렬
        }

        //TIME+ 출력
        if (startCol <= TIME_P_IDX && TIME_P_IDX < endCol) {
            gap = columnWidth[TIME_P_IDX] - strlen(sorted[i]->time);	//TIME의 길이 차 구함
            mvprintw(COLUMN_ROW + 1 + i - row, startX[TIME_P_IDX] + gap, "%s", sorted[i]->time);	//우측 정렬
        }

        //COMMAND 출력
        int tap = col - COMMAND_IDX;
        if ((col == COMMAND_IDX) && (strlen(sorted[i]->command) < tap * TAB_WIDTH))		//COMMAND를 출력할 수 없는 경우
            continue;
        if (col < COLUMN_CNT - 1)	//다른 column도 함께 출력하는 경우
            tap = 0;
        sorted[i]->cmd[maxCmd] = '\0';
        mvprintw(COLUMN_ROW + 1 + i - row, startX[COMMAND_IDX], "%s", sorted[i]->cmd + tap * TAB_WIDTH);	//좌측 정렬

    }

    /*****		process 출력 종료	*****/
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


int main(int argc, char* argv[])
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
    do {						//무한 반복
        now = time(NULL);	//현재 시각 갱신

        switch (ch) {			//방향키 입력 좌표 처리
        case KEY_LEFT:
            col--;
            if (col < 0)
                col = 0;
            print = true;
            break;
        case KEY_RIGHT:
            col++;
            print = true;
            break;
        case KEY_UP:
            row--;
            if (row < 0)
                row = 0;
            print = true;
            break;
        case KEY_DOWN:
            row++;
            if (row > procCnt)
                row = procCnt;
            print = true;
            break;
        }

        if (print || now - before >= 3) {	//3초 경과 시 화면 갱신
            erase();
            erase_proc_list();
            search_proc(false, false, false, false, cpuTimeTable);
            sort_by_cpu();			//cpu 순으로 정렬
            print_ttop();
            refresh();
            before = now;
            print = false;
        }

    } while ((ch = getch()) != 'q');	//q 입력 시 종료

    endwin();

    return 0;
}
