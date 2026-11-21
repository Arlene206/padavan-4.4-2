#include <stdio.h>
#include <stdlib.h>
#include "fullconenat_namespace_policy.h"

static void expect_equal(const struct fullconenat_namespace_context *a,
                         const struct fullconenat_namespace_context *b,
                         int expected, const char *label)
{
  int actual = fullconenat_namespace_context_equal(a, b);
  if (actual != expected) {
    fprintf(stderr, "%s: expected %d got %d\n", label, expected, actual);
    exit(1);
  }
}

int main(void)
{
  static int net_a, net_b;
  struct fullconenat_namespace_context a, same, other_net;
  struct fullconenat_namespace_context other_zone, other_flags, other_dir;
  struct fullconenat_namespace_context zero_net;

  fullconenat_namespace_context_set(&a, &net_a, 0, 0, 3);
  fullconenat_namespace_context_set(&same, &net_a, 0, 0, 3);
  fullconenat_namespace_context_set(&other_net, &net_b, 0, 0, 3);
  fullconenat_namespace_context_set(&other_zone, &net_a, 7, 0, 3);
  fullconenat_namespace_context_set(&other_flags, &net_a, 0, 1, 3);
  fullconenat_namespace_context_set(&other_dir, &net_a, 0, 0, 1);
  fullconenat_namespace_context_set(&zero_net, NULL, 0, 0, 3);

  expect_equal(&a, &same, 1, "same namespace and zone");
  expect_equal(&a, &other_net, 0, "different network namespace");
  expect_equal(&a, &other_zone, 0, "different zone id");
  expect_equal(&a, &other_flags, 0, "different zone flags");
  expect_equal(&a, &other_dir, 0, "different zone direction");
  expect_equal(&a, &zero_net, 0, "different null namespace");

  if (!fullconenat_namespace_context_belongs_to_net(&a, &net_a) ||
      fullconenat_namespace_context_belongs_to_net(&a, &net_b) ||
      fullconenat_namespace_context_belongs_to_net(&a, NULL) ||
      fullconenat_namespace_context_belongs_to_net(NULL, &net_a) ||
      fullconenat_namespace_context_equal(NULL, &a) ||
      fullconenat_namespace_context_equal(&a, NULL)) {
    fprintf(stderr, "namespace ownership predicate mismatch\n");
    return 1;
  }
  puts("FULLCONENAT namespace/zone context regression: PASS");
  return 0;
}
