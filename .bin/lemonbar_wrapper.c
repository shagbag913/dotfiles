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

enum {
	UNAVAILABLE = -1
};

char *build_slider(int current_place);
char *get_battery_glyph();
char *get_brightness_slider();
char *get_formatted_time();
char *get_network_status();
int get_charge();
int is_charging();
void *build_bspwm_status();
void mem_stats(struct meminfo *mi);

int main(int argc, char *argv[]) {
	int max_args = 2, min_args = 2;

	/*
	 * Handle arguments.
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

		// Only do these if build_bspwm_status doesn't return NULL, in case memory allocation fails.
                if (bspwm_status != NULL) {
			printf("bspwm-status%s\n", bspwm_status);
			free(bspwm_status);
                }

	} else if (strcmp(argv[1], "--charge-glyph") == 0) {
		printf("charge-glyph%s\n", get_battery_glyph());
	} else if (strcmp(argv[1], "--build-slider") == 0) {
		printf("%s\n", build_slider(atoi(argv[2])));
	} else if (strcmp(argv[1], "--brightness-slider") == 0) {
		printf("brightness-slider%s\n", get_brightness_slider());
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
	static char final_slider[17] = "\0";
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
void *build_bspwm_status() {
	int active_window, bg_window, index = 0;
	char *delim_ptr, *return_window_status, *tmp_status, wm_status[80];
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
			tmp_status = return_window_status;
			return_window_status = realloc(return_window_status,
					strlen(return_window_status) + 10 +
					(index >= 10 ? 33 : 31) + (active_window ? 26 : 0) + 1);

			if (return_window_status == NULL) {
				free(tmp_status);
				goto failed_alloc;
			}

			// Add underline under active window numbers
			if (active_window)
				strcat(return_window_status, "%{+u}%{U#ffffff}");

			// Temporary string identical to return_window_status for sprintf
			tmp_status = malloc(strlen(return_window_status) + 1);

			if (tmp_status == NULL)
				goto failed_alloc;

			strcpy(tmp_status, return_window_status);
			sprintf(return_window_status, "%s%%{A:bspc desktop -f ^%i:}      %i", tmp_status, index, index);
			free(tmp_status);

			strcat(return_window_status, "      %{A}");

			if (active_window)
				strcat(return_window_status, "%{U-}%{-u}");
		}

		++index;
		delim_ptr = strtok(NULL, ":");
	}

	return return_window_status;

failed_alloc:
	printf("%s: Memory allocation failed!\n", __func__);
	return NULL;
}

/*
 * Returns a glyph showing current battery charge status.
 */
char *get_battery_glyph() {
	int charging = is_charging(), charge = get_charge();
	static char glyph[20] = "\0";

	if (charge == -1)
		return "";

	if (charging == 0)
		strcat(glyph, "%{F#A1FFA2}");

	if (charge >= 90) {
		strcat(glyph, "");
	} else if (charge >= 70) {
		strcat(glyph, "");
	} else if (charge >= 45) {
		strcat(glyph, "");
	} else if (charge >= 20) {
		strcat(glyph, "");
	} else {
		if (charging != 0) {
			if (charge >= 10)
				strcat(glyph, "%{F#FF7575}");
			else
				strcat(glyph, "%{F#FF3838}");
		}

		strcat(glyph, "");
	}

	if (glyph[0] == '%')
		strcat(glyph, "%{F-}");

	return glyph;
}

/*
 * Retrieves current battery charge in a range from 0 - 100.
 * -1 is returned if a battery is not available.
 */
int get_charge() {
	FILE *cap_file;
	int charge;

	cap_file = fopen("/sys/class/power_supply/BAT0/capacity", "r");

	if (cap_file == NULL)
		return UNAVAILABLE;

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

	if (status_file == NULL)
		return UNAVAILABLE;

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
	static char slider[21] = " ";

	brightness_file = fopen("/sys/class/backlight/intel_backlight/brightness", "r");
	max_brightness_file = fopen("/sys/class/backlight/intel_backlight/max_brightness", "r");

	if (brightness_file == NULL || max_brightness_file == NULL)
		return "";

	fscanf(brightness_file, "%i", &brightness);
	fscanf(max_brightness_file, "%i", &max_brightness);
	fclose(brightness_file);
	fclose(max_brightness_file);

	percent_brightness = round((float) brightness / (float) max_brightness * 100);
	strcat(slider, build_slider(percent_brightness));

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
