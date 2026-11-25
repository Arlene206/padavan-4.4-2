/*
 * Validate the FTP MDTM time-val token without touching the filesystem.
 * Supported forms are YYYYMMDD, YYYYMMDDHHMMSS and
 * YYYYMMDDHHMMSS.<fraction>, followed by one space and a pathname.
 */
#ifndef VSFTPD_FTP_MDTM_POLICY_H
#define VSFTPD_FTP_MDTM_POLICY_H

#include <stddef.h>

static inline int
ftp_mdtm_is_digit(char value)
{
	return value >= '0' && value <= '9';
}

static inline unsigned int
ftp_mdtm_decimal_field(const char *text, size_t offset, size_t width)
{
	size_t i;
	unsigned int value = 0;

	for (i = 0; i < width; ++i)
		value = value * 10U + (unsigned int)(text[offset + i] - '0');
	return value;
}

static inline int
ftp_mdtm_is_leap_year(unsigned int year)
{
	return (year % 400U == 0U) ||
	       ((year % 4U == 0U) && (year % 100U != 0U));
}

static inline int
ftp_mdtm_timeval_is_valid(const char *argument, size_t token_length)
{
	static const unsigned char month_days[12] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};
	size_t i;
	unsigned int year, month, day, hour = 0, minute = 0, second = 0;
	unsigned int maximum_day;

	if (argument == NULL ||
	    (token_length != 8U && token_length != 14U && token_length < 16U))
		return 0;
	if (argument[token_length] != ' ' || argument[token_length + 1U] == '\0')
		return 0;
	if (token_length > 14U && argument[14] != '.')
		return 0;

	for (i = 0; i < token_length; ++i) {
		if (token_length > 14U && i == 14U)
			continue;
		if (!ftp_mdtm_is_digit(argument[i]))
			return 0;
	}

	year = ftp_mdtm_decimal_field(argument, 0U, 4U);
	month = ftp_mdtm_decimal_field(argument, 4U, 2U);
	day = ftp_mdtm_decimal_field(argument, 6U, 2U);
	if (year == 0U || month == 0U || month > 12U || day == 0U)
		return 0;

	maximum_day = month_days[month - 1U];
	if (month == 2U && ftp_mdtm_is_leap_year(year))
		++maximum_day;
	if (day > maximum_day)
		return 0;

	if (token_length >= 14U) {
		hour = ftp_mdtm_decimal_field(argument, 8U, 2U);
		minute = ftp_mdtm_decimal_field(argument, 10U, 2U);
		second = ftp_mdtm_decimal_field(argument, 12U, 2U);
		if (hour > 23U || minute > 59U || second > 59U)
			return 0;
	}

	return 1;
}

#endif /* VSFTPD_FTP_MDTM_POLICY_H */
