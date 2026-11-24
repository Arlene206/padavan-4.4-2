#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "../ra_desc_policy.h"

static void test_valid_ge1_and_ge2(void)
{
	u32 txd1 = 0, txd2 = 0, tx_descs = 0, tx_queues = 0;

	assert(raeth_desc_policy_calculate(1, 1024, 1, 1024, 1024, 1,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == 0);
	assert(txd1 == 1024 && txd2 == 1024);
	assert(tx_descs == 2048 && tx_queues == 2);
}

static void test_optional_ge2(void)
{
	u32 txd1 = 0, txd2 = 99, tx_descs = 0, tx_queues = 0;

	assert(raeth_desc_policy_calculate(1, 512, 0, 0, 1024, 0,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == 0);
	assert(txd1 == 512 && txd2 == 0);
	assert(tx_descs == 512 && tx_queues == 1);
}

static void test_reject_invalid_inputs(void)
{
	u32 txd1 = 7, txd2 = 8, tx_descs = 9, tx_queues = 10;

	assert(raeth_desc_policy_calculate(0, 512, 0, 0, 1024, 0,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == -EINVAL);
	assert(raeth_desc_policy_calculate(1, 512, 0, 1, 1024, 0,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == -EINVAL);
	assert(raeth_desc_policy_calculate(1, 512, 0, 0, 1024, 1,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == -EINVAL);
	assert(raeth_desc_policy_calculate(1, 512, 0, 0, 0, 0,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == -EINVAL);
	assert(raeth_desc_policy_calculate(1, 512, 0, 0, 1024, 2,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == -EINVAL);
	assert(txd1 == 7 && txd2 == 8 && tx_descs == 9 && tx_queues == 10);
	assert(raeth_desc_policy_calculate(1, 512, 0, 0, 1024, 0,
					   NULL, &txd2, &tx_descs,
					   &tx_queues) == -EINVAL);
}

static void test_detect_overflow(void)
{
	u32 txd1 = 0, txd2 = 0, tx_descs = 0, tx_queues = 0;
	u32 max_value = ~(u32)0;

	assert(raeth_desc_policy_calculate(2, max_value / 2 + 1,
					   0, 0, 1, 0,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == -EOVERFLOW);
	assert(raeth_desc_policy_calculate(1, max_value / 2 + 1,
					   1, max_value / 2 + 1, 1, 1,
					   &txd1, &txd2, &tx_descs,
					   &tx_queues) == -EOVERFLOW);
}

int main(void)
{
	test_valid_ge1_and_ge2();
	test_optional_ge2();
	test_reject_invalid_inputs();
	test_detect_overflow();
	puts("RAETH descriptor counts and allocation policy: PASS");
	return 0;
}
