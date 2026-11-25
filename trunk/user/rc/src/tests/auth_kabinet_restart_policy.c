#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "auth_kabinet_restart_policy.h"

struct fixture {
    int gateway_status;
    int password_status;
    int stop_status;
    int start_status;
    const char *gateway;
    const char *password;
    int calls[8];
    size_t call_count;
    int stop_calls;
    int start_calls;
    int start_received_values;
};

static void
record_call(struct fixture *fixture, int call)
{
    if (fixture->call_count < sizeof(fixture->calls) / sizeof(fixture->calls[0]))
        fixture->calls[fixture->call_count++] = call;
}

static int
read_value(void *context, const char *name, char *value, size_t value_size)
{
    struct fixture *fixture = (struct fixture *)context;
    const char *source;
    int status;
    int call;
    size_t length;

    if (strcmp(name, AUTH_KABINET_GATEWAY_KEY) == 0) {
        call = 1;
        source = fixture->gateway;
        status = fixture->gateway_status;
    } else if (strcmp(name, AUTH_KABINET_PASSWORD_KEY) == 0) {
        call = 2;
        source = fixture->password;
        status = fixture->password_status;
    } else {
        return EINVAL;
    }
    record_call(fixture, call);
    if (status != 0) {
        if (value_size > 1)
            memcpy(value, "x", 2);
        return status;
    }
    if (!source)
        source = "";
    length = strlen(source);
    if (length >= value_size)
        return ENAMETOOLONG;
    memcpy(value, source, length + 1);
    return 0;
}

static int
stop_service(void *context)
{
    struct fixture *fixture = (struct fixture *)context;

    fixture->stop_calls++;
    record_call(fixture, 3);
    return fixture->stop_status;
}

static int
start_service(void *context, const char *gateway, const char *password)
{
    struct fixture *fixture = (struct fixture *)context;

    fixture->start_calls++;
    record_call(fixture, 4);
    fixture->start_received_values =
        strcmp(gateway, fixture->gateway ? fixture->gateway : "") == 0 &&
        strcmp(password, fixture->password ? fixture->password : "") == 0;
    return fixture->start_status;
}

static void
fixture_init(struct fixture *fixture)
{
    memset(fixture, 0, sizeof(*fixture));
    fixture->gateway = "192.0.2.7";
    fixture->password = "secret";
}

static int
run_restart(struct fixture *fixture, enum auth_kabinet_restart_stage *stage,
            char *gateway, size_t gateway_size,
            char *password, size_t password_size)
{
    return auth_kabinet_restart_with_ops(read_value, stop_service, start_service,
                                         fixture, gateway, gateway_size,
                                         password, password_size, stage);
}

static int
buffers_are_clear(const char *gateway, size_t gateway_size,
                  const char *password, size_t password_size)
{
    size_t i;

    for (i = 0; i < gateway_size; i++)
        if (gateway[i] != '\0')
            return 0;
    for (i = 0; i < password_size; i++)
        if (password[i] != '\0')
            return 0;
    return 1;
}

static int
test_gateway_read_error_preserves_daemon(void)
{
    struct fixture fixture;
    enum auth_kabinet_restart_stage stage;
    char gateway[32], password[32];
    int status;

    fixture_init(&fixture);
    fixture.gateway_status = EIO;
    status = run_restart(&fixture, &stage, gateway, sizeof(gateway),
                         password, sizeof(password));
    if (status != EIO || stage != AUTH_KABINET_RESTART_STAGE_READ_GATEWAY ||
        fixture.call_count != 1 || fixture.calls[0] != 1 ||
        fixture.stop_calls != 0 || fixture.start_calls != 0 ||
        !buffers_are_clear(gateway, sizeof(gateway), password, sizeof(password)))
        return 1;
    return 0;
}

static int
test_password_read_error_preserves_daemon(void)
{
    struct fixture fixture;
    enum auth_kabinet_restart_stage stage;
    char gateway[32], password[32];
    int status;

    fixture_init(&fixture);
    fixture.password_status = EACCES;
    status = run_restart(&fixture, &stage, gateway, sizeof(gateway),
                         password, sizeof(password));
    if (status != EACCES || stage != AUTH_KABINET_RESTART_STAGE_READ_PASSWORD ||
        fixture.call_count != 2 || fixture.calls[0] != 1 || fixture.calls[1] != 2 ||
        fixture.stop_calls != 0 || fixture.start_calls != 0 ||
        !buffers_are_clear(gateway, sizeof(gateway), password, sizeof(password)))
        return 1;
    return 0;
}

static int
test_empty_password_stops_without_start(void)
{
    struct fixture fixture;
    enum auth_kabinet_restart_stage stage;
    char gateway[32], password[32];
    int status;

    fixture_init(&fixture);
    fixture.password = "";
    status = run_restart(&fixture, &stage, gateway, sizeof(gateway),
                         password, sizeof(password));
    if (status != -1 || stage != AUTH_KABINET_RESTART_STAGE_EMPTY_PASSWORD ||
        fixture.call_count != 3 || fixture.calls[0] != 1 || fixture.calls[1] != 2 ||
        fixture.calls[2] != 3 || fixture.stop_calls != 1 || fixture.start_calls != 0 ||
        !buffers_are_clear(gateway, sizeof(gateway), password, sizeof(password)))
        return 1;
    return 0;
}

static int
test_stop_error_blocks_start(void)
{
    struct fixture fixture;
    enum auth_kabinet_restart_stage stage;
    char gateway[32], password[32];
    int status;

    fixture_init(&fixture);
    fixture.stop_status = EBUSY;
    status = run_restart(&fixture, &stage, gateway, sizeof(gateway),
                         password, sizeof(password));
    if (status != EBUSY || stage != AUTH_KABINET_RESTART_STAGE_STOP ||
        fixture.call_count != 3 || fixture.stop_calls != 1 || fixture.start_calls != 0 ||
        fixture.calls[0] != 1 || fixture.calls[1] != 2 || fixture.calls[2] != 3 ||
        !buffers_are_clear(gateway, sizeof(gateway), password, sizeof(password)))
        return 1;
    return 0;
}

static int
test_reads_then_stop_then_start(void)
{
    struct fixture fixture;
    enum auth_kabinet_restart_stage stage;
    char gateway[32], password[32];
    int status;

    fixture_init(&fixture);
    status = run_restart(&fixture, &stage, gateway, sizeof(gateway),
                         password, sizeof(password));
    if (status != 0 || stage != AUTH_KABINET_RESTART_STAGE_NONE ||
        fixture.call_count != 4 || fixture.calls[0] != 1 || fixture.calls[1] != 2 ||
        fixture.calls[2] != 3 || fixture.calls[3] != 4 ||
        fixture.stop_calls != 1 || fixture.start_calls != 1 ||
        !fixture.start_received_values ||
        !buffers_are_clear(gateway, sizeof(gateway), password, sizeof(password)))
        return 1;
    return 0;
}

static int
test_start_error_is_propagated(void)
{
    struct fixture fixture;
    enum auth_kabinet_restart_stage stage;
    char gateway[32], password[32];
    int status;

    fixture_init(&fixture);
    fixture.start_status = EIO;
    status = run_restart(&fixture, &stage, gateway, sizeof(gateway),
                         password, sizeof(password));
    if (status != EIO || stage != AUTH_KABINET_RESTART_STAGE_START ||
        fixture.call_count != 4 || fixture.stop_calls != 1 || fixture.start_calls != 1 ||
        !fixture.start_received_values ||
        !buffers_are_clear(gateway, sizeof(gateway), password, sizeof(password)))
        return 1;
    return 0;
}

int
main(void)
{
    if (test_gateway_read_error_preserves_daemon() != 0 ||
        test_password_read_error_preserves_daemon() != 0 ||
        test_empty_password_stops_without_start() != 0 ||
        test_stop_error_blocks_start() != 0 ||
        test_reads_then_stop_then_start() != 0 ||
        test_start_error_is_propagated() != 0) {
        fprintf(stderr, "LAN authentication restart policy regression failed\n");
        return 1;
    }
    puts("LAN authentication restart/read-failure policy: PASS");
    return 0;
}
