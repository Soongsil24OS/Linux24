#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <utmp.h>
#include <ctype.h>
#include <ncurses.h>

#define TOKEN_LEN 32



#define PROCESS_MAX 4096

// column에 출력할 문자열
#define PID_STR "PID"
#define USER_STR "USER"
#define PR_STR "PR"
#define NI_STR "NI"
#define VSZ_STR "VSZ"
#define VIRT_STR "VIRT"
#define RSS_STR "RSS"
#define RES_STR "RES"
#define SHR_STR "SHR"
#define S_STR "S"
#define STAT_STR "STAT"
#define START_STR "START"
#define TTY_STR "TTY"
#define CPU_STR "%CPU"
#define MEM_STR "MEM"
#define TIME_STR "TIME"
#define TIME_P_STR "TIME+"
#define CMD_STR "CMD"
#define COMMAND_STR "COMMAND"

#define TAB_WIDTH 8					//tab 길이