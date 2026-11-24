#ifndef MTK_WPA_PSK_POLICY_H
#define MTK_WPA_PSK_POLICY_H

/* WPA PSK storage carries at most 64 payload bytes plus a terminator. */
static inline int
mtk_wpa_psk_length_valid(unsigned int length, unsigned int maximum)
{
	return maximum != 0U && length <= maximum;
}

#endif /* MTK_WPA_PSK_POLICY_H */
