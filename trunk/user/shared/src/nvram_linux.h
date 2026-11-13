#ifndef __NVRAM_LINUX__
#define __NVRAM_LINUX__

#include <sys/types.h>

#define NVRAM_MAX_PARAM_LEN	64
#define NVRAM_MAX_VALUE_LEN	4096

struct nvram_pair {
	char *name;
	char *value;
};


/* Shared-userland transaction: keep one exclusive lock across restore
 * snapshots, clear, profile apply and compensating rollback. */
struct nvram_transaction {
	int lock_fd;
	pid_t owner_pid;
};
extern int nvram_transaction_begin(struct nvram_transaction *transaction);
extern int nvram_transaction_end(struct nvram_transaction *transaction);
extern int nvram_transaction_getall(struct nvram_transaction *transaction,
				    char *buf, int count, int include_temp);
extern int nvram_transaction_set(struct nvram_transaction *transaction,
				 const char *name, const char *value, int is_temp);
extern int nvram_transaction_clear(struct nvram_transaction *transaction);

extern char *nvram_get(const char *name);
/* 1=value present, 0=key absent, -1=backend/read/close error. */
extern int nvram_get_checked(const char *name, char **value);
extern char *nvram_safe_get(const char *name);
extern int nvram_get_int(const char *name);
extern int nvram_safe_get_int(const char* name, int val_def, int val_min, int val_max);
/* include_temp: 0=persistent only, 1=all tuples, 2=volatile tuples only. */
extern int nvram_getall(char *buf, int count, int include_temp);

extern int nvram_set(const char *name, const char *value);
extern int nvram_set_int(const char *name, int value);
extern int nvram_unset(const char *name);

extern int nvram_set_temp(const char *name, const char *value);
extern int nvram_set_int_temp(const char *name, int value);

extern int nvram_match(const char *name, char *match);
extern int nvram_invmatch(const char *name, char *invmatch);

extern int nvram_commit(void);
extern int nvram_clear(void);


#endif

