/*
 * Utility for fetching memory (RAM) info from /proc/meminfo
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct meminfo {
	unsigned long total;
	unsigned long used;
	unsigned long free;
	unsigned long buffers;
	unsigned long cached;
	unsigned long shmem;
	unsigned long sreclaimable;
};

struct meminfo mem_stats();

int main(int argc, char *argv[])
{
	int byte_type = 0, percentage_total = 0, quiet = 0, tmp, amnt_dgts = 0, amnt_mem_cpy;
	float return_mem = 0;
	char *return_mem_str;
	struct meminfo mi = mem_stats();

	if (argc < 2)
		goto not_enough_args;

	for (tmp = 0; tmp < argc; tmp++) {
		if (!strcmp(argv[tmp], "--percentage-total") || !strcmp(argv[tmp], "-p"))
			percentage_total = 1;

		if ((!strcmp(argv[tmp], "--quiet") || !strcmp(argv[tmp], "-q")) && !percentage_total)
			quiet = 1;

		if ((!strcmp(argv[tmp], "--kilobytes") || !strcmp(argv[tmp], "-k")) && !percentage_total)
			byte_type = 1;
		else if ((!strcmp(argv[tmp], "--megabytes") || !strcmp(argv[tmp], "-m")) && !percentage_total)
			byte_type = 2;
		else if ((!strcmp(argv[tmp], "--gigabytes") || !strcmp(argv[tmp], "-g")) && !percentage_total)
			byte_type = 3;

		if (!strcmp(argv[tmp], "--free") || !strcmp(argv[tmp], "-f"))
			return_mem = mi.free;
		else if (!strcmp(argv[tmp], "--used") || !strcmp(argv[tmp], "-u"))
			return_mem = mi.used;
		else if (!strcmp(argv[tmp], "--total") || !strcmp(argv[tmp], "-t"))
			return_mem = mi.total;
	}

	if (!return_mem)
		goto no_mem_type;

	for (tmp = 0; tmp < byte_type; tmp++)
		return_mem /= 1000;

	if (quiet) {
		printf("%0.0f\n", return_mem);
	} else if (percentage_total) {
		printf("%0.0f%%\n", return_mem / mi.total * 100);
	} else {
		amnt_mem_cpy = return_mem;

		while (amnt_mem_cpy != 0) {
			amnt_mem_cpy /= 10;
			amnt_dgts++;
		}

		return_mem_str = malloc(amnt_dgts + 3);

		switch (byte_type) {
			case 0:
				sprintf(return_mem_str, "%0.0fB", return_mem);
				break;
			case 1:
				sprintf(return_mem_str, "%0.0fkB", return_mem);
				break;
			case 2:
				sprintf(return_mem_str, "%0.0fMB", return_mem);
				break;
			case 3:
				sprintf(return_mem_str, "%0.2fGB", return_mem);
				break;
		}

		printf("%s\n", return_mem_str);
	}

	return 0;

not_enough_args:
	printf("ERR: no arguments specified.\n");
	return 1;
no_mem_type:
	printf("ERR: no memory return type specified.\n");
	return 1;
}

struct meminfo mem_stats()
{
	char file_content[20];
	struct meminfo mi;
	int loc = 0;

	FILE *memfile = fopen("/proc/meminfo", "r");

	while (fscanf(memfile, "%s", file_content) != EOF) {
		if (!strcmp(file_content, "MemTotal:")) {
			loc = 1;
			continue;
		} else if (!strcmp(file_content, "MemFree:")) {
			loc = 2;
			continue;
		} else if (!strcmp(file_content, "Buffers:")) {
			loc = 3;
			continue;
		} else if (!strcmp(file_content, "Cached:")) {
			loc = 4;
			continue;
		} else if (!strcmp(file_content, "Shmem:")) {
			loc = 5;
			continue;
		} else if (!strcmp(file_content, "SReclaimable:")) {
			loc = 6;
			continue;
		}

		// /proc/meminfo records values in kB by default, convert them to bytes.
		switch (loc) {
			case 1:
				mi.total = (unsigned long)atoi(file_content) * 1000;
			case 2:
				mi.free = (unsigned long)atoi(file_content) * 1000;
			case 3:
				mi.buffers = (unsigned long)atoi(file_content) * 1000;
			case 4:
				mi.cached = (unsigned long)atoi(file_content) * 1000;
			case 5:
				mi.shmem = (unsigned long)atoi(file_content) * 1000;
			case 6:
				mi.sreclaimable = (unsigned long)atoi(file_content) * 1000;
		}

		loc = 0;
	}

	fclose(memfile);

	mi.used = mi.total + mi.shmem - mi.free - mi.buffers - mi.cached - mi.sreclaimable;

	return mi;
}
