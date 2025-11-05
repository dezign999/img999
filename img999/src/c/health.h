#pragma once
#include <pebble.h>

typedef void (*HealthUpdateFn)(int steps);

// Returns whether the health service events are available/subscribed.
bool health_is_available(void);

// Returns the sum for the given metric (today) or 0 if not available.
int health_get_metric_sum(HealthMetric metric);

// Abbreviate steps (e.g. 12345 -> "12.3k") into result buffer (at least 7 bytes).
void health_abbrev_steps(int num, char* result);

// Initialize health with a callback that receives the step count.
// The callback will be invoked on health events.
void health_init(HealthUpdateFn on_update);

// Deinitialize health subscription.
void health_deinit(void);
