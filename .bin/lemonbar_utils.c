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

#define USEC_TO_SEC(x) (x * 1000000)
#define COLOR_HEX_LENGTH 8

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

char bat_stat[25];
char net_stat[20];
char *bspwm_stat = NULL;
char time_stat[20];
char vol_slider[39];
char used_mem[11];

/* Used to keep track of last `bspc wm --get-status` output */
char wm_status_test[80];

/* Used to track shortest sleep() time */
unsigned long shortest_sleep;

/* Commonly used pywal color values */
char off_glyph_color[COLOR_HEX_LENGTH];
char normal_glyph_color[COLOR_HEX_LENGTH];
char low_battery_glyph_color[COLOR_HEX_LENGTH];

int main(int argc, char *argv[]) {
	pthread_t thread1, thread2, thread3, thread4, thread5, thread6;
	struct args function1_args, function2_args, function3_args, function4_args,
		    function5_args, function6_args;

	/* lemonbar_wrapper needs pywal color function */
	if (argc == 4) {
		if (!strcmp(argv[1], "--get-pywal-color")) {
			printf("%s\n", get_pywal_color_value(atoi(argv[2]), argv[3]));
			return 0;
		}
	/* TODO: handle incorrect arguments / number of arguments */
	}

	/* Set common color values */
	strcpy(off_glyph_color, get_pywal_color_value(8, "#303030"));
	strcpy(normal_glyph_color, get_pywal_color_value(15, "#FFFFFF"));
	strcpy(low_battery_glyph_color, get_pywal_color_value(1, "#FFA3A3"));

	/* Battery status */
	function1_args.us = USEC_TO_SEC(5);
	function1_args.function = battery_status;
	pthread_create(&thread1, NULL, function_thread, &function1_args);

	/* Network status */
	function2_args.us = USEC_TO_SEC(1);
	function2_args.function = network_status;
	pthread_create(&thread2, NULL, function_thread, &function2_args);

	/* BSPWM status */
	function3_args.us = 100000;
	function3_args.function = build_bspwm_status;
	pthread_create(&thread3, NULL, function_thread, &function3_args);

	/* Time */
	function4_args.us = USEC_TO_SEC(5);
	function4_args.function = formatted_time;
	pthread_create(&thread4, NULL, function_thread, &function4_args);

	/* Volume slider */
	function5_args.us = USEC_TO_SEC(1);
	function5_args.function = volume_slider;
	pthread_create(&thread5, NULL, function_thread, &function5_args);

	/* Memory usage */
	function6_args.us = USEC_TO_SEC(2);
	function6_args.function = used_memory_percentage;
	pthread_create(&thread6, NULL, function_thread, &function6_args);

	while (1) {
		/* Make sure all variables are set before printing anything */
		if (bspwm_stat == NULL || !(strlen(time_stat) && strlen(net_stat) &&
					strlen(bat_stat) && strlen(used_mem) && strlen(vol_slider)))
			continue;

		printf("%%{l}%s%%{c}%s%%{r}%s    |    %s    |    %s    |    %s    \n", bspwm_stat,
				time_stat, used_mem, vol_slider, net_stat, bat_stat);
		fflush(stdout);
		usleep(shortest_sleep);
	}
}

void *function_thread(void *function_args)
{
	struct args *arguments = function_args;

	/* Keep track of shortest sleep time */
	if (!shortest_sleep || arguments->us < shortest_sleep)
		shortest_sleep = arguments->us;

	while (1) {
		/* Fill global variable */
		arguments->function();
		usleep(arguments->us);
	}
}

char *get_pywal_color_value(int color_index, char *fallback_color)
{
	FILE *pywal_file;
	static char line[8];
	char pywal_path[100];
	const char *suffix_pywal_path = "/.cache/wal/colors";
	int tmp = 0;

	strncpy(pywal_path, getenv("HOME"), sizeof(pywal_path) - strlen(suffix_pywal_path) - 1);
	strcat(pywal_path, suffix_pywal_path);

	pywal_file = fopen(pywal_path, "r");

	if (pywal_file == NULL)
		return fallback_color;

	while (fgets(line, sizeof(line), pywal_file) != NULL) {
		if (tmp == color_index * 2)
			break;
		tmp++;
	}

	fclose(pywal_file);

	/* Make sure specified color index is within range */
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
	int active_window, bg_window, index = 0, bspwm_status_alloc_size = 0;
	char *delim_ptr, *tmp_status, wm_status[80];
	FILE *bspwm_status;

	bspwm_status = popen("bspc wm --get-status", "r");
	fscanf(bspwm_status, "%s", wm_status);
	pclose(bspwm_status);

	/* Don't proceed if bspwm_status is the same as the last check */
	if (!strcmp(wm_status, wm_status_test))
		return;

	strcpy(wm_status_test, wm_status);

	if (bspwm_stat != NULL)
		free(bspwm_stat);

	bspwm_stat = malloc(2);

	if (bspwm_stat == NULL)
		goto failed_alloc;

	strcpy(bspwm_stat, "\0");

	delim_ptr = strtok(wm_status, ":");

	while (delim_ptr != NULL) {
		active_window = delim_ptr[0] == 'O' || delim_ptr[0] == 'F';
		bg_window = delim_ptr[0] == 'o';

		if (active_window || bg_window) {
			bspwm_status_alloc_size += 10 + (index >= 10 ? 33 : 31) +
				(active_window ? 26 : 0) + 1;
			tmp_status = bspwm_stat;
			bspwm_stat = realloc(bspwm_stat, bspwm_status_alloc_size);

			if (bspwm_stat == NULL) {
				free(tmp_status);
				goto failed_alloc;
			}

			/* Add underline under active window numbers */
			if (active_window)
				sprintf(bspwm_stat, "%s%%{+u}%%{U%s}", bspwm_stat,
						normal_glyph_color);

			sprintf(bspwm_stat,
					"%s%%{A:bspc desktop -f ^%i:}      %i      %%{A}",
					bspwm_stat, index, index);

			if (active_window)
				strcat(bspwm_stat, "%{U-}%{-u}");
		}

		++index;
		delim_ptr = strtok(NULL, ":");
	}

	return;

failed_alloc:
	printf("%s: Memory allocation failed!\n", __func__);
}

void battery_status() {
	int charging, charge;

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
		if (!charging)
			sprintf(bat_stat, "%s%%{F%s}", bat_stat, low_battery_glyph_color);
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
		sprintf(net_stat,"%%{F%s}%%{F-}", off_glyph_color);
	else if (!strcmp(estate, "up"))
		strcpy(net_stat,"");
}

#ifdef SUPPORTS_ASOUNDLIB
struct volume volume_info() {
	long volume, min, max;
	struct volume volinfo;
	const char *card = "pulse";
	const char *selem_name = "Master";

	snd_mixer_elem_t *elem;

	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);

	elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &volume);
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &volinfo.muted);

	snd_mixer_close(handle);

	volinfo.level = round((float) volume / (float) max * 100);
	volinfo.muted = !volinfo.muted;

	return volinfo;
}

void volume_slider() {
	struct volume volinfo = volume_info();

	if (volinfo.muted || volinfo.level == 0)
		sprintf(vol_slider, "%%{F%s} %%{F-} %s", off_glyph_color,
				build_slider(volinfo.level));
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
