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
assert "int allocated = 0;" in body
assert "int mapped = 0;" in body
ring_alloc = body.index("ei_local->rx_ring[0] = dma_alloc_coherent(")
ring_check = body.index("if (!ei_local->rx_ring[0])", ring_alloc)
allocation = body.index("raeth_alloc_skb_data(", ring_check)
allocation_success = body.index("allocated++;", allocation)
first_ring_access = body.index("&ei_local->rx_ring[0][i]", allocation_success)
assert ring_alloc < ring_check < allocation < allocation_success < first_ring_access

map_check = body.index("dma_mapping_error", first_ring_access)
assert allocation < allocation_success < map_check
map_fail = body.index("goto no_rx_mem;", map_check)
map_account = body.index("mapped++;", map_fail)
assert map_check < map_fail < map_account

rollback = body.index("no_rx_mem:")
unmap = body.index("dma_unmap_single(", rollback)
allocated_loop = body.index("for (j = 0; j < allocated; j++)", unmap)
free_skb = body.index("raeth_free_skb_data(", allocated_loop)
free_ring = body.index("dma_free_coherent(", free_skb)
null_ring = body.index("ei_local->rx_ring[0] = NULL;", free_ring)
assert rollback < unmap < free_skb < free_ring < null_ring
assert "for (j = 0; j < mapped; j++)" in body[rollback:free_skb]
assert allocated_loop < free_skb < free_ring
assert "for (j = 0; j < num_rx_desc; j++)" not in body[allocated_loop:free_ring]
assert "if (ei_local->rx_ring[0])" in body[free_ring:null_ring]
assert "ei_local->netrx_skb_data[0][j] = NULL;" in body[free_skb:free_ring]
print("PDMA RX partial-allocation rollback and DMA-unmap ordering: PASS")

teardown = function_body(SOURCE, "void fe_pdma_rx_dma_deinit(")
unmap_at = teardown.index("dma_unmap_single(")
free_data_at = teardown.index("raeth_free_skb_data(")
free_ring_at = teardown.index("dma_free_coherent(")
assert unmap_at < free_data_at < free_ring_at
assert "if (ei_local->rx_ring[0])" in teardown[unmap_at:free_data_at]
assert "ei_local->netrx_skb_data[0][i] = NULL;" in teardown[free_data_at:free_ring_at]
assert "ei_local->rx_ring[0] = NULL;" in teardown[free_ring_at:]
print("PDMA RX teardown unmaps each live descriptor before releasing its buffer: PASS")

tx_body = function_body(SOURCE, "int fe_pdma_tx_dma_init(")
tx_alloc = tx_body.index("ei_local->tx_ring0 = dma_alloc_coherent(")
tx_null = tx_body.index("if (!ei_local->tx_ring0)", tx_alloc)
tx_return = tx_body.index("return -ENOMEM;", tx_null)
tx_first_access = tx_body.index("&ei_local->tx_ring0[i]", tx_return)
assert tx_alloc < tx_null < tx_return < tx_first_access

tx_teardown = function_body(SOURCE, "void fe_pdma_tx_dma_deinit(")
tx_free = tx_teardown.index("dma_free_coherent(")
tx_clear = tx_teardown.index("ei_local->tx_ring0 = NULL;", tx_free)
assert "if (ei_local->tx_ring0)" in tx_teardown[:tx_free]
assert tx_free < tx_clear
print("PDMA TX allocation failure is detected before descriptor access: PASS")

QDMA_SOURCE = Path(
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether_qdma.c"
).read_text()
qdma_body = function_body(QDMA_SOURCE, "int fe_qdma_rx_dma_init(")
assert "int allocated = 0;" in qdma_body and "int mapped = 0;" in qdma_body
qdma_alloc = qdma_body.index("ei_local->qrx_ring =")
qdma_ring_check = qdma_body.index("if (!ei_local->qrx_ring)", qdma_alloc)
qdma_skb_alloc = qdma_body.index("raeth_alloc_skb_data(", qdma_ring_check)
qdma_allocated = qdma_body.index("allocated++;", qdma_skb_alloc)
qdma_ring_use = qdma_body.index("&ei_local->qrx_ring[i]", qdma_allocated)
qdma_map_check = qdma_body.index("dma_mapping_error", qdma_ring_use)
qdma_mapped = qdma_body.index("mapped++;", qdma_map_check)
assert qdma_alloc < qdma_ring_check < qdma_skb_alloc < qdma_allocated < qdma_ring_use < qdma_map_check < qdma_mapped
qdma_rollback = qdma_body.index("no_rx_mem:")
qdma_unmap = qdma_body.index("dma_unmap_single(", qdma_rollback)
qdma_allocated_loop = qdma_body.index("for (j = 0; j < allocated; j++)", qdma_unmap)
qdma_free_skb = qdma_body.index("raeth_free_skb_data(", qdma_allocated_loop)
qdma_free_ring = qdma_body.index("dma_free_coherent(", qdma_free_skb)
assert "for (j = 0; j < mapped; j++)" in qdma_body[qdma_rollback:qdma_free_skb]
assert qdma_allocated_loop < qdma_free_skb < qdma_free_ring
assert "ei_local->netrx0_skb_data[j] = NULL;" in qdma_body[qdma_free_skb:qdma_free_ring]
assert "ei_local->qrx_ring = NULL;" in qdma_body[qdma_free_ring:]

qdma_teardown = function_body(QDMA_SOURCE, "void fe_qdma_rx_dma_deinit(")
qdma_unmap_at = qdma_teardown.index("dma_unmap_single(")
qdma_free_data_at = qdma_teardown.index("raeth_free_skb_data(")
qdma_free_ring_at = qdma_teardown.index("dma_free_coherent(")
assert qdma_unmap_at < qdma_free_data_at < qdma_free_ring_at
assert "ei_local->qrx_ring = NULL;" in qdma_teardown[qdma_free_ring_at:]
assert "ei_local->rx_ring[0] = NULL;" in qdma_teardown[qdma_free_ring_at:]
print("QDMA RX partial-init rollback and normal DMA teardown: PASS")
