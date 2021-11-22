#include <stdbool.h>

#define __opt_t(t) t ## _option

#define Option(t) \
typedef struct { \
	bool present; \
	t value; \
} __opt_t(t)

#define None(t) (__opt_t(t)) { .present = false }
#define Some(t, item) (__opt_t(t)) { .present = true, .value = item }
