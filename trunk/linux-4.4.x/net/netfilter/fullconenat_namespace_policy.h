#ifndef _FULLCONENAT_NAMESPACE_POLICY_H
#define _FULLCONENAT_NAMESPACE_POLICY_H

/* Global FULLCONENAT tables must retain the conntrack owner of each entry. */
struct fullconenat_namespace_context {
  const void *net;
  unsigned int zone_id;
  unsigned int zone_flags;
  unsigned int zone_dir;
};

static inline void
fullconenat_namespace_context_set(struct fullconenat_namespace_context *context,
                                  const void *net, unsigned int zone_id,
                                  unsigned int zone_flags, unsigned int zone_dir)
{
  context->net = net;
  context->zone_id = zone_id;
  context->zone_flags = zone_flags;
  context->zone_dir = zone_dir;
}

static inline int
fullconenat_namespace_context_equal(
    const struct fullconenat_namespace_context *left,
    const struct fullconenat_namespace_context *right)
{
  return left && right && left->net == right->net &&
         left->zone_id == right->zone_id &&
         left->zone_flags == right->zone_flags &&
         left->zone_dir == right->zone_dir;
}

static inline int
fullconenat_namespace_context_belongs_to_net(
    const struct fullconenat_namespace_context *context, const void *net)
{
  return context && context->net == net;
}

#endif
