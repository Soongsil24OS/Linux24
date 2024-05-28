#include "header.h"

#define COLUMN_CNT 12 //출력할 column 최대 갯수

#define PID_IDX 0
#define USER_IDX 1
#define PR_IDX 2
#define NI_IDX 3
#define VIRT_IDX 4
#define RES_IDX 5
#define SHR_IDX 6
#define S_IDX 7
#define CPU_IDX 8
#define MEM_IDX 9
#define TIME_P_IDX 10
#define COMMAND_IDX 11

#define COLUMN_ROW 6			//column 출력할 행

myProc *sorted[PROCESS_MAX];	//procList를 cpu 순으로 sorting한 myProc 포인터 배열

int procCnt = 0;				//현재까지 완성한 myProc 갯수
int row, col;

int columnWidth[COLUMN_CNT] = {					//column의 x축 길이 저장하는 배열
		strlen(PID_STR), strlen(USER_STR), strlen(PR_STR), strlen(NI_STR),
		strlen(VIRT_STR), strlen(RES_STR), strlen(SHR_STR), strlen(S_STR),
		strlen(CPU_STR), strlen(MEM_STR), strlen(TIME_P_STR), strlen(COMMAND_STR) };

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
attron(A_REVERSE); //attron: 특정 속성 활성화, atron(A_REVERSE): 텍스트 반전 
	for(int i = 0; i < COLS; i++)
		mvprintw(COLUMN_ROW, i, " ");

	int gap = 0;

	//PID 출력
	if(startCol <= PID_IDX && PID_IDX < endCol){
		gap = columnWidth[PID_IDX] - strlen(PID_STR);	//PID의 길이 차 구함
		mvprintw(COLUMN_ROW, startX[PID_IDX] + gap, "%s", PID_STR);	//우측 정렬
	}

	//USER 출력
	if(startCol <= USER_IDX && USER_IDX < endCol)
		mvprintw(COLUMN_ROW, startX[USER_IDX], "%s", USER_STR);	//좌측 정렬

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

		//USER 출력 -> 여기서 memset을 안 하는 이유: 문자열로 변환하여 저장할 필요가 없음
		if(startCol <= USER_IDX && USER_IDX < endCol){
			gap = columnWidth[USER_IDX] - strlen(sorted[i]->user);	//TIME의 길이 차 구함
			mvprintw(COLUMN_ROW+1+i-row, startX[USER_IDX], "%s", sorted[i]->user);	//좌측 정렬
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

	return;
}
