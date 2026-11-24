#!/usr/bin/env python3
"""Check that PDMA RX ring initialization rolls back partial allocations."""

from pathlib import Path


SOURCE = Path(
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether_pdma.c"
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


body = function_body(SOURCE, "int fe_pdma_rx_dma_init(")
assert "int mapped = 0;" in body
ring_alloc = body.index("ei_local->rx_ring[0] = dma_alloc_coherent(")
ring_check = body.index("if (!ei_local->rx_ring[0])", ring_alloc)
first_ring_access = body.index("&ei_local->rx_ring[0][i]", ring_check)
assert ring_alloc < ring_check < first_ring_access

map_check = body.index("dma_mapping_error", first_ring_access)
map_fail = body.index("goto no_rx_mem;", map_check)
map_account = body.index("mapped++;", map_fail)
assert map_check < map_fail < map_account

rollback = body.index("no_rx_mem:")
unmap = body.index("dma_unmap_single(", rollback)
free_skb = body.index("raeth_free_skb_data(", unmap)
free_ring = body.index("dma_free_coherent(", free_skb)
null_ring = body.index("ei_local->rx_ring[0] = NULL;", free_ring)
assert rollback < unmap < free_skb < free_ring < null_ring
assert "for (j = 0; j < mapped; j++)" in body[rollback:free_skb]
assert "for (j = 0; j < num_rx_desc; j++)" in body[free_skb:free_ring]
assert "if (ei_local->netrx_skb_data[0][j])" in body[free_skb:free_ring]
assert "if (ei_local->rx_ring[0])" in body[free_ring:null_ring]
assert "ei_local->netrx_skb_data[0][j] = NULL;" in body[free_skb:free_ring]
print("PDMA RX partial-allocation rollback and DMA-unmap ordering: PASS")
