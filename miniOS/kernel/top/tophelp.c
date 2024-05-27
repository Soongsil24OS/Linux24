#include "top.h"

unsigned long get_uptime(void)
{
    FILE *fp;
    char buf[BUFFER_SIZE];
    long double time;

	memset(buf, '\0', BUFFER_SIZE);

    if ((fp = fopen(UPTIME, "r")) == NULL){
		fprintf(stderr, "fopen error for %s\n", UPTIME);
        exit(1);
    }
    fgets(buf, BUFFER_SIZE, fp);
    sscanf(buf, "%Lf", &time);
    fclose(fp);

    return (unsigned long)time;
}