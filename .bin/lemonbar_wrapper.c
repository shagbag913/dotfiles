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
char *get_brightness_slider();
void mem_stats(struct meminfo *mi);
char *build_bspwm_status();
char *get_formatted_time();
char *build_slider(int current_place);
char *get_network_status();

int main(int argc, char *argv[]) {
	int max_args = 2, min_args = 2;

	/*
	 * Handle command line arguments.
	 */
	if (argc >= min_args) {
		if (strcmp(argv[1], "--build-slider") == 0) {
			++max_args;
			++min_args;
		}

		if (argc < min_args) {
			printf("Not enough arguments specified. You must specify one argument.\n");
			return 1;
		} else if (argc > max_args) {
			printf("Too many arguments specified. Only specify one argument.\n");
			return 1;
		}
	} else {
		printf("Not enough arguments specified. You must specify one argument.\n");
		return 1;
	}


	if (strcmp(argv[1], "--used-memory-percentage") == 0) {
		struct meminfo mi;
		mem_stats(&mi);
		printf("used-memory-percentage  %i%%\n", mi.percentage_used);
	} else if (strcmp(argv[1], "--time") == 0) {
		printf("time%s\n", get_formatted_time());
	} else if (strcmp(argv[1], "--bspwm-status") == 0) {
		char *bspwm_status = build_bspwm_status();
		printf("bspwm-status%s\n", bspwm_status);
		free(bspwm_status);
	} else if (strcmp(argv[1], "--charge") == 0) {
		printf("%i\n", get_charge());
	} else if (strcmp(argv[1], "--is-charging") == 0) {
		printf("%i\n", is_charging());
	} else if (strcmp(argv[1], "--build-slider") == 0) {
		printf("%s\n", build_slider(atoi(argv[2])));
	} else if (strcmp(argv[1], "--brightness-slider") == 0) {
		printf("brightness-slider  %s\n", get_brightness_slider());
	} else if (strcmp(argv[1], "--network-status") == 0) {
		printf("network-status%s\n", get_network_status());
	} else {
		printf("Unknown argument: %s.\n", argv[1]);
		return 1;
	}
}

/*
 * Builds a visual slider representing something on a scale of 100.
 */
char *build_slider(int current_place) {
	static char final_slider[6] = "\0";
	int net_length = 5, plc;
	int current_small_place = (float) current_place / (float) (100 / net_length);

	for (plc = 0; plc < current_small_place; plc++)
		strcat(final_slider, "—");
	strcat(final_slider, "|");
	for (; plc < net_length; ++plc)
		strcat(final_slider, "—");

	return final_slider;
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
	char *return_window_status, click_command[27];
	FILE *bspwm_status;

	return_window_status = malloc(2);
	if (return_window_status == NULL)
		goto failed_alloc;

	strcpy(return_window_status, "\0");

	bspwm_status = popen("bspc wm --get-status", "r");
	fscanf(bspwm_status, "%s", wm_status);
	pclose(bspwm_status);
	delim_ptr = strtok(wm_status, ":");

	while (delim_ptr != NULL) {
		active_window = delim_ptr[0] == 'O' || delim_ptr[0] == 'F';
		bg_window = delim_ptr[0] == 'o';

		if (active_window || bg_window) {
			return_window_status = realloc(return_window_status,
					strlen(return_window_status) + 28);
			if (return_window_status == NULL)
				goto failed_alloc;

			sprintf(click_command, "%%{A:bspc desktop -f ^%i:}", index);
			strcat(return_window_status, click_command);

			if (active_window) {
				return_window_status = realloc(return_window_status,
						strlen(return_window_status) + strlen(glyph) + 2);
				if (return_window_status == NULL)
					goto failed_alloc;

				strcat(return_window_status, glyph);
			} else if (bg_window) {
				return_window_status = realloc(return_window_status,
						strlen(return_window_status) + strlen(grey_glyph) + 2);
				if (return_window_status == NULL)
					goto failed_alloc;

				strcat(return_window_status, grey_glyph);
			}

			return_window_status = realloc(return_window_status,
					strlen(return_window_status) + strlen(space) + 7);
			if (return_window_status == NULL)
				goto failed_alloc;

			strcat(return_window_status, space);
			strcat(return_window_status, "%{A}");
		}

		++index;
		delim_ptr = strtok(NULL, ":");
	}

	return return_window_status;

failed_alloc:
	printf("Failed to allocate memory for return_window_status.\n");
	free(return_window_status);
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
	if (cap_file == NULL) {
		fclose(cap_file);
		return -1;
	}

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
	if (status_file == NULL) {
		fclose(status_file);
		return -1;
	}

	fscanf(status_file, "%s", status);
	fclose(status_file);

	return strcmp(status, "Charging");
}

/*
 * Returns a brightness slider for Intel drivers.
 */
char *get_brightness_slider() {
	FILE *brightness_file, *max_brightness_file;
	int brightness, max_brightness, percent_brightness;
	static char slider[5];

	brightness_file = fopen("/sys/class/backlight/intel_backlight/brightness", "r");
	max_brightness_file = fopen("/sys/class/backlight/intel_backlight/max_brightness", "r");

	/* return empty array if the file couldn't be opened (Intel backlight not available) */
	if (brightness_file == NULL || max_brightness_file == NULL) {
		fclose(brightness_file);
		fclose(max_brightness_file);
		return "";
	}

	fscanf(brightness_file, "%i", &brightness);
	fscanf(max_brightness_file, "%i", &max_brightness);
	fclose(brightness_file);
	fclose(max_brightness_file);

	percent_brightness = round((float) brightness / (float) max_brightness * 100);
	strcpy(slider, build_slider(percent_brightness));

	return slider;
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

	fclose(meminfo);

	mi->used = mi->total + mi->shmem - mi->free - mi->buffers - mi->cached - mi->sreclaimable;
	mi->percentage_used = (float) mi->used / (float) mi->total * 100;
}

/* Returns network status like so:
 * - WiFi available and connected: white WiFi glyph
 * - WiFi available but not connected and no ethernet available: greyed out WiFi glyph
 * - WiFi available but not connected but ethernet available: wired connection glyph
 * - Connected to Ethernet: wired connection glyph
 * - Else: no output
 */
char *get_network_status() {
	FILE *wifi_operstate, *ethernet_operstate;
	static char wstate[5], estate[5];

	wifi_operstate = fopen("/sys/class/net/wlp2s0/operstate", "r");
	ethernet_operstate = fopen("/sys/class/net/enp3s0/operstate", "r");

	if (wifi_operstate != NULL) {
		fscanf(wifi_operstate, "%s", wstate);
		fclose(wifi_operstate);
        }

        if (ethernet_operstate != NULL) {
		fscanf(ethernet_operstate, "%s", estate);
		fclose(ethernet_operstate);
	}

	if (strcmp(wstate, "up") == 0)
		return "";
	else if (strcmp(estate, "up") != 0)
		return "%{F#99FFFFFF}%{F-}";
	else if (strcmp(estate, "up") == 0)
		return "";

	return "";
}
