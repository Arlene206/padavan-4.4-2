#!/usr/bin/env python3
"""Check QDMA TX allocation rollback on every two-stage setup failure."""

from pathlib import Path


SOURCE = Path(
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether_qdma.c"
).read_text()


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError(f"unterminated function: {signature}")


desc_alloc = function_body(SOURCE, "bool qdma_tx_desc_alloc(")
skb_init = desc_alloc.index("ei_local->skb_free[i] = 0;")
ring_alloc = desc_alloc.index("ei_local->txd_pool =")
assert skb_init < ring_alloc
assert desc_alloc.index("if (!ei_local->txd_pool)", ring_alloc) > ring_alloc

fq_init = function_body(SOURCE, "bool fq_qdma_init(")
first_alloc = fq_init.index("free_head = dma_alloc_coherent(")
second_alloc = fq_init.index("free_page_head =", first_alloc)
second_check = fq_init.index("if (unlikely(!free_page_head))", second_alloc)
rollback = fq_init.index("dma_free_coherent(", second_check)
rollback_null = fq_init.index("free_head = NULL;", rollback)
rollback_return = fq_init.index("return 0;", rollback_null)
assert first_alloc < second_alloc < second_check < rollback < rollback_null < rollback_return

tx_init = function_body(SOURCE, "int fe_qdma_tx_dma_init(")
first_failure = tx_init.index("if (!pass)")
first_cleanup = tx_init.index("fe_qdma_tx_dma_deinit(dev);", first_failure)
second_failure = tx_init.index("if (!pass)", first_cleanup)
second_cleanup = tx_init.index("fe_qdma_tx_dma_deinit(dev);", second_failure)
assert first_failure < first_cleanup < second_failure < second_cleanup
assert tx_init.count("return -ENOMEM;") == 2

tx_deinit = function_body(SOURCE, "void fe_qdma_tx_dma_deinit(")
assert "if (ei_local->txd_pool)" in tx_deinit
assert "ei_local->txd_pool = NULL;" in tx_deinit
assert "ei_local->free_head = NULL;" in tx_deinit
assert "ei_local->free_page_head = NULL;" in tx_deinit
assert "ei_local->skb_free[i] = NULL;" in tx_deinit
print("QDMA TX staged allocation failure rollback and teardown: PASS")

sfq_init = function_body(SOURCE, "bool sfq_init(")
assert "netdev_priv(dev)" in sfq_init
assert "netdev_priv(dev_raether)" not in sfq_init
assert "struct SFQ_table *sfq0 =" not in sfq_init
assert "struct SFQ_table *sfq7 =" not in sfq_init
alloc_at = sfq_init.index("dma_alloc_coherent(")
alloc_check = sfq_init.index("if (!*tables[i])", alloc_at)
memset_at = sfq_init.index("memset(*tables[i]", alloc_check)
assert alloc_at < alloc_check < memset_at
assert "for (i = 0; i < table_count; i++)" in sfq_init
assert "table_count = 8;" in sfq_init and "table_count = 4;" in sfq_init
sfq_fail = sfq_init.index("no_sfq_mem:")
sfq_rollback = sfq_init.index("sfq_deinit(dev);", sfq_fail)
assert sfq_fail < sfq_rollback < sfq_init.index("return false;", sfq_rollback)
assert "return true;" in sfq_init

sfq_cleanup = function_body(SOURCE, "static void sfq_deinit(")
assert "dma_free_coherent(" in sfq_cleanup
assert "*tables[i] = NULL;" in sfq_cleanup
assert "*phys[i] = 0;" in sfq_cleanup

tx_init = function_body(SOURCE, "int fe_qdma_tx_dma_init(")
assert "if ((ei_local->features & FE_HW_SFQ) && !sfq_init(dev))" in tx_init
assert "return -ENOMEM;" in tx_init
tx_deinit = function_body(SOURCE, "void fe_qdma_tx_dma_deinit(")
assert "sfq_deinit(dev);" in tx_deinit
print("QDMA SFQ allocation failure propagation and DMA lifetime cleanup: PASS")
