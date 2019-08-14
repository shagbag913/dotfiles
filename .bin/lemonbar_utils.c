#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#if __has_include(<alsa/asoundlib.h>)
#include <alsa/asoundlib.h>

#define SUPPORTS_ASOUNDLIB

/* Audio devices for fetching volume */
const char *card = "pulse";
const char *selem_name = "Master";

struct volume {
	int muted;
	int level;
};

struct volume volume_info();
#endif

struct args {
	unsigned long us;
	void (*function)();
};

struct meminfo {
        unsigned long total;
        unsigned long used;
        unsigned long free;
        unsigned long buffers;
        unsigned long cached;
        unsigned long shmem;
        unsigned long sreclaimable;
};

char *build_slider(int current_place);
void volume_slider();
void battery_status();
void formatted_time();
void network_status();
char *get_pywal_color_value(int color_index, char *fallback_color);
void build_bspwm_status();
int get_charge();
int is_charging();
void *function_thread();
void used_memory_percentage();

static char bat_stat[25];
static char net_stat[22];
static char *bspwm_stat = NULL;
static char time_stat[20];
static char vol_slider[22];
static char used_mem[11];

int main(int argc, char *argv[]) {
	/* lemonbar_wrapper needs pywal color function */
	if (argc == 4) {
		if (!strcmp(argv[1], "--get-pywal-color")) {
			printf("%s\n", get_pywal_color_value(atoi(argv[2]), argv[3]));
			return 0;
		}
	// TODO: handle incorrect arguments / number of arguments
	}

	pthread_t thread1, thread2, thread3, thread4, thread5, thread6;
	struct args function1_args, function2_args, function3_args, function4_args,
		    function5_args, function6_args;

	/* battery status */
	function1_args.us = 5000000;
	function1_args.function = battery_status;
	pthread_create(&thread1, NULL, function_thread, &function1_args);

	/* network status */
	function2_args.us = 10000000;
	function2_args.function = network_status;
	pthread_create(&thread2, NULL, function_thread, &function2_args);

	/* bspwm status */
	function3_args.us = 100000;
	function3_args.function = build_bspwm_status;
	pthread_create(&thread3, NULL, function_thread, &function3_args);

	/* time */
	function4_args.us = 10000000;
	function4_args.function = formatted_time;
	pthread_create(&thread4, NULL, function_thread, &function4_args);

	/* volume slider */
	function5_args.us = 1000000;
	function5_args.function = volume_slider;
	pthread_create(&thread5, NULL, function_thread, &function5_args);

	/* memory usage */
	function6_args.us = 4000000;
	function6_args.function = used_memory_percentage;
	pthread_create(&thread6, NULL, function_thread, &function6_args);

	while (1) {
		/* make sure all variables are not NULL or empty before printing */
		if (!(bspwm_stat && time_stat && net_stat && bat_stat && used_mem))
			continue;

		printf("%{l}%s%{c}%s%{r}%s    |    %s    |    %s    |    %s  \n", bspwm_stat,
				time_stat, used_mem, vol_slider, net_stat, bat_stat);
		fflush(stdout);

		/*
		 * The amount of time we sleep here should be the less than or
		 * equal to the smallest sleep time in the threaded functions.
		 */
		usleep(100000);
	}
}

void *function_thread(void *function_args)
{
	struct args *arguments = function_args;

	while (1) {
		// Fill global variable
		arguments->function();
		usleep(arguments->us);
	}
}

char *get_pywal_color_value(int color_index, char *fallback_color)
{
	FILE *pywal_file;
	static char line[8];
	char pywal_path[100];
	int tmp = 0;

	strcpy(pywal_path, getenv("HOME"));
	strcat(pywal_path, "/.cache/wal/colors");

	pywal_file = fopen(pywal_path, "r");

	if (pywal_file == NULL)
		return fallback_color;

	while (fgets(line, sizeof(line), pywal_file) != NULL) {
		if (tmp == color_index * 2)
			break;
		tmp++;
	}

	fclose(pywal_file);

	// Make sure specified color index is within range
	if (line[0] != '#') {
		printf("Specified color index (%i) not within range, returning fallback color\n",
				color_index);
		strcpy(line, fallback_color);
	}

	return line;
}

char *build_slider(int current_place) {
	static char final_slider[14];
	int net_length = 4, plc;
	int current_small_place = (float) current_place / (float) (100 / net_length);

	/* nuke old final_slider string */
	if (final_slider)
		memset(&final_slider, 0, strlen(final_slider));

	final_slider[0] = '\0';

	for (plc = 0; plc < current_small_place; plc++)
		strcat(final_slider, "—");

	strcat(final_slider, "|");

	for (; plc < net_length; ++plc)
		strcat(final_slider, "—");

	return final_slider;
}

void formatted_time() {
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

	sprintf(time_stat, "%i:%02i %s", hours, time->tm_min, amorpm);
}

void build_bspwm_status() {
	int active_window, bg_window, index = 0;
	char *delim_ptr, *tmp_status, *bspwm_return_status, wm_status[80];
	FILE *bspwm_status;

	bspwm_return_status = malloc(2);

	if (bspwm_return_status == NULL)
		goto failed_alloc;

	strcpy(bspwm_return_status, "\0");

	bspwm_status = popen("bspc wm --get-status", "r");
	fscanf(bspwm_status, "%s", wm_status);
	pclose(bspwm_status);

	delim_ptr = strtok(wm_status, ":");

	while (delim_ptr != NULL) {
		active_window = delim_ptr[0] == 'O' || delim_ptr[0] == 'F';
		bg_window = delim_ptr[0] == 'o';

		if (active_window || bg_window) {
			tmp_status = bspwm_stat;
			bspwm_return_status = realloc(bspwm_return_status,
					strlen(bspwm_return_status) + 10 +
					(index >= 10 ? 33 : 31) + (active_window ? 26 : 0) + 1);

			if (bspwm_return_status == NULL) {
				free(tmp_status);
				goto failed_alloc;
			}

			// Add underline under active window numbers
			if (active_window)
				sprintf(bspwm_return_status, "%s%{+u}%{U%s}", bspwm_return_status,
						get_pywal_color_value(15, "#FFFFFF"));

			sprintf(bspwm_return_status,
					"%s%%{A:bspc desktop -f ^%i:}      %i      %%{A}",
					bspwm_return_status, index, index);

			if (active_window)
				strcat(bspwm_return_status, "%{U-}%{-u}");
		}

		++index;
		delim_ptr = strtok(NULL, ":");
	}

	if (bspwm_stat != NULL)
		free(bspwm_stat);
	bspwm_stat = malloc(strlen(bspwm_return_status)+1);
	if (bspwm_stat == NULL)
		goto failed_alloc;
	strcpy(bspwm_stat, bspwm_return_status);
	free(bspwm_return_status);

	return;

failed_alloc:
	printf("%s: Memory allocation failed!\n", __func__);
}

void battery_status() {
	int charging, charge;
	char *low_color;

	charging = is_charging();
	charge = get_charge();

	if (charge == -1)
		return;

	if (charge >= 90) {
		strcpy(bat_stat, "");
	} else if (charge >= 70) {
		strcpy(bat_stat, "");
	} else if (charge >= 45) {
		strcpy(bat_stat, "");
	} else if (charge >= 15) {
		strcpy(bat_stat, "");
	} else {
		if (!charging) {
			low_color = get_pywal_color_value(1, "#FFA3A3");
			sprintf(bat_stat, "%s%%{F%s}", bat_stat, low_color);
		}
		strcat(bat_stat, "");
	}

	if (bat_stat[0] == '%')
		strcat(bat_stat, "%{F-}");

	sprintf(bat_stat, "%s  %i%%", bat_stat, charge);

	if (charging)
		strcat(bat_stat, "+");
}

int get_charge() {
	FILE *cap_file;
	int charge;

	cap_file = fopen("/sys/class/power_supply/BAT0/capacity", "r");

	if (cap_file == NULL)
		return -1;

	fscanf(cap_file, "%i", &charge);
	fclose(cap_file);

	return charge;
}

int is_charging() {
	FILE *status_file;
	char status[13];

	status_file = fopen("/sys/class/power_supply/BAT0/status", "r");

	if (status_file == NULL)
		return -1;

	fscanf(status_file, "%s", status);
	fclose(status_file);

	return strcmp(status, "Discharging");
}

void network_status() {
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

	if (!strcmp(wstate, "up"))
		strcpy(net_stat,"");
	else if (strcmp(estate, "up"))
		strcpy(net_stat,"%{F#99FFFFFF}%{F-}");
	else if (!strcmp(estate, "up"))
		strcpy(net_stat,"");
}

#ifdef SUPPORTS_ASOUNDLIB
struct volume volume_info() {
	long volume, min, max;
	struct volume volinfo;

	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &volume);
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &volinfo.muted);

	snd_mixer_close(handle);

	volinfo.level = round((float) volume / (float) max * 100);

	// Invert this value
	volinfo.muted = !volinfo.muted;

	return volinfo;
}

void volume_slider() {
	struct volume volinfo = volume_info();

	if (volinfo.muted || volinfo.level == 0)
		sprintf(vol_slider, "  %s", build_slider(volinfo.level));
	else
		sprintf(vol_slider, "  %s", build_slider(volinfo.level));
}
#endif

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

                switch (loc) {
                        case 1:
                                mi.total = (unsigned long)atoi(file_content);
                        case 2:
                                mi.free = (unsigned long)atoi(file_content);
                        case 3:
                                mi.buffers = (unsigned long)atoi(file_content);
                        case 4:
                                mi.cached = (unsigned long)atoi(file_content);
                        case 5:
                                mi.shmem = (unsigned long)atoi(file_content);
                        case 6:
                                mi.sreclaimable = (unsigned long)atoi(file_content);
                }

                loc = 0;
        }

        fclose(memfile);

        mi.used = mi.total + mi.shmem - mi.free - mi.buffers - mi.cached - mi.sreclaimable;

        return mi;
}

void used_memory_percentage()
{
	struct meminfo mi;
	int used_mem_dec;

	mi = mem_stats();

	used_mem_dec = 100 * ((float)mi.used / (float)mi.total);

	sprintf(used_mem, "  %i%%", used_mem_dec);
}
