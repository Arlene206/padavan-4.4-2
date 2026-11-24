#ifndef MTK_FT_ROKHID_POLICY_H
#define MTK_FT_ROKHID_POLICY_H

/* The 802.11r R0KH-ID is a nonempty string bounded by its profile field. */
static inline int
mtk_ft_r0khid_length_valid(unsigned int length, unsigned int maximum)
{
	return length != 0U && maximum != 0U && length <= maximum;
}

#endif /* MTK_FT_ROKHID_POLICY_H */
