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

int main(int argc, char *argv[]) {
	int byte_type = 0, percentage_total = 0, quiet = 0;
	float return_mem = 0;
	char return_mem_str[30];
	struct meminfo mi = mem_stats();

	if (argc < 2) {
		printf("ERR: Not enough arguments\n");
		return 1;
	}

	for (int iter = 0; iter < argc; iter++) {
		if (strcmp(argv[iter], "--percentage-total") == 0 || strcmp(argv[iter], "-p") == 0)
			percentage_total = 1;

		if ((strcmp(argv[iter], "--quiet") == 0 || strcmp(argv[iter], "-q") == 0) && !percentage_total)
			quiet = 1;

		if ((strcmp(argv[iter], "--kilobytes") == 0 || strcmp(argv[iter], "-k") == 0) && !percentage_total)
			byte_type = 1;
		else if ((strcmp(argv[iter], "--megabytes") == 0 || strcmp(argv[iter], "-m") == 0) && !percentage_total)
			byte_type = 2;
		else if ((strcmp(argv[iter], "--gigabytes") == 0 || strcmp(argv[iter], "-g") == 0) && !percentage_total)
			byte_type = 3;

		if (strcmp(argv[iter], "--free") == 0 || strcmp(argv[iter], "-f") == 0)
			return_mem = mi.free;
		else if (strcmp(argv[iter], "--used") == 0 || strcmp(argv[iter], "-u") == 0)
			return_mem = mi.used;
		else if (strcmp(argv[iter], "--total") == 0 || strcmp(argv[iter], "-t") == 0)
			return_mem = mi.total;
	}

	if (!return_mem) {
		printf("ERR: No memory return type specified\n");
		return 1;
	}

	if (byte_type == 1)
		return_mem = return_mem / 1000;
	else if (byte_type == 2)
		return_mem = return_mem / 1000 / 1000;
	else if (byte_type == 3)
		return_mem = return_mem / 1000 / 1000 / 1000;

	if (quiet) {
		printf("%0.0f\n", return_mem);
	} else if (percentage_total) {
		printf("%0.0f%%\n", return_mem / mi.total * 100);
	} else {
		switch (byte_type) {
			case 0:
				sprintf(return_mem_str, "%0.2f B", return_mem);
				break;
			case 1:
				sprintf(return_mem_str, "%0.2f kB", return_mem);
				break;
			case 2:
				sprintf(return_mem_str, "%0.2f MB", return_mem);
				break;
			case 3:
				sprintf(return_mem_str, "%0.2f GB", return_mem);
				break;
		}

		printf("%s\n", return_mem_str);
	}
}

struct meminfo mem_stats() {
	char file_content[20];
	struct meminfo mi;
	int loc = 0;

	FILE *memfile = fopen("/proc/meminfo", "r");

	while (fscanf(memfile, "%s", file_content) != EOF) {
		if (strcmp(file_content, "MemTotal:") == 0) {
			loc = 1;
			continue;
		} else if (strcmp(file_content, "MemFree:") == 0) {
			loc = 2;
			continue;
		} else if (strcmp(file_content, "Buffers:") == 0) {
			loc = 3;
			continue;
		} else if (strcmp(file_content, "Cached:") == 0) {
			loc = 4;
			continue;
		} else if (strcmp(file_content, "Shmem:") == 0) {
			loc = 5;
			continue;
		} else if (strcmp(file_content, "SReclaimable:") == 0) {
			loc = 6;
			continue;
		}

		// /proc/meminfo records values in kB by default, convert them to bytes.
		switch (loc) {
			case 1:
				mi.total = (unsigned long) atoi(file_content) * 1000;
			case 2:
				mi.free = (unsigned long) atoi(file_content) * 1000;
			case 3:
				mi.buffers = (unsigned long) atoi(file_content) * 1000;
			case 4:
				mi.cached = (unsigned long) atoi(file_content) * 1000;
			case 5:
				mi.shmem = (unsigned long) atoi(file_content) * 1000;
			case 6:
				mi.sreclaimable = (unsigned long) atoi(file_content) * 1000;
		}

		loc = 0;
	}

	fclose(memfile);

	mi.used = mi.total + mi.shmem - mi.free - mi.buffers - mi.cached - mi.sreclaimable;

	return mi;
}
