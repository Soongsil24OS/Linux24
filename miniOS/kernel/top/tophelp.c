#include "top.h"

extern myProc procList[4096];
extern int procCnt;

//proc의 내용을 지우는 함수
void erase_proc(myProc *proc)
{
	proc->pid = 0;
	proc->uid = 0;
	memset(proc->user, '\0', UNAME_LEN);
	proc->cpu = 0.0;
	proc->mem = 0.0;
	proc->vsz = 0;
	proc->rss = 0;
	proc->shr = 0;
	proc->priority = 0;
	proc->nice = 0;
	memset(proc->tty, '\0', TTY_LEN);
	memset(proc->stat, '\0', STAT_LEN);
	memset(proc->start, '\0', TIME_LEN);
	memset(proc->time, '\0', TIME_LEN);
	memset(proc->cmd, '\0', CMD_LEN);
	memset(proc->command, '\0', CMD_LEN);
	return;
}

// procList 내용 지우는 함수
void erase_proc_list(void)
{
	for(int i = 0; i < procCnt; i++)
		erase_proc(procList + i);
	procCnt = 0;
	return;
}

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
    char statusPath[1024];
    strcpy(statusPath, path);
    strcat(statusPath, "/status");

    if (access(statusPath, R_OK) < 0) {
        fprintf(stderr, "access error for %s\n", statusPath);
        return;
    }
    FILE *statusFp;
    if ((statusFp = fopen(statusPath, "r")) == NULL) {
        sleep(1);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), statusFp)) {
        if (strncmp(line, "Uid:", 4) == 0) {
            unsigned long uid;
            sscanf(line, "Uid:\t%lu", &uid);
            proc.uid = uid;
            struct passwd *pw = getpwuid(uid);
            if (pw) {
                strncpy(proc.user, pw->pw_name, UNAME_LEN - 1);
                proc.user[UNAME_LEN - 1] = '\0'; // Ensure null-termination
            } else {
                strncpy(proc.user, "unknown", UNAME_LEN - 1);
                proc.user[UNAME_LEN - 1] = '\0'; // Ensure null-termination
            }
            break;
        }
    }
    fclose(statusFp);

    // %cpu 계산
    unsigned long utime = (unsigned long)atoi(statToken[13]);
    unsigned long stime = (unsigned long)atoi(statToken[14]);
    unsigned long startTime = (unsigned long)atoi(statToken[21]);
    unsigned long long totalTime = utime + stime;

    struct sysinfo sysInfo;
    sysinfo(&sysInfo);
    unsigned long long uptime = sysInfo.uptime;
    unsigned long long Hertz = sysconf(_SC_CLK_TCK);

    long double seconds = uptime - (startTime / Hertz);
    long double cpuUsage = 100.0 * ((totalTime / Hertz) / seconds);

    if (cpuUsage < 0 || cpuUsage > 100) {
        proc.cpu = 0;
    } else {
        proc.cpu = roundl(cpuUsage * 100) / 100.0; // 소수점 2자리 반올림
    }

    // 메모리 사용률 계산
    unsigned long vsz = 0, rss = 0, shr = 0, vmLck = 0;
    if ((statusFp = fopen(statusPath, "r")) != NULL) {
        while (fgets(line, sizeof(line), statusFp)) {
            if (strncmp(line, "VmSize:", 7) == 0) {
                sscanf(line, "VmSize:\t%lu", &vsz);
            } else if (strncmp(line, "VmRSS:", 6) == 0) {
                sscanf(line, "VmRSS:\t%lu", &rss);
            } else if (strncmp(line, "RssShmem:", 9) == 0) {
                sscanf(line, "RssShmem:\t%lu", &shr);
            } else if (strncmp(line, "VmLck:", 6) == 0) {
                sscanf(line, "VmLck:\t%lu", &vmLck);
            }
        }
        fclose(statusFp);
    }

    proc.vsz = vsz;
    proc.rss = rss;
    proc.shr = shr;

    long double memUsage = (long double)rss / sysInfo.totalram * 100.0;
    if (memUsage < 0 || memUsage > 100) {
        proc.mem = 0;
    } else {
        proc.mem = roundl(memUsage * 100) / 100.0; // 소수점 2자리 반올림
    }

    // priority 및 nice 값 획득
    proc.priority = atoi(statToken[17]);
    proc.nice = atoi(statToken[18]);

    // START 획득
    unsigned long start = time(NULL) - uptime + (startTime / Hertz);
    struct tm *tmStart = localtime(&start);
    if (time(NULL) - start < 24 * 60 * 60) {
        strftime(proc.start, TIME_LEN, "%H:%M", tmStart);
    } else if (time(NULL) - start < 7 * 24 * 60 * 60) {
        strftime(proc.start, TIME_LEN, "%b %d", tmStart);
    } else {
        strftime(proc.start, TIME_LEN, "%y", tmStart);
    }

    // TIME 획득
    unsigned long cpuTime = totalTime / Hertz;
    struct tm *tmCpuTime = localtime(&cpuTime);
    if (!isPPS || (!aOption && !uOption && !xOption)) { // ttop이거나 pps에서 옵션이 없을 경우
        sprintf(proc.time, "%02d:%02d:%02d", tmCpuTime->tm_hour, tmCpuTime->tm_min, tmCpuTime->tm_sec);
    } else {
        sprintf(proc.time, "%1d:%02d", tmCpuTime->tm_min, tmCpuTime->tm_sec);
    }

    // command 획득
    sscanf(statToken[1], "(%s", proc.cmd); // cmd 획득
    proc.cmd[strlen(proc.cmd) - 1] = '\0'; // 마지막 ')' 제거

    // procList에 정보 저장
    procList[procCnt] = proc;
    procCnt++;

    return;
}

=======
myProc procList[999999]; // assuming a large enough array for demonstration
int procCnt = 0;

// 시스템 시작부터의 업타임을 가져오는 함수
unsigned long get_uptime(void) {
    FILE* fp;
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

//proc의 내용을 지우는 함수
void erase_proc(myProc* proc)
{
    proc->pid = 0;
    proc->uid = 0;
    memset(proc->user, '\0', UNAME_LEN);
    proc->cpu = 0.0;
    proc->mem = 0.0;
    proc->vsz = 0;
    proc->rss = 0;
    proc->shr = 0;
    proc->priority = 0;
    proc->nice = 0;
    memset(proc->tty, '\0', TTY_LEN);
    memset(proc->stat, '\0', STAT_LEN);
    memset(proc->start, '\0', TIME_LEN);
    memset(proc->time, '\0', TIME_LEN);
    memset(proc->cmd, '\0', CMD_LEN);
    memset(proc->command, '\0', CMD_LEN);
    return;
}

// procList 내용 지우는 함수
void erase_proc_list(void)
{
    for (int i = 0; i < procCnt; i++)
        erase_proc(procList + i);
    procCnt = 0;
    return;
}

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
    FILE* statFp;
    if ((statFp = fopen(statPath, "r")) == NULL) {
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
    char statusPath[1024];
    strcpy(statusPath, path);
    strcat(statusPath, "/status");

    if (access(statusPath, R_OK) < 0) {
        fprintf(stderr, "access error for %s\n", statusPath);
        return;
    }
    FILE* statusFp;
    if ((statusFp = fopen(statusPath, "r")) == NULL) {
        sleep(1);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), statusFp)) {
        if (strncmp(line, "Uid:", 4) == 0) {
            unsigned long uid;
            sscanf(line, "Uid:\t%lu", &uid);
            proc.uid = uid;
            struct passwd* pw = getpwuid(uid);
            if (pw) {
                strncpy(proc.user, pw->pw_name, UNAME_LEN - 1);
                proc.user[UNAME_LEN - 1] = '\0'; // Ensure null-termination
            }
            else {
                strncpy(proc.user, "unknown", UNAME_LEN - 1);
                proc.user[UNAME_LEN - 1] = '\0'; // Ensure null-termination
            }
            break;
        }
    }
    fclose(statusFp);

    // %cpu 계산
    unsigned long utime = (unsigned long)atoi(statToken[13]);
    unsigned long stime = (unsigned long)atoi(statToken[14]);
    unsigned long startTime = (unsigned long)atoi(statToken[21]);
    unsigned long long totalTime = utime + stime;

    struct sysinfo sysInfo;
    sysinfo(&sysInfo);
    unsigned long long uptime = sysInfo.uptime;
    unsigned long long Hertz = sysconf(_SC_CLK_TCK);

    long double seconds = uptime - (startTime / Hertz);
    long double cpuUsage = 100.0 * ((totalTime / Hertz) / seconds);

    if (cpuUsage < 0 || cpuUsage > 100) {
        proc.cpu = 0;
    }
    else {
        proc.cpu = roundl(cpuUsage * 100) / 100.0; // 소수점 2자리 반올림
    }

    // 메모리 사용률 계산
    unsigned long vsz = 0, rss = 0, shr = 0, vmLck = 0;
    if ((statusFp = fopen(statusPath, "r")) != NULL) {
        while (fgets(line, sizeof(line), statusFp)) {
            if (strncmp(line, "VmSize:", 7) == 0) {
                sscanf(line, "VmSize:\t%lu", &vsz);
            }
            else if (strncmp(line, "VmRSS:", 6) == 0) {
                sscanf(line, "VmRSS:\t%lu", &rss);
            }
            else if (strncmp(line, "RssShmem:", 9) == 0) {
                sscanf(line, "RssShmem:\t%lu", &shr);
            }
            else if (strncmp(line, "VmLck:", 6) == 0) {
                sscanf(line, "VmLck:\t%lu", &vmLck);
            }
        }
        fclose(statusFp);
    }

    proc.vsz = vsz;
    proc.rss = rss;
    proc.shr = shr;

    long double memUsage = (long double)rss / sysInfo.totalram * 100.0;
    if (memUsage < 0 || memUsage > 100) {
        proc.mem = 0;
    }
    else {
        proc.mem = roundl(memUsage * 100) / 100.0; // 소수점 2자리 반올림
    }

    // priority 및 nice 값 획득
    proc.priority = atoi(statToken[17]);
    proc.nice = atoi(statToken[18]);

    // START 획득
    unsigned long start = time(NULL) - uptime + (startTime / Hertz);
    struct tm* tmStart = localtime(&start);
    if (time(NULL) - start < 24 * 60 * 60) {
        strftime(proc.start, TIME_LEN, "%H:%M", tmStart);
    }
    else if (time(NULL) - start < 7 * 24 * 60 * 60) {
        strftime(proc.start, TIME_LEN, "%b %d", tmStart);
    }
    else {
        strftime(proc.start, TIME_LEN, "%y", tmStart);
    }

    // TIME 획득
    unsigned long cpuTime = totalTime / Hertz;
    struct tm* tmCpuTime = localtime(&cpuTime);
    if (!isPPS || (!aOption && !uOption && !xOption)) { // ttop이거나 pps에서 옵션이 없을 경우
        sprintf(proc.time, "%02d:%02d:%02d", tmCpuTime->tm_hour, tmCpuTime->tm_min, tmCpuTime->tm_sec);
    }
    else {
        sprintf(proc.time, "%1d:%02d", tmCpuTime->tm_min, tmCpuTime->tm_sec);
    }

    // command 획득
    sscanf(statToken[1], "(%s", proc.cmd); // cmd 획득
    proc.cmd[strlen(proc.cmd) - 1] = '\0'; // 마지막 ')' 제거

    // procList에 정보 저장
    procList[procCnt] = proc;
    procCnt++;

    return;
}

>>>>>>> 8ca88d3c8b853004ec3f00ec075b6bf4ede155c9
void get_procpath(unsigned long cpuTimeTable[999999]) {
    DIR* dirPtr;
    if ((dirPtr = opendir("/proc")) == NULL) { // /proc 디렉터리 open
        fprintf(stderr, "dirp error for /proc\n");
        exit(1);
    }
    struct dirent* dentryPtr;
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