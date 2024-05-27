#include "top.h"
extern myProc procList[4096];
extern int procCnt;

// 시스템 시작부터의 업타임을 가져오는 함수
unsigned long get_uptime(void) {
    FILE *fp;
    char buf[BUFFER_SIZE];
    long double time;

    memset(buf, '\0', BUFFER_SIZE);

    if ((fp = fopen(UPTIME, "r")) == NULL) {
        fprintf(stderr, "fopen error for %s\n", UPTIME);
        exit(1);
    }
    fgets(buf, BUFFER_SIZE, fp);
    sscanf(buf, "%Lf", &time);
    fclose(fp);

    return (unsigned long)time;
}

// 프로세스 정보를 검색하여 procList에 추가하는 함수
void get_procpath(unsigned long cpuTimeTable[999999]) {
    DIR *dirPtr;
    if ((dirPtr = opendir("/proc")) == NULL) { // /proc 디렉터리 open
        fprintf(stderr, "dirp error for /proc\n");
        exit(1);
    }
    struct dirent *dentryPtr;
    while ((dentryPtr = readdir(dirPtr)) != NULL) { // /proc 디렉터리 내 하위 파일들 탐색 시작
        if (dentryPtr->d_type != DT_DIR) // 디렉터리가 아닐 경우 skip
            continue;

        int len = strlen(dentryPtr->d_name);
        bool isPid = true;
        for (int i = 0; i < len; i++) { // 디렉터리가 PID인지 찾기
            if (!isdigit(dentryPtr->d_name[i])) { // 디렉터리명 중 숫자 아닌 문자가 있을 경우
                isPid = false;
                break;
            }
        }
        if (!isPid) // PID 디렉터리가 아닌 경우 skip
            continue;

        char path[1024]; // 디렉터리의 절대 경로 저장
        memset(path, '\0', 1024);
        strcpy(path, "/proc/");
        strcat(path, dentryPtr->d_name);

        add_proc_list(path, cpuTimeTable); // PID 디렉터리인 경우 procList에 추가
    }
    closedir(dirPtr);
    return;
}

// 프로세스 정보를 구조체에 저장하고 procList에 추가하는 함수
void add_proc_list(char path[1024], unsigned long cpuTimeTable[999999]) {
    if (access(path, R_OK) < 0) {
        fprintf(stderr, "access error for %s\n", path);
        return;
    }
    myProc proc;
    erase_proc(&proc);

    char statPath[1024];
    strcpy(statPath, path);
    strcat(statPath, "/stat");

    if (access(statPath, R_OK) < 0) {
        fprintf(stderr, "access error for %s\n", statPath);
        return;
    }
    FILE *statFp;
    if ((statFp = fopen(statPath, "r")) == NULL) {
        // fprintf(stderr, "fopen error %s %s\n", strerror(errno), statPath);
        sleep(1);
        return;
    }

    char statToken[32][32];
    memset(statToken, '\0', 32 * 32);
    for (int i = 0; i < 32; i++)
        fscanf(statFp, "%s", statToken[i]);
    fclose(statFp);

    proc.pid = (long)atoi(statToken[0]); // pid 획득

    // user명 획득

    // %cpu 계산

    // priority 및 nice 값 획득

    // START 획득

    // TIME 획득

    // command 획득

    // procList에 정보 저장
    procList[procCnt].pid = proc.pid;
    procCnt++;

    return;
}
