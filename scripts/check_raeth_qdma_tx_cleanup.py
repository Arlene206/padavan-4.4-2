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
