/*
 * Lemonbar wrapper.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

enum {UNAVAILABLE = -1};

char *build_slider(int current_place);
char *build_volume_slider();
char *battery_status();
char *formatted_time();
char *network_status();
char *get_pywal_color_value(int color_index, char *fallback_color);
void *build_bspwm_status();
int get_charge();
int is_charging();

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Not enough arguments specified. You must specify one argument.\n");
		return 1;
	}

	if (!strcmp(argv[1], "--time")) {
		printf("time%s\n", formatted_time());
	} else if (!strcmp(argv[1], "--bspwm-status")) {
		char *bspwm_status = build_bspwm_status();

		if (bspwm_status != NULL) {
			printf("bspwm-status%s\n", bspwm_status);
			free(bspwm_status);
		}
	} else if (!strcmp(argv[1], "--charge-glyph")) {
		printf("charge-glyph%s\n", battery_status());
	} else if (!strcmp(argv[1], "--network-status")) {
		printf("network-status%s\n", network_status());
#ifdef SUPPORTS_ASOUNDLIB
	} else if (!strcmp(argv[1], "--volume-slider")) {
		printf("volume-slider%s\n", build_volume_slider());
#endif
	} else if (!strcmp(argv[1], "--get-pywal-color")) {
		if (argc < 4) {
			printf("Not enough arguments.\n");
			return 1;
		}

		printf("%s\n", get_pywal_color_value(atoi(argv[2]), argv[3]));
	} else {
		printf("Unknown argument: %s.\n", argv[1]);
		return 1;
	}
}

/*
 * Fetch color_index value from pywal cache file, if it fails return fallback_color.
 */
char *get_pywal_color_value(int color_index, char *fallback_color)
{
	FILE *pywal_file;
	static char line[8];
	int tmp = 0;

	pywal_file = fopen("/home/shagbag913/.cache/wal/colors", "r");

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

/*
 * Builds a visual slider representing something on a scale of 100.
 */
char *build_slider(int current_place) {
	static char final_slider[14] = "\0";
	int net_length = 4, plc;
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
char *formatted_time() {
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
				sprintf(return_window_status, "%s%{+u}%{U%s}", return_window_status,
						get_pywal_color_value(15, "#FFFFFF"));

			sprintf(return_window_status,
					"%s%%{A:bspc desktop -f ^%i:}      %i      %%{A}",
					return_window_status, index, index);

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
char *battery_status() {
	int charging, charge;
	static char status[25];
	char *low_color;

	charging = is_charging();
	charge = get_charge();

	if (charge == UNAVAILABLE)
		return "";

	if (charge >= 90) {
		strcpy(status, "");
	} else if (charge >= 70) {
		strcpy(status, "");
	} else if (charge >= 45) {
		strcpy(status, "");
	} else if (charge >= 15) {
		strcpy(status, "");
	} else {
		if (!charging) {
			low_color = get_pywal_color_value(1, "#FFA3A3");
			sprintf(status, "%s%%{F%s}", status, low_color);
		}
		strcat(status, "");
	}

	if (status[0] == '%')
		strcat(status, "%{F-}");

	sprintf(status, "%s  %i%%", status, charge);

	if (charging)
		strcat(status, "+");

	return status;
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

	return strcmp(status, "Discharging");
}

/* Returns network status like so:
 * - WiFi available and connected: white WiFi glyph
 * - WiFi available but not connected and no ethernet available: greyed out WiFi glyph
 * - WiFi available but not connected but ethernet available: wired connection glyph
 * - Connected to Ethernet: wired connection glyph
 * - Else: no output
 */
char *network_status() {
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
		return "";
	else if (strcmp(estate, "up"))
		return "%{F#99FFFFFF}%{F-}";
	else if (!strcmp(estate, "up"))
		return "";

	return "";
}

#ifdef SUPPORTS_ASOUNDLIB
/*
 * Returns PulseAudio volume in percentage.
 */
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

/*
 * Returns visual volume slider with glyph indicating volume level / status.
 */
char *build_volume_slider() {
	static char return_slider[22];
	struct volume volinfo = volume_info();

	if (volinfo.muted || volinfo.level == 0)
		sprintf(return_slider, "  %s", build_slider(volinfo.level));
	else
		sprintf(return_slider, "  %s", build_slider(volinfo.level));

	return return_slider;
}
#endif
