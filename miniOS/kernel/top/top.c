#include "top.h"
#define TIME_SLICE 2
unsigned long beforeUptime = 0;
unsigned long beforeTicks[CPUTicks] = {0,};
unsigned long memTotal = 0;
unsigned hertz = 0;
unsigned long uptime = 0;
unsigned long cpuTimeTable[PID_MAX];
myProc procList[PROCESS_MAX];
myProc *sorted[PROCESS_MAX];
int procCnt = 0;
bool by_cpu = true;

time_t before; // CPU usage calculation
time_t now;

pid_t myPid;
uid_t myUid;
char myPath[PATH_LEN];

int ch;
int row, col;

bool isGreater(myProc *a, myProc *b) {
    if (a->cpu < b->cpu)
        return true;
    else if (a->cpu > b->cpu)
        return false;
    else {
        return a->pid > b->pid;
    }
}

void sort_processes(bool by_cpu) {
    // Copy pointers
    for (int i = 0; i < procCnt; i++)
        sorted[i] = &procList[i];

    // Bubble sort
    for (int i = procCnt - 1; i > 0; i--) {
        for (int j = 0; j < i; j++) {
            bool should_swap = false;

            if (by_cpu) {
                // Compare by CPU usage
                should_swap = isGreater(sorted[j], sorted[j + 1]);
            } else {
                // Compare by PID
                should_swap = sorted[j]->pid > sorted[j + 1]->pid;
            }

            if (should_swap) {
                myProc *tmp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = tmp;
            }
        }
    }
}

void print_column_headers(int startCol, int endCol, int col, int *startX, int *columnWidth) {
    for (int i = startCol; i < endCol; i++) {
        if (i == col) { // Highlight the current column
            attron(A_REVERSE);
        }

        // Print the text for each column
        switch (i) {
            case PID_IDX:
                mvprintw(COLUMN_ROW, startX[PID_IDX], "%*s", columnWidth[PID_IDX], PID_STR);
                break;
            case PR_IDX:
                mvprintw(COLUMN_ROW, startX[PR_IDX], "%*s", columnWidth[PR_IDX], PR_STR);
                break;
            case NI_IDX:
                mvprintw(COLUMN_ROW, startX[NI_IDX], "%*s", columnWidth[NI_IDX], NI_STR);
                break;
            case VIRT_IDX:
                mvprintw(COLUMN_ROW, startX[VIRT_IDX], "%*s", columnWidth[VIRT_IDX], VIRT_STR);
                break;
            case RES_IDX:
                mvprintw(COLUMN_ROW, startX[RES_IDX], "%*s", columnWidth[RES_IDX], RES_STR);
                break;
            case SHR_IDX:
                mvprintw(COLUMN_ROW, startX[SHR_IDX], "%*s", columnWidth[SHR_IDX], SHR_STR);
                break;
            case S_IDX:
                mvprintw(COLUMN_ROW, startX[S_IDX], "%s", S_STR);
                break;
            case CPU_IDX:
                mvprintw(COLUMN_ROW, startX[CPU_IDX], "%*s", columnWidth[CPU_IDX], CPU_STR);
                break;
            case MEM_IDX:
                mvprintw(COLUMN_ROW, startX[MEM_IDX], "%*s", columnWidth[MEM_IDX], MEM_STR);
                break;
            case TIME_P_IDX:
                mvprintw(COLUMN_ROW, startX[TIME_P_IDX], "%*s", columnWidth[TIME_P_IDX], TIME_P_STR);
                break;
            case COMMAND_IDX:
                mvprintw(COLUMN_ROW, startX[COMMAND_IDX], "%s", COMMAND_STR);
                break;
        }

        if (i == col) { // Remove the highlight from the current column
            attroff(A_REVERSE);
        }
    }
}

void roundRobinScheduling(myProc sorted[], int procCnt) {
    initscr(); // Initialize ncurses
    noecho();  // Do not echo input characters
    curs_set(FALSE); // Do not display the cursor
    timeout(0); // Non-blocking input

    int time_passed = 0;
    while (1) {
        for (int i = 0; i < procCnt; i++) {
            // Clear the screen
            clear();

            // Display process info
            mvprintw(0, 0, "PID: %lu", sorted[i].pid);
            mvprintw(1, 0, "User: %s", sorted[i].user);
            mvprintw(2, 0, "CPU Usage: %.2Lf", sorted[i].cpu);
            mvprintw(3, 0, "Memory Usage: %.2Lf", sorted[i].mem);
            mvprintw(4, 0, "Command: %s", sorted[i].command);
            mvprintw(5, 0, "Time Slice: %d", TIME_SLICE);
            mvprintw(6, 0, "Time Passed: %d", time_passed);
            mvprintw(7, 0, "Press 'q' to quit.");

            // Refresh to show changes
            refresh();

            // Check if 'q' is pressed
            int ch = getch();
            if (ch == 'q') {
                endwin(); // End ncurses mode
                return;
            }

            // Sleep for the time slice duration
            sleep(TIME_SLICE);

            // Increment time passed
            time_passed += TIME_SLICE;
        }
    }

    endwin(); // End ncurses mode
}

void print_top(void) {
    /*1. Get Uptime */
    time_t uptime = get_uptime(); // Time since the system boot
    char buf[BUFFER_SIZE];

    /* 1st row: Print UPTIME */
    /*2. Create a string for the current time */
    char nowStr[128] = {0}; // Initialize the string for the current time
    time_t now = time(NULL); // Get the current time
    struct tm *tmNow = localtime(&now); // Convert the current time to struct tm

    // Save the current time in the format "top - HH:MM:SS " in nowStr
    strftime(nowStr, sizeof(nowStr), "top - %H:%M:%S ", tmNow);

    /*3. Create an uptime
        string */
    struct tm *tmUptime = localtime(&uptime);

    char upStr[128] = {0}; // Initialize the uptime string
    if (uptime < 60 * 60) {
        snprintf(upStr, sizeof(upStr), "%2d min", tmUptime->tm_min);
    } else if (uptime < 60 * 60 * 24) {
        snprintf(upStr, sizeof(upStr), "%2d:%02d", tmUptime->tm_hour, tmUptime->tm_min);
    } else {
        snprintf(upStr, sizeof(upStr), "%3d days, %02d:%02d", tmUptime->tm_yday, tmUptime->tm_hour, tmUptime->tm_min);
    }

    /* 4. Get Load Average */
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

    /*5. Print */
    mvprintw(TOP_ROW, 0, "%sup %s, load average: %4.2Lf, %4.2Lf, %4.2Lf", nowStr, upStr, loadAvg[0], loadAvg[1], loadAvg[2]);

    /* 2nd row */
    unsigned int total = 0, running = 0, sleeping = 0, stopped = 0, zombie = 0, uninterruptible_sleep = 0, tracedORstopped = 0;
    total = procCnt;
    for (int i = 0; i < procCnt; i++) {
        if (!strcmp(procList[i].stat, "R")) // Running process
            running++;
        else if (!strcmp(procList[i].stat, "D")) // Uninterruptible sleep process
            uninterruptible_sleep++;
        else if (!strcmp(procList[i].stat, "S")) // Sleeping process
            sleeping++;
        else if (!strcmp(procList[i].stat, "T")) // Stopped process
            stopped++;
        else if (!strcmp(procList[i].stat, "t")) // Traced or stopped process
            tracedORstopped++;
        else if (!strcmp(procList[i].stat, "Z")) // Zombie process
            zombie++;
    }

    mvprintw(TASK_ROW, 0, "Tasks:  %4u total,  %4u running, %4u uninterruptible_sleep, %4u sleeping,  %4u stopped, %4u tracedORstopped, %4u zombie", total, running, uninterruptible_sleep, sleeping, stopped, tracedORstopped, zombie);

    hertz = (unsigned int)sysconf(_SC_CLK_TCK); // Get the hertz value of the OS (context switching per second)
    char buffer[BUFFER_SIZE]; // 0th row
    uptime = get_uptime();

    /* 3rd row */
    char *CPUptr;
    FILE *cpuStatFP;
    if ((cpuStatFP = fopen(CPUSTAT, "r")) == NULL) {
        fprintf(stderr, "Failed to read CPU usage file (%s).\n", CPUSTAT);
        exit(1);
    }
    memset(buffer, '\0', BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, cpuStatFP); // Read CPU information
    fclose(cpuStatFP);

    CPUptr = buffer;
    while (!isdigit(*CPUptr)) CPUptr++; // Find the digit

    long double ticks[CPUTicks] = {0.0,};
    sscanf(CPUptr, "%Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf", &ticks[0], &ticks[1], &ticks[2], &ticks[3], &ticks[4], &ticks[5], &ticks[6], &ticks[7]);
    unsigned long nowTicks = 0;
    long double results[CPUTicks] = {0.0,};

    if (beforeUptime == 0) {
        nowTicks = uptime * hertz;
        for (int i = 0; i < CPUTicks; i++) {
            results[i] = ticks[i];
        }
    } else {
        nowTicks = (uptime - beforeUptime) * hertz;
        for (int i = 0; i < CPUTicks; i++) {
            results[i] = ticks[i] - beforeTicks[i];
        }
    }

    for (int i = 0; i < CPUTicks; i++) {
        results[i] = (results[i] / nowTicks) * 100;
        if (isnan(results[i]) || isinf(results[i])) {
            results[i] = 0;
        }
    }

    mvprintw(CPU_ROW, 0, "%%Cpu(s):  %4.1Lf us, %4.1Lf sy, %4.1Lf ni, %4.1Lf id, %4.1Lf wa, %4.1Lf hi, %4.1Lf si, %4.1Lf st",
             results[0], results[1], results[2], results[3], results[4], results[5], results[6], results[7]);

    beforeUptime = uptime;
    for (int i = 0; i < CPUTicks; i++)
        beforeTicks[i] = ticks[i];

    /* 4th row */
    char *MEMptr;
    unsigned long memTotal, memFree, memUsed, memAvailable, buffers, cached;

    FILE *meminfoFP;
    if ((meminfoFP = fopen(MEMINFO, "r")) == NULL) {
        fprintf(stderr, "Failed to read memory usage file (%s).\n", MEMINFO);
        exit(1);
    }

    fgets(buffer, BUFFER_SIZE, meminfoFP);
    MEMptr = buffer;
    while (!isdigit(*MEMptr)) MEMptr++;
    sscanf(MEMptr, "%lu", &memTotal);

    fgets(buffer, BUFFER_SIZE, meminfoFP);
    MEMptr = buffer;
    while (!isdigit(*MEMptr)) MEMptr++;
    sscanf(MEMptr, "%lu", &memFree);

    fgets(buffer, BUFFER_SIZE, meminfoFP);
    MEMptr = buffer;
    while (!isdigit(*MEMptr)) MEMptr++;
    sscanf(MEMptr, "%lu", &memAvailable);

    fgets(buffer, BUFFER_SIZE, meminfoFP);
    MEMptr = buffer;
    while (!isdigit(*MEMptr)) MEMptr++;
    sscanf(MEMptr, "%lu", &buffers);

    fgets(buffer, BUFFER_SIZE, meminfoFP);
    MEMptr = buffer;
    while (!isdigit(*MEMptr)) MEMptr++;
    sscanf(MEMptr, "%lu", &cached);

    memUsed = memTotal - memFree - buffers - cached; // Calculate used memory

    mvprintw(MEM_ROW, 0, "Kib Mem : %8lu total,  %8lu free,  %8lu used,  %8lu buff/cache", memTotal, memFree, memUsed, buffers + cached); // Print

    fclose(meminfoFP);

    int columnWidth[COLUMN_CNT] = {
        strlen(PID_STR), strlen(PR_STR), strlen(NI_STR),
        strlen(VIRT_STR), strlen(RES_STR), strlen(SHR_STR), strlen(S_STR),
        strlen(CPU_STR), strlen(MEM_STR), strlen(TIME_P_STR), strlen(COMMAND_STR)
    };

    for (int i = 0; i < procCnt; i++) {
        sprintf(buf, "%lu", procList[i].pid);
        if (columnWidth[PID_IDX] < strlen(buf))
            columnWidth[PID_IDX] = strlen(buf);
    }

    for (int i = 0; i < procCnt; i++) {
        sprintf(buf, "%d", procList[i].priority);
        if (columnWidth[PR_IDX] < strlen(buf))
            columnWidth[PR_IDX] = strlen(buf);
    }

    for (int i = 0; i < procCnt; i++) {
        sprintf(buf, "%d", procList[i].nice);
        if (columnWidth[NI_IDX] < strlen(buf))
            columnWidth[NI_IDX] = strlen(buf);
    }

    for (int i = 0; i < procCnt; i++) {
        sprintf(buf, "%lu", procList[i].vsz);
        if (columnWidth[VIRT_IDX] < strlen(buf))
            columnWidth[VIRT_IDX] = strlen(buf);
    }

    for (int i = 0; i < procCnt; i++) {
        sprintf(buf, "%lu", procList[i].rss);
        if (columnWidth[RES_IDX] < strlen(buf))
            columnWidth[RES_IDX] = strlen(buf);
    }

    for (int i = 0; i < procCnt; i++) {
        sprintf(buf, "%lu", procList[i].shr);
        if (columnWidth[SHR_IDX] < strlen(buf))
            columnWidth[SHR_IDX] = strlen(buf);
    }

    for (int i = 0; i < procCnt; i++) {
        if (columnWidth[S_IDX] < strlen(procList[i].stat))
            columnWidth[S_IDX] = strlen(procList[i].stat);
    }
    for (int i = 0; i < procCnt; i++) {
    sprintf(buf, "%3.1Lf", procList[i].cpu);
    if (columnWidth[CPU_IDX] < strlen(buf))
        columnWidth[CPU_IDX] = strlen(buf);
    }

    for (int i = 0; i < procCnt; i++) {
        sprintf(buf, "%3.1Lf", procList[i].mem);
        if (columnWidth[MEM_IDX] < strlen(buf))
            columnWidth[MEM_IDX] = strlen(buf);
    }

    for (int i = 0; i < procCnt; i++) {
        if (columnWidth[TIME_P_IDX] < strlen(procList[i].time))
            columnWidth[TIME_P_IDX] = strlen(procList[i].time);
    }

    for (int i = 0; i < procCnt; i++) {
        if (columnWidth[COMMAND_IDX] < strlen(procList[i].command))
            columnWidth[COMMAND_IDX] = strlen(procList[i].command);
    }

    int startX[COLUMN_CNT] = {0};
    for (int i = 1; i < COLUMN_CNT; i++) {
        startX[i] = startX[i - 1] + columnWidth[i - 1] + 2;
    }

    int startCol = 0;
    int endCol = COLUMN_CNT;
    int maxCmd = COLS - startX[COMMAND_IDX];

    print_column_headers(startCol, endCol, col, startX, columnWidth);

    char token[TOKEN_LEN];
    memset(token, '\0', TOKEN_LEN);
    for (int i = row; i < procCnt; i++) {
        int gap = 0;
        if (startCol <= PID_IDX && PID_IDX < endCol) {
            sprintf(token, "%lu", sorted[i]->pid);
            gap = columnWidth[PID_IDX] - strlen(token);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[PID_IDX] + gap, "%s", token);
        }

        if (startCol <= PR_IDX && PR_IDX < endCol) {
            sprintf(token, "%d", sorted[i]->priority);
            gap = columnWidth[PR_IDX] - strlen(token);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[PR_IDX] + gap, "%s", token);
        }

        if (startCol <= NI_IDX && NI_IDX < endCol) {
            sprintf(token, "%d", sorted[i]->nice);
            gap = columnWidth[NI_IDX] - strlen(token);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[NI_IDX] + gap, "%s", token);
        }

        if (startCol <= VIRT_IDX && VIRT_IDX < endCol) {
            sprintf(token, "%lu", sorted[i]->vsz);
            gap = columnWidth[VIRT_IDX] - strlen(token);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[VIRT_IDX] + gap, "%s", token);
        }

        if (startCol <= RES_IDX && RES_IDX < endCol) {
            sprintf(token, "%lu", sorted[i]->rss);
            gap = columnWidth[RES_IDX] - strlen(token);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[RES_IDX] + gap, "%s", token);
        }

        if (startCol <= SHR_IDX && SHR_IDX < endCol) {
            sprintf(token, "%lu", sorted[i]->shr);
            gap = columnWidth[SHR_IDX] - strlen(token);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[SHR_IDX] + gap, "%s", token);
        }

        if (startCol <= S_IDX && S_IDX < endCol) {
            gap = columnWidth[S_IDX] - strlen(sorted[i]->stat);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[S_IDX], "%s", sorted[i]->stat);
        }

        if (startCol <= CPU_IDX && CPU_IDX < endCol) {
            sprintf(token, "%3.1Lf", sorted[i]->cpu);
            gap = columnWidth[CPU_IDX] - strlen(token);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[CPU_IDX] + gap, "%s", token);
        }

        if (startCol <= MEM_IDX && MEM_IDX < endCol) {
            sprintf(token, "%3.1Lf", sorted[i]->mem);
            gap = columnWidth[MEM_IDX] - strlen(token);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[MEM_IDX] + gap, "%s", token);
        }

        if (startCol <= TIME_P_IDX && TIME_P_IDX < endCol) {
            gap = columnWidth[TIME_P_IDX] - strlen(sorted[i]->time);
            mvprintw(COLUMN_ROW + 1 + i - row, startX[TIME_P_IDX] + gap, "%s", sorted[i]->time);
        }

        int tap = col - COMMAND_IDX;
        if (col == COMMAND_IDX && strlen(sorted[i]->command) < tap * TAB_WIDTH) {
            continue;
        }
        if (col < COLUMN_CNT - 1) {
            tap = 0;
        }
        sorted[i]->cmd[maxCmd] = '\0';
        mvprintw(COLUMN_ROW + 1 + i - row, startX[COMMAND_IDX], "%s", sorted[i]->cmd + tap * TAB_WIDTH);
    }
}

void top(void) {
    bool print = false;
    memTotal = get_mem_total(); // Total physical memory size
    hertz = (unsigned int)sysconf(_SC_CLK_TCK); // OS’s hertz value (context switching per second)
    now = time(NULL);
    memset(cpuTimeTable, 0, sizeof(cpuTimeTable)); // Initialize CPU time table
    /* Get process information */
    myPid = getpid(); // Get own PID
    char pidPath[128];
    snprintf(pidPath, sizeof(pidPath), "/%d", myPid);

    /* Set up display environment */
    initscr(); // Initialize the screen
    halfdelay(10); // Refresh input every 0.1 seconds
    noecho(); // Disable echo
    keypad(stdscr, TRUE); // Enable special key input
    curs_set(0); // Make cursor invisible

    /* Initialize process information and first print */
    get_procPath(cpuTimeTable);
    row = 0;
    col = 0;
    ch = 0;
    before = time(NULL);

    sort_processes(by_cpu); // Sort by CPU usage
    print_top(); // Initial print
    refresh();

    /* Infinite loop for periodic screen updates */
    do {
        now = time(NULL); // Update current time

        switch (ch) {
            case KEY_LEFT:
                col--;
                if (col < 0)
                    col = 0;
                print = true;
                break;
            case KEY_RIGHT:
                col++;
                if (col >= COLUMN_CNT)
                    col = COLUMN_CNT - 1;
                print = true;
                break;
            case '\n': // Enter key
            case KEY_ENTER:
                if(col == 0){
                    by_cpu = false;
                    print = true;}
                if(col == 7)
                    roundRobinScheduling(sorted, procCnt);
                break;
        }

        if (print || now - before >= 3) { // Refresh screen every 3 seconds
            erase();
            erase_proc_list();
            get_procPath(cpuTimeTable);
            sort_processes(by_cpu); // Sort by CPU usage
            print_top();
            refresh();
            before = now;
            print = false;
        }

    } while ((ch = getch()) != 'q'); // Exit on 'q' key press

    endwin();
}


void clear_scr(void) {
    for (int i = 0; i < LINES; i++) {
        move(i, 0); // Move the cursor to the start of each line
    for (int j = 0; j < COLS; j++) {
        addch(' ');
        }
    }
}