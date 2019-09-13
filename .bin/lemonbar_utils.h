#if __has_include(<alsa/asoundlib.h>)

#define SUPPORTS_ASOUNDLIB

struct volume {
	int muted;
	int level;
};

struct volume volume_info();
#endif

struct args {
	unsigned int us;
	void (*function)();
};

struct meminfo {
	unsigned int total;
	unsigned int used;
	unsigned int free;
	unsigned int buffers;
	unsigned int cached;
	unsigned int shmem;
	unsigned int sreclaimable;
};

#define USEC_TO_SEC(x) (x * 1000000)
#define COLOR_HEX_LENGTH 8
#define HOME getenv("HOME")
#define PRINTD(...) \
	do { \
		if (debug_enable) printf(__VA_ARGS__); \
	} while (0)

char *build_slider(int current_place);
char *get_pywal_color_value(int color_index, char *fallback_color);
short get_charge();
short is_charging();
void volume_slider();
void battery_status();
void formatted_time();
void network_status();
void libnotify_notify(char *message);
void build_bspwm_status();
void *function_thread();
void used_memory_percentage();
struct tm *get_time();

char bat_stat[25];
char net_stat[20];
char *bspwm_stat = NULL;
char time_stat[20];
char vol_slider[39];
char used_mem[11];

char *wm_status_test;
unsigned int shortest_sleep;
unsigned int *battery_status_sleep_time_ptr;
unsigned short debug_enable;
char off_glyph_color[COLOR_HEX_LENGTH];
char normal_glyph_color[COLOR_HEX_LENGTH];
char low_battery_glyph_color[COLOR_HEX_LENGTH];
