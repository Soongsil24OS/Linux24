#include "ttopsource2.h"

extern myProc procList[PROCESS_MAX];
extern int procCnt;
extern char myPath[PATH_LEN];		//자기 자신의 path

void add_proc_list(char path[PATH_LEN], bool isPPS) {
    FILE *statFp;
    if ((statFp = fopen(path, "r")) == NULL) {
        fprintf(stderr, "fopen error %s %s\n", strerror(errno), path);
        return;
    }

    myProc proc;
    memset(&proc, 0, sizeof(myProc));

    char statToken[MAX_TOKEN][TOKEN_LEN];
    memset(statToken, '\0', MAX_TOKEN * TOKEN_LEN);
    for (int i = 0; i < MAX_TOKEN; i++) {
        fscanf(statFp, "%s", statToken[i]);
    }
    fclose(statFp);

    proc.nice = atoi(statToken[STAT_NICE_IDX]); 	//nice 획득
    
    // 기본 state 획득
    proc.pid = (unsigned long)atoi(statToken[STAT_PID_IDX]);
    strcpy(proc.stat, statToken[STAT_STATE_IDX]);

    if (isPPS) {  // PPS(프로세스의 세부 상태를 구해야되는 경우)인 경우에만 세부 state 추가
        if (proc.nice < 0) {
            strcat(proc.stat, "<");
        } else if (proc.nice > 0) {
            strcat(proc.stat, "N");
        }

        unsigned long vmLck = 0;
        char statusPath[PATH_LEN];
        strcpy(statusPath, path);
        strcat(statusPath, "/status");

        FILE *statusFp;
        if ((statusFp = fopen(statusPath, "r")) != NULL) {
            char buf[256];
            while (fgets(buf, sizeof(buf), statusFp) != NULL) {
                if (strncmp(buf, "VmLck:", 6) == 0) {
                    sscanf(buf + 6, "%lu", &vmLck);
                    break;
                }
            }
            fclose(statusFp);
        }

        if (vmLck > 0) {
            strcat(proc.stat, "L");
        }

        int sid = atoi(statToken[5]);
        if (sid == proc.pid) {
            strcat(proc.stat, "s");
        }

        int threadCnt = atoi(statToken[STAT_N_THREAD_IDX]);
        if (threadCnt > 1) {
            strcat(proc.stat, "l");
        }

        int tpgid = atoi(statToken[STAT_TPGID_IDX]);
        if (tpgid != -1) {
            strcat(proc.stat, "+");
        }
    }
     extern myProc procList[];
    extern int procCnt;

    //전역 변수로 선언된 procList와 procCnt에 접근하여 값을 추가
    procList[procCnt] = proc;
    procCnt++;
}
