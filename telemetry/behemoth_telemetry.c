#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>

#define THERMAL "/sys/class/thermal"
#define POWER   "/sys/class/power_supply"

static int readstr(const char *p, char *b, size_t n) {
    FILE *f = fopen(p, "r");
    if (!f) return 0;
    if (!fgets(b, n, f)) { fclose(f); return 0; }
    fclose(f);
    b[strcspn(b, "\r\n")] = 0;
    return 1;
}

static long readnum(const char *p) {
    char b[128];
    if (!readstr(p, b, sizeof(b))) return -1;
    return strtol(b, NULL, 10);
}

static void stamp(void) {
    time_t t = time(NULL);
    struct tm tmv;
    char b[64];
    localtime_r(&t, &tmv);
    strftime(b, sizeof(b), "%Y-%m-%dT%H:%M:%S%z", &tmv);
    printf("timestamp=%s\n", b);
    printf("node=0x363\n");
    printf("authority=did:nws:nathan-wayne-salles-01\n");
    printf("device=Samsung Galaxy S25 Ultra\n");
}

static void thermal(void) {
    DIR *d = opendir(THERMAL);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strncmp(e->d_name, "thermal_zone", 12)) continue;
        char p[PATH_MAX];
        char type[256] = "unknown";
        snprintf(p, sizeof(p), "%s/%s/type", THERMAL, e->d_name);
        readstr(p, type, sizeof(type));
        snprintf(p, sizeof(p), "%s/%s/temp", THERMAL, e->d_name);
        long raw = readnum(p);
        if (raw >= 0)
            printf("thermal=%s type=%s raw=%ld celsius=%.3f\n", e->d_name, type, raw, raw / 1000.0);
    }
    closedir(d);
}

static void battery(void) {
    DIR *d = opendir(POWER);
    if (!d) { printf("battery=permission_denied_or_unavailable\n"); return; }
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strncmp(e->d_name, "battery", 7)) continue;
        char p[PATH_MAX]; char b[128];
        snprintf(p, sizeof(p), "%s/%s/capacity", POWER, e->d_name);
        if (readstr(p, b, sizeof(b))) printf("battery_capacity=%s\n", b);
        snprintf(p, sizeof(p), "%s/%s/status", POWER, e->d_name);
        if (readstr(p, b, sizeof(b))) printf("battery_status=%s\n", b);
        snprintf(p, sizeof(p), "%s/%s/voltage_now", POWER, e->d_name);
        if (readstr(p, b, sizeof(b))) printf("battery_voltage_now=%s\n", b);
        snprintf(p, sizeof(p), "%s/%s/current_now", POWER, e->d_name);
        if (readstr(p, b, sizeof(b))) printf("battery_current_now=%s\n", b);
        snprintf(p, sizeof(p), "%s/%s/temp", POWER, e->d_name);
        if (readstr(p, b, sizeof(b))) printf("battery_temp_raw=%s\n", b);
    }
    closedir(d);
}

static void system_info(void) {
    char b[256];
    if (readstr("/proc/loadavg", b, sizeof(b))) printf("loadavg=%s\n", b);
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return;
    while (fgets(b, sizeof(b), f))
        if (!strncmp(b, "MemTotal:", 9) || !strncmp(b, "MemAvailable:", 13)) printf("%s", b);
    fclose(f);
}

static void snapshot(void) {
    printf("=== NWS SOVEREIGN TELEMETRY ===\n");
    stamp();
    thermal();
    battery();
    system_info();
    printf("=== END ===\n");
    fflush(stdout);
}

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "--watch")) {
        unsigned s = 2;
        if (argc > 2) s = (unsigned)strtoul(argv[2], NULL, 10);
        if (!s) s = 2;
        for (;;) { snapshot(); sleep(s); }
    }
    snapshot();
    return 0;
}
