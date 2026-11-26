#ifndef RALINK_COUNTRY_REGION_POLICY_H
#define RALINK_COUNTRY_REGION_POLICY_H

#include <stddef.h>
#include <string.h>
#include <strings.h>

typedef struct {
    const char *cc;
    unsigned char cc_2g;
    unsigned char cc_5g;
    unsigned char regspec_idx;
} ralink_country_region_entry_t;

/* Missing country NVRAM is a normal first-boot state, not a crash path. */
static inline int
ralink_country_region_lookup(const char *cc, int is_aband,
                             const ralink_country_region_entry_t *table,
                             size_t table_count)
{
    size_t i;

    if (!cc || !*cc || !table)
        return 1;

    for (i = 0; i < table_count; i++) {
        if (table[i].cc && strcasecmp(table[i].cc, cc) == 0)
            return is_aband ? (int)table[i].cc_5g : (int)table[i].cc_2g;
    }
    return 1;
}

/* Keep the legacy two-character GB fallback without calling strlen(NULL). */
static inline const char *
ralink_country_code_normalize(const char *cc)
{
    if (!cc || strlen(cc) != 2)
        return "GB";
    return cc;
}

#endif
