#ifndef MTK_FT_ROKHID_POLICY_H
#define MTK_FT_ROKHID_POLICY_H

/*
 * iwpriv character arguments may include the trailing NUL in the reported
 * byte count. Permit at most the payload maximum plus that optional byte.
 */
static inline int
mtk_ft_r0khid_input_length_valid(unsigned int length, unsigned int maximum)
{
	return length != 0U && maximum != 0U && length - 1U <= maximum;
}

/* Normalize either a raw payload length or a payload length including NUL. */
static inline int
mtk_ft_r0khid_normalize_length(const unsigned char *value,
			       unsigned int input_length,
			       unsigned int maximum,
			       unsigned int *normalized_length)
{
	unsigned int length = input_length;

	if (!value || !normalized_length ||
	    !mtk_ft_r0khid_input_length_valid(length, maximum))
		return -1;

	if (value[length - 1U] == '\0')
		length--;
	if (length == 0U || length > maximum)
		return -1;

	*normalized_length = length;
	return 0;
}

#endif /* MTK_FT_ROKHID_POLICY_H */
