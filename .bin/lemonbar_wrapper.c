/*
 * Lemonbar wrapper.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct meminfo {
	int total;
	int used;
	int percentage_used;
	int free;
	int buffers;
	int cached;
	int shmem;
	int sreclaimable;
};

int get_charge();
int is_charging();
int get_brightness();
void mem_stats(struct meminfo *mi);
char *build_bspwm_status();
char *get_formatted_time();

int main(int argc, char *argv[]) {
	/*
	 * Handle command line arguments.
	 *
	 * Don't allow more or less than one argument.
	 */
	if (argc > 2) {
		printf("Too many arguments specified. Only specify one argument.\n");
		return 1;
	} else if (argc < 2) {
		printf("Not enough arguments specified. You must specify one argument.\n");
		return 1;
	}

	if (strcmp(argv[1], "--used-memory-percentage") == 0) {
		struct meminfo mi;
		mem_stats(&mi);
		printf("used-memory-percentage  %i%%\n", mi.percentage_used);
	} else if (strcmp(argv[1], "--intel-brightness") == 0) {
		printf("%i\n", get_brightness());
	} else if (strcmp(argv[1], "--time") == 0) {
		printf("time%s\n", get_formatted_time());
	} else if (strcmp(argv[1], "--bspwm-status") == 0) {
		printf("bspwm-status%s\n", build_bspwm_status());
	} else if (strcmp(argv[1], "--charge") == 0) {
		printf("%i\n", get_charge());
	} else if (strcmp(argv[1], "--is-charging") == 0) {
		printf("%i\n", is_charging());
	} else {
		printf("Unknown argument: %s.\n", argv[1]);
		return 1;
	}
}

/*
 * Returns a char array with the 12 hour time in the following format:
 * HH:MM AM/PM
 */
char *get_formatted_time() {
	static char formatted_time[20];
	char amorpm[3];

	time_t t = time(NULL);
	struct tm *time = localtime(&t);
	int hours = time->tm_hour;

	if (hours < 12) {
		strcpy(amorpm, "AM");
	} else if (hours >= 12) {
		hours -= 12;
		strcpy(amorpm, "PM");
	}

	if (hours == 0)
		hours = 12;

	sprintf(formatted_time, "%i:%02i %s", hours, time->tm_min, amorpm);

	return formatted_time;
}

/*
 * Retrieves BSPWM active/inactive desktops and sorts them into glyphs.
 */
char *build_bspwm_status() {
	int active_window, bg_window, rs_empty, index = 0;
	char *delim_ptr, wm_status[90], glyph[] = "";
	char grey_glyph[] = "%{F#a5a5a5}%{F-}", space[] = "    ";
	static char return_window_status[545] = "\0", *click_command;
	FILE *bspwm_status;

	bspwm_status = popen("bspc wm --get-status", "r");
	fscanf(bspwm_status, "%s", wm_status);
	pclose(bspwm_status);
	delim_ptr = strtok(wm_status, ":");

	while (delim_ptr != NULL) {
		active_window = delim_ptr[0] == 'O' || delim_ptr[0] == 'F';
		bg_window = delim_ptr[0] == 'o';

		if (active_window || bg_window) {
			click_command = malloc(30);
			sprintf(click_command, "%%{A:bspc desktop -f ^%i:}", index);
			strcat(return_window_status, click_command);
			free(click_command);
		}

		if (active_window)
			strcat(return_window_status, glyph);
		else if (bg_window)
			strcat(return_window_status, grey_glyph);

		if (active_window || bg_window) {
			strcat(return_window_status, space);
			strcat(return_window_status, "%{A}");
		}

		++index;
		delim_ptr = strtok(NULL, ":");
	}

	return return_window_status;
}

/*
 * Retrieves current battery charge in a range from 0 - 100.
 * -1 is returned if a battery is not available.
 */
int get_charge() {
	FILE *cap_file;
	int charge;

	cap_file = fopen("/sys/class/power_supply/BAT0/capacity", "r");

	/* return -1 if the file couldn't be opened (battery not available) */
	if (cap_file == NULL) return -1;

	fscanf(cap_file, "%i", &charge);
	fclose(cap_file);

	return charge;
}

/*
 * Returns 0 if the device is charging, and 1 if it's not charging.
 * -1 is returned if a battery is not available.
 */
int is_charging() {
	FILE *status_file;
	char status[13];

	status_file = fopen("/sys/class/power_supply/BAT0/status", "r");

	/* return -1 if the file couldn't be opened (battery not available) */
	if (status_file == NULL) return -1;

	fscanf(status_file, "%s", status);
	fclose(status_file);

	return strcmp(status, "Charging");
}

/*
 * Returns brightness for Intel drivers in a range from 0-100.
 */
int get_brightness() {
	FILE *brightness_file, *max_brightness_file;
	int brightness, max_brightness, percent_brightness;

	brightness_file = fopen("/sys/class/backlight/intel_backlight/brightness", "r");
	max_brightness_file = fopen("/sys/class/backlight/intel_backlight/max_brightness", "r");

	/* return -1 if the file couldn't be opened (Intel backlight not available) */
	if (brightness_file == NULL || max_brightness_file == NULL) return -1;

	fscanf(brightness_file, "%i", &brightness);
	fscanf(max_brightness_file, "%i", &max_brightness);

	percent_brightness = round((float) brightness / (float) max_brightness * 100);

	return percent_brightness;
}

/*
 * Fills meminfo struct members.
 */
void mem_stats(struct meminfo *mi) {
	FILE *meminfo;
	char file_content[100];
	int loc;

	meminfo = fopen("/proc/meminfo", "r");

	while (fscanf(meminfo, "%s", file_content) != EOF) {
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

		switch (loc) {
			case 1:
				mi->total = atoi(file_content);
			case 2:
				mi->free = atoi(file_content);
			case 3:
				mi->buffers = atoi(file_content);
			case 4:
				mi->cached = atoi(file_content);
			case 5:
				mi->shmem = atoi(file_content);
			case 6:
				mi->sreclaimable = atoi(file_content);
		}

		loc = 0;
	}

	mi->used = mi->total + mi->shmem - mi->free - mi->buffers - mi->cached - mi->sreclaimable;
	mi->percentage_used = (float) mi->used / (float) mi->total * 100;
}
