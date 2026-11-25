/* GNU89 regression tests for UDPXY multicast renewal retry policy. */
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "udpxy_multicast_refresh_policy.h"

struct update_fixture {
    int results[4];
    int calls;
    char operations[4][5];
};

static int
fake_update(int msockfd, const struct in_addr *mifaddr,
            const struct in_addr *s_in_addr, const char *operation,
            void *context)
{
    struct update_fixture *fixture = (struct update_fixture *)context;
    int index = fixture->calls;

    assert(msockfd == 7);
    assert(NULL != mifaddr);
    assert(NULL != s_in_addr);
    assert(index < 4);
    assert((0 == strcmp(operation, "DROP")) ||
           (0 == strcmp(operation, "ADD")));
    (void)snprintf(fixture->operations[index],
                   sizeof(fixture->operations[index]), "%s", operation);
    ++fixture->calls;
    return fixture->results[index];
}

static void
assert_order(const struct update_fixture *fixture)
{
    assert(fixture->calls == 2);
    assert(0 == strcmp(fixture->operations[0], "DROP"));
    assert(0 == strcmp(fixture->operations[1], "ADD"));
}

static void
test_drop_failure_still_adds(void)
{
    struct update_fixture fixture;
    struct in_addr mifaddr, source;
    int rc;

    memset(&fixture, 0, sizeof(fixture));
    memset(&mifaddr, 0, sizeof(mifaddr));
    memset(&source, 0, sizeof(source));
    fixture.results[0] = -1;
    fixture.results[1] = 0;

    rc = udpxy_renew_multicast_with(fake_update, &fixture, 7,
                                     &mifaddr, &source);
    assert(rc == 0);
    assert_order(&fixture);
}

static void
test_add_failure_is_reported_after_drop_success(void)
{
    struct update_fixture fixture;
    struct in_addr mifaddr, source;
    int rc;

    memset(&fixture, 0, sizeof(fixture));
    memset(&mifaddr, 0, sizeof(mifaddr));
    memset(&source, 0, sizeof(source));
    fixture.results[0] = 0;
    fixture.results[1] = -7;

    rc = udpxy_renew_multicast_with(fake_update, &fixture, 7,
                                     &mifaddr, &source);
    assert(rc == -7);
    assert_order(&fixture);
}

static void
test_retry_recovers_after_partial_renewal(void)
{
    struct update_fixture fixture;
    struct in_addr mifaddr, source;
    int first_rc, second_rc;

    memset(&fixture, 0, sizeof(fixture));
    memset(&mifaddr, 0, sizeof(mifaddr));
    memset(&source, 0, sizeof(source));
    /* First DROP works, first ADD fails; second DROP sees no membership,
     * but ADD is still attempted and repairs the subscription. */
    fixture.results[0] = 0;
    fixture.results[1] = -1;
    fixture.results[2] = -1;
    fixture.results[3] = 0;

    first_rc = udpxy_renew_multicast_with(fake_update, &fixture, 7,
                                           &mifaddr, &source);
    second_rc = udpxy_renew_multicast_with(fake_update, &fixture, 7,
                                            &mifaddr, &source);
    assert(first_rc == -1);
    assert(second_rc == 0);
    assert(fixture.calls == 4);
    assert(0 == strcmp(fixture.operations[0], "DROP"));
    assert(0 == strcmp(fixture.operations[1], "ADD"));
    assert(0 == strcmp(fixture.operations[2], "DROP"));
    assert(0 == strcmp(fixture.operations[3], "ADD"));
}

static void
test_retry_deadline(void)
{
    assert(udpxy_mcast_refresh_anchor(100, 60, 0) == 100);
    /* At now+1, the elapsed time equals the configured 60-second interval. */
    assert(101 - udpxy_mcast_refresh_anchor(100, 60, -1) == 60);
    assert(udpxy_mcast_refresh_anchor(100, 1, -1) == 100);
}

int
main(void)
{
    test_drop_failure_still_adds();
    test_add_failure_is_reported_after_drop_success();
    test_retry_recovers_after_partial_renewal();
    test_retry_deadline();

    puts("UDPXY multicast refresh retry policy regression: PASS");
    return 0;
}
