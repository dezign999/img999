#include "health.h"

static bool s_health_available = false;
static HealthUpdateFn s_on_update = NULL;

bool health_is_available(void) {
    return s_health_available;
}

int health_get_metric_sum(HealthMetric metric) {
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric,
      time_start_of_today(), time(NULL));
    if(mask == HealthServiceAccessibilityMaskAvailable) {
      return (int)health_service_sum_today(metric);
    } else {
      return 0;
    }
}

void health_abbrev_steps(int num, char* result) {
    for (int i = 0; i < 7; i++) {
        result[i] = '\0';
    }
    if (num == 0) {
        result[0] = '0';
        return;
    }
    char temp[10];
    int digitCount = 0;
    int tempNum = num;
    while (tempNum > 0 && digitCount < 10) {
        temp[digitCount] = (tempNum % 10) + '0';
        tempNum /= 10;
        digitCount++;
    }
    if (digitCount <= 3) {
        for (int i = 0; i < digitCount; i++) {
            result[i] = temp[digitCount - 1 - i];
        }
        return;
    }
    if (digitCount == 4) {
        result[0] = temp[3];
        if (temp[2] != '0') {
            result[1] = '.';
            result[2] = temp[2];
            result[3] = 'k';
        } else {
            result[1] = 'k';
        }
        return;
    }
    if (digitCount == 5) {
        result[0] = temp[4];
        result[1] = temp[3];
        if (temp[2] != '0') {
            result[2] = '.';
            result[3] = temp[2];
            result[4] = 'k';
        } else {
            result[2] = 'k';
        }
        return;
    }
}

static void prv_on_health_data(HealthEventType type, void *context) {
    s_health_available = true;
    if (s_on_update) {
        int steps = health_get_metric_sum(HealthMetricStepCount);
        s_on_update(steps);
    }
}

void health_init(HealthUpdateFn on_update) {
    s_on_update = on_update;
    s_health_available = health_service_events_subscribe(prv_on_health_data, NULL);
}

void health_deinit(void) {
    health_service_events_unsubscribe();
    s_on_update = NULL;
}
