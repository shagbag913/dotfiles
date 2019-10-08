#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <libnotify/notify.h>

#include "lemonbar_utils.h"

#ifdef SUPPORTS_ASOUNDLIB
#include <alsa/asoundlib.h>
#endif

int main(int argc, char *argv[]) {
	pthread_t thread[6];
	struct args function_args[6];
	unsigned short iter;

	for (iter = 1; iter < argc; iter++) {
		if (!strcmp(argv[iter], "--get-pywal-color")) {
			char pywal_fallback[] = "#000000";
			unsigned short pywal_index;

			if (argv[iter + 1] != NULL) {
				pywal_index = atoi(argv[iter + 1]);
			} else {
				printf("Color index must be specified for pywal function, aborting\n");
				return 1;
			}
			if (argv[iter + 2] != NULL)
				strncpy(pywal_fallback, argv[iter + 2], sizeof(pywal_fallback));
			printf("%s\n", get_pywal_color_value(pywal_index, pywal_fallback));
			return 0;
		}
	}

	/* Set common color values */
	strcpy(off_glyph_color, get_pywal_color_value(8, "#303030"));
	strcpy(normal_glyph_color, get_pywal_color_value(15, "#FFFFFF"));
	strcpy(low_battery_glyph_color, get_pywal_color_value(1, "#FFA3A3"));

	function_exec_count = 0;

	/* Battery status */
	battery_status_sleep_time_ptr = &function_args[0].us;
	function_args[0].us = 0;
	function_args[0].function = battery_status;
	pthread_create(&thread[0], NULL, function_thread, &function_args[0]);

	/* Network status */
	function_args[1].us = USEC_TO_SEC(5);
	function_args[1].function = network_status;
	pthread_create(&thread[1], NULL, function_thread, &function_args[1]);

	/* BSPWM status */
	function_args[2].us = USEC_TO_SEC(.15);
	function_args[2].function = build_bspwm_status;
	pthread_create(&thread[2], NULL, function_thread, &function_args[2]);

	/* Time */
	function_args[3].us = USEC_TO_SEC(5);
	function_args[3].function = formatted_time;
	pthread_create(&thread[3], NULL, function_thread, &function_args[3]);

	/* Volume slider */
	function_args[4].us = USEC_TO_SEC(1);
	function_args[4].function = volume_slider;
	pthread_create(&thread[4], NULL, function_thread, &function_args[4]);

	/* Memory usage */
	function_args[5].us = USEC_TO_SEC(2);
	function_args[5].function = used_memory_percentage;
	pthread_create(&thread[5], NULL, function_thread, &function_args[5]);

	while (1) {
		/* Left of the bar */
		if (bspwm_stat != NULL)
			printf("%%{l}%s", bspwm_stat);

		/* Center of the bar */
		if (strlen(time_stat))
			printf("%%{c}%s", time_stat);

		/* Right of the bar */
		if (strlen(used_mem) && strlen(vol_slider) && strlen(bat_stat))
			printf("%%{r}%s    |    %s    |    %s    |    %s    ",
					used_mem, vol_slider, net_stat, bat_stat);

		printf("\n");

		fflush(stdout);

		/* Only honor shortest_sleep value if all functions have had a chance to set it */
		if (function_exec_count == sizeof(function_args) / sizeof(function_args[0]))
			usleep(shortest_sleep);
		else
			usleep(100000);
	}
}

void *function_thread(void *function_args)
{
	const struct args *arguments = function_args;

	function_exec_count++;

	/* Keep track of shortest sleep time */
	if ((!shortest_sleep || arguments->us < shortest_sleep) && arguments->us)
		shortest_sleep = arguments->us;

	while (1) {
		/* Fill global variable */
		if (!arguments->function())
			/*
			 * If function fails, sleep for a longer period of time.
			 * This was inspired by build_bspwm_status taking up loads
			 * of CPU time because it was failing over and over due to
			 * display being off.
			 */
			usleep(FAIL_SLEEP_TIME);
		else
			usleep(arguments->us);
	}
}

char *get_pywal_color_value(int color_index, char *fallback_color)
{
	FILE *pywal_file;
	static char line[COLOR_HEX_LENGTH];
	char pywal_path[100];
	const char *suffix_pywal_path = "/.cache/wal/colors";
	int tmp = 0;

	strncpy(pywal_path, HOME, sizeof(pywal_path) - strlen(suffix_pywal_path) - 1);
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
	const unsigned short net_length = 4;
	unsigned short plc, current_small_place = (float)current_place / (float)(100 / net_length);

	memset(&final_slider, 0, strlen(final_slider));

	final_slider[0] = '\0';

	for (plc = 0; plc < current_small_place; plc++)
		strcat(final_slider, "—");

	strcat(final_slider, "|");

	for (; plc < net_length; ++plc)
		strcat(final_slider, "—");

	return final_slider;
}

struct tm *get_time()
{
	time_t t = time(NULL);
	struct tm *time = localtime(&t);

	return time;
}

void libnotify_notify(char *message)
{
	NotifyNotification *notification = notify_notification_new(message, "lemonbar", NULL);

	notify_init("lemonbar");
	notify_notification_set_timeout(notification, 2000);
	notify_notification_show(notification, NULL);
}

unsigned short formatted_time() {
	char amorpm[3];
	const struct tm *time = get_time();
	unsigned short hours = time->tm_hour;

	if (hours < 12) {
		strcpy(amorpm, "AM");
	} else if (hours >= 12) {
		hours -= 12;
		strcpy(amorpm, "PM");
	}

	if (hours == 0)
		hours = 12;

	sprintf(time_stat, "%i:%02i %s", hours, time->tm_min, amorpm);

	return 1;
}

unsigned short build_bspwm_status() {
	unsigned short ret, bspwm_status_alloc_size = 0, index = 0;
	char *delim_ptr, *tmp_status, wm_status[80];
	FILE *bspwm_status = popen("bspc wm --get-status", "r");

	fscanf(bspwm_status, "%s", wm_status);
	ret = pclose(bspwm_status);

	if (ret) {
		printf("Command `bspm wm --get-status` failed\n");
		return 0;
	}

	if (wm_status_test != NULL) {
		/* Don't proceed if bspwm_status is the same as the last check */
		if (!strcmp(wm_status, wm_status_test))
			return 1;
		free(wm_status_test);
	}

	wm_status_test = malloc(strlen(wm_status) + 1);
	if (wm_status_test == NULL)
		goto failed_alloc;

	strcpy(wm_status_test, wm_status);

	if (bspwm_stat != NULL)
		free(bspwm_stat);

	bspwm_stat = malloc(2);
	if (bspwm_stat == NULL)
		goto failed_alloc;

	strcpy(bspwm_stat, "\0");

	delim_ptr = strtok(wm_status, ":");

	while (delim_ptr != NULL) {
		unsigned short active_window, bg_window;

		/* Break after completing the first monitor sequence */
		if (*delim_ptr == 'G')
			break;

		active_window = delim_ptr[0] == 'O' || delim_ptr[0] == 'F';
		bg_window = delim_ptr[0] == 'o';

		if (active_window || bg_window) {
			bspwm_status_alloc_size += 10 + (index >= 10 ? 33 : 31) +
				(active_window ? 26 : 0) + 1;
			tmp_status = bspwm_stat;
			bspwm_stat = realloc(bspwm_stat, bspwm_status_alloc_size);

			if (!bspwm_stat) {
				if (!tmp_status)
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

	return 1;

failed_alloc:
	printf("%s: Memory allocation failed!\n", __func__);

	return 0;
}

unsigned short battery_status() {
	unsigned short battery_glyph;
	const char *battery_glyphs[] = {
		"",
		"",
		"",
		"",
		""
	};
	const short charging = is_charging();
	const short charge = get_charge();

	memset(&bat_stat, 0, strlen(bat_stat));

	if (charge == -1)
		return 0;

	battery_glyph = round((float)charge / 25);

	if (!charging) {
		if (battery_glyph < 1)
			sprintf(bat_stat, "%s%%{F%s}", bat_stat, low_battery_glyph_color);

		/* Slower sleep time */
		*battery_status_sleep_time_ptr = USEC_TO_SEC(5);
	} else {
		static unsigned short charging_battery_glyph = 6;

		if (charging_battery_glyph < sizeof(battery_glyphs)/sizeof(battery_glyphs[0]) - 1)
			charging_battery_glyph++;
		else
			charging_battery_glyph = battery_glyph;
		battery_glyph = charging_battery_glyph;

		/* Faster sleep time so charging animation isn't slow */
		*battery_status_sleep_time_ptr = USEC_TO_SEC(1);
	}

	strcat(bat_stat, battery_glyphs[battery_glyph]);

	if (bat_stat[0] == '%')
		strcat(bat_stat, "%{F-}");

	sprintf(bat_stat, "%s  %i%%", bat_stat, charge);

	if (charging)
		strcat(bat_stat, "+");

	return 1;
}

short get_charge() {
	unsigned short charge;
	FILE *cap_file = fopen("/sys/class/power_supply/BAT0/capacity", "r");

	if (cap_file == NULL)
		return -1;

	fscanf(cap_file, "%hu", &charge);
	fclose(cap_file);

	return charge;
}

short is_charging() {
	char status[12];
	FILE *status_file = fopen("/sys/class/power_supply/BAT0/status", "r");

	if (status_file == NULL)
		return -1;

	fscanf(status_file, "%s", status);
	fclose(status_file);

	return strcmp(status, "Discharging");
}

unsigned short network_status() {
	FILE *wifi_operstate = fopen("/sys/class/net/wlp2s0/operstate", "r");
	FILE *ethernet_operstate = fopen("/sys/class/net/enp3s0/operstate", "r");

	if (ethernet_operstate != NULL) {
		char estate[5];

		fscanf(ethernet_operstate, "%s", estate);
		fclose(ethernet_operstate);

		if (!strcmp(estate, "up")) {
			strcpy(net_stat,"");
			return 1;
		}
	}

	if (wifi_operstate != NULL) {
		char wstate[5];

		fscanf(wifi_operstate, "%s", wstate);
		fclose(wifi_operstate);

		if (!strcmp(wstate, "up"))
			strcpy(net_stat,"");
		else
			sprintf(net_stat,"%%{F%s}%%{F-}", off_glyph_color);

		return 1;
	}

	return 0;
}

#ifdef SUPPORTS_ASOUNDLIB
struct volume volume_info() {
	long volume, min, max;
	struct volume volinfo;
	const char *card = "pulse", *selem_name = "Master";

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

	volinfo.level = round((float)volume / (float)max * 100);
	volinfo.muted = !volinfo.muted;

	return volinfo;
}

unsigned short volume_slider() {
	struct volume volinfo = volume_info();

	if (volinfo.muted || volinfo.level == 0)
		sprintf(vol_slider, "%%{F%s} %%{F-} %s", off_glyph_color,
				build_slider(volinfo.level));
	else
		sprintf(vol_slider, "  %s", build_slider(volinfo.level));

	return 1;
}
#endif /* SUPPORTS_ASOUNDLIB */

struct meminfo mem_stats()
{
	char file_content[20];
	unsigned short iter = 0;
	struct meminfo mi;
	FILE *memfile = fopen("/proc/meminfo", "r");
	unsigned int *mem_value[] = {
		&mi.total,
		&mi.free,
		&mi.buffers,
		&mi.cached,
		&mi.shmem,
		&mi.sreclaimable
	};

	while (fscanf(memfile, "%s", file_content) != EOF) {
		if (!strcmp(file_content, "MemTotal:"))
			iter = 1;
		else if (!strcmp(file_content, "MemFree:"))
			iter = 2;
		else if (!strcmp(file_content, "Buffers:"))
			iter = 3;
		else if (!strcmp(file_content, "Cached:"))
			iter = 4;
		else if (!strcmp(file_content, "Shmem:"))
			iter = 5;
		else if (!strcmp(file_content, "SReclaimable:"))
			iter = 6;
		else
			continue;

		if (iter) {
			fscanf(memfile, "%s", file_content);
			*mem_value[iter - 1] = atoi(file_content);
		}

		iter = 0;
	}

	fclose(memfile);

	mi.used = mi.total + mi.shmem - mi.free - mi.buffers - mi.cached - mi.sreclaimable;

	return mi;
}

unsigned short used_memory_percentage()
{
	struct meminfo mi = mem_stats();
	int used_mem_dec = 100 * ((float)mi.used / (float)mi.total);

	sprintf(used_mem, "  %i%%", used_mem_dec);

	return 1;
}
