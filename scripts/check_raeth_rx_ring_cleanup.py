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
ring_free_guard = body.rfind("if (ei_local->rx_ring[0])", 0, free_ring)
assert ring_free_guard >= 0 and ring_free_guard < free_ring
assert "ei_local->netrx_skb_data[0][j] = NULL;" in body[free_skb:free_ring]
print("PDMA RX partial-allocation rollback and DMA-unmap ordering: PASS")

teardown = function_body(SOURCE, "void fe_pdma_rx_dma_deinit(")
unmap_at = teardown.index("dma_unmap_single(")
free_data_at = teardown.index("raeth_free_skb_data(")
free_ring_at = teardown.index("dma_free_coherent(")
assert unmap_at < free_data_at < free_ring_at
ring_unmap_guard = teardown.rfind("if (ei_local->rx_ring[0])", 0, unmap_at)
assert ring_unmap_guard >= 0 and ring_unmap_guard < unmap_at
assert "ei_local->netrx_skb_data[0][i] = NULL;" in teardown[free_data_at:free_ring_at]
assert "ei_local->rx_ring[0] = NULL;" in teardown[free_ring_at:]
print("PDMA RX teardown unmaps each live descriptor before releasing its buffer: PASS")

tx_body = function_body(SOURCE, "int fe_pdma_tx_dma_init(")
tx_alloc = tx_body.index("ei_local->tx_ring0 =")
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
qdma_ring_free_guard = qdma_body.rfind("if (ei_local->qrx_ring)", 0, qdma_free_ring)
assert qdma_ring_free_guard >= 0 and qdma_ring_free_guard < qdma_free_ring
assert "ei_local->qrx_ring = NULL;" in qdma_body[qdma_free_ring:]

qdma_teardown = function_body(QDMA_SOURCE, "void fe_qdma_rx_dma_deinit(")
qdma_unmap_at = qdma_teardown.index("dma_unmap_single(")
qdma_free_data_at = qdma_teardown.index("raeth_free_skb_data(")
qdma_free_ring_at = qdma_teardown.index("dma_free_coherent(")
assert qdma_unmap_at < qdma_free_data_at < qdma_free_ring_at
qdma_unmap_guard = qdma_teardown.rfind("if (ei_local->qrx_ring)", 0, qdma_unmap_at)
assert qdma_unmap_guard >= 0 and qdma_unmap_guard < qdma_unmap_at
assert "ei_local->qrx_ring = NULL;" in qdma_teardown[qdma_free_ring_at:]
assert "ei_local->rx_ring[0] = NULL;" in qdma_teardown[qdma_free_ring_at:]
print("QDMA RX partial-init rollback and normal DMA teardown: PASS")

RSS_SOURCE = Path(
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether_rss.c"
).read_text()


def check_rss_ring(mode: str, max_rings: str) -> None:
    init = function_body(RSS_SOURCE, f"int fe_rss_{mode}ring_init(")
    assert "int allocated[" in init and "int mapped[" in init
    ring_alloc = init.index("ei_local->rx_ring[i] =")
    ring_check = init.index("if (!ei_local->rx_ring[i])", ring_alloc)
    skb_alloc = init.index("raeth_alloc_skb_data(", ring_check)
    allocated = init.index("allocated[i]++;", skb_alloc)
    first_desc = init.index("&ei_local->rx_ring[i][j]", allocated)
    map_check = init.index("dma_mapping_error", first_desc)
    mapped = init.index("mapped[i]++;", map_check)
    assert ring_alloc < ring_check < skb_alloc < allocated < first_desc < map_check < mapped

    failure = init.index("no_rx_mem:")
    unmap = init.index("dma_unmap_single(", failure)
    free_loop = init.index("for (j = 0; j < allocated[i]; j++)", unmap)
    free_skb = init.index("raeth_free_skb_data(", free_loop)
    free_ring = init.index("dma_free_coherent(", free_skb)
    ring_clear = init.index("ei_local->rx_ring[i] = NULL;", free_ring)
    assert failure < unmap < free_loop < free_skb < free_ring < ring_clear
    assert "for (j = 0; j < mapped[i]; j++)" in init[failure:free_loop]
    ring_guard = init.rfind("if (ei_local->rx_ring[i])", 0, free_ring)
    assert ring_guard >= 0 and ring_guard < free_ring
    assert "ei_local->netrx_skb_data[i][j] = NULL;" in init[free_skb:free_ring]

    deinit = function_body(RSS_SOURCE, f"void fe_rss_{mode}ring_deinit(")
    unmap_at = deinit.index("dma_unmap_single(")
    skb_free_at = deinit.index("raeth_free_skb_data(")
    ring_free_at = deinit.index("dma_free_coherent(")
    assert unmap_at < skb_free_at < ring_free_at
    unmap_guard = deinit.rfind("if (ei_local->rx_ring[i])", 0, unmap_at)
    ring_free_guard = deinit.rfind("if (ei_local->rx_ring[i])", 0, ring_free_at)
    assert unmap_guard >= 0 and unmap_guard < unmap_at
    assert ring_free_guard >= 0 and ring_free_guard < ring_free_at
    assert "ei_local->netrx_skb_data[i][j] = NULL;" in deinit[skb_free_at:ring_free_at]
    assert "ei_local->rx_ring[i] = NULL;" in deinit[ring_free_at:]


check_rss_ring("4", "MAX_RX_RING_NUM")
check_rss_ring("2", "MAX_RX_RING_NUM_2RING")
print("PDMA RSS 4-ring/2-ring partial-init rollback and DMA teardown: PASS")

LRO_SOURCE = Path(
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether_hwlro.c"
).read_text()
lro_init = function_body(LRO_SOURCE, "int fe_hw_lro_init(")
assert "int allocated[MAX_RX_RING_NUM] = { 0 };" in lro_init
assert "int mapped[MAX_RX_RING_NUM] = { 0 };" in lro_init
lro_ring_alloc = lro_init.index("ei_local->rx_ring[i] =")
lro_ring_check = lro_init.index("if (!ei_local->rx_ring[i])", lro_ring_alloc)
lro_skb_alloc = lro_init.index("raeth_alloc_skb_data(", lro_ring_check)
lro_allocated = lro_init.index("allocated[i]++;", lro_skb_alloc)
lro_desc = lro_init.index("&ei_local->rx_ring[i][j]", lro_allocated)
lro_map_check = lro_init.index("dma_mapping_error", lro_desc)
lro_mapped = lro_init.index("mapped[i]++;", lro_map_check)
assert lro_ring_alloc < lro_ring_check < lro_skb_alloc < lro_allocated < lro_desc < lro_map_check < lro_mapped
lro_failure = lro_init.index("no_rx_mem:")
lro_unmap = lro_init.index("dma_unmap_single(", lro_failure)
lro_free_loop = lro_init.index("for (j = 0; j < allocated[i]; j++)", lro_unmap)
lro_free_skb = lro_init.index("raeth_free_skb_data(", lro_free_loop)
lro_free_ring = lro_init.index("dma_free_coherent(", lro_free_skb)
lro_ring_guard = lro_init.rfind("if (ei_local->rx_ring[i])", 0, lro_free_ring)
assert lro_failure < lro_unmap < lro_free_loop < lro_free_skb < lro_free_ring
assert lro_ring_guard >= 0 and lro_ring_guard < lro_free_ring
assert "for (j = 0; j < mapped[i]; j++)" in lro_init[lro_failure:lro_free_loop]
assert "ei_local->netrx_skb_data[i][j] = NULL;" in lro_init[lro_free_skb:lro_free_ring]

lro_deinit = function_body(LRO_SOURCE, "void fe_hw_lro_deinit(")
lro_unmap_at = lro_deinit.index("dma_unmap_single(")
lro_skb_free_at = lro_deinit.index("raeth_free_skb_data(")
lro_ring_free_at = lro_deinit.index("dma_free_coherent(")
assert lro_unmap_at < lro_skb_free_at < lro_ring_free_at
lro_unmap_guard = lro_deinit.rfind("if (ei_local->rx_ring[i])", 0, lro_unmap_at)
lro_ring_free_guard = lro_deinit.rfind("if (ei_local->rx_ring[i])", 0, lro_ring_free_at)
assert lro_unmap_guard >= 0 and lro_unmap_guard < lro_unmap_at
assert lro_ring_free_guard >= 0 and lro_ring_free_guard < lro_ring_free_at
assert "ei_local->rx_ring[i] = NULL;" in lro_deinit[lro_ring_free_at:]
print("Hardware LRO partial-init rollback and DMA teardown: PASS")

RAETHER_SOURCE = Path(
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether.c"
).read_text()

ptx = function_body(RAETHER_SOURCE, "int ei_init_ptx_prx(")
assert "err = fe_pdma_rx_dma_init(dev);" in ptx
assert "err = fe_hw_lro_init(dev);" in ptx
assert "err = fe_rss_4ring_init(dev);" in ptx
assert "err = fe_rss_2ring_init(dev);" in ptx
assert "err = fe_pdma_tx_dma_init(dev);" in ptx
ptx_compact = " ".join(ptx.split())
assert "if (err) goto fail_accel;" in ptx_compact
ptx_accel_fail = ptx.index("fail_accel:")
ptx_rx_fail = ptx.index("fail_pdma_rx:", ptx_accel_fail)
assert ptx.index("fe_hw_lro_deinit(dev);", ptx_accel_fail) < ptx_rx_fail
assert ptx.index("fe_rss_4ring_deinit(dev);", ptx_accel_fail) < ptx_rx_fail
assert ptx.index("fe_rss_2ring_deinit(dev);", ptx_accel_fail) < ptx_rx_fail
assert ptx.index("fe_pdma_rx_dma_deinit(dev);", ptx_rx_fail) > ptx_rx_fail

qtx_prx = function_body(RAETHER_SOURCE, "int ei_init_qtx_prx(")
assert qtx_prx.index("fe_qdma_rx_dma_init(dev)") < qtx_prx.index("fe_pdma_rx_dma_init(dev)")
qtx_prx_compact = " ".join(qtx_prx.split())
assert "if (err) goto fail_qdma_rx;" in qtx_prx_compact
assert "if (err) goto fail_pdma_rx;" in qtx_prx_compact
assert "if (err) goto fail_accel;" in qtx_prx_compact
qtx_accel_fail = qtx_prx.index("fail_accel:")
qtx_pdma_fail = qtx_prx.index("fail_pdma_rx:", qtx_accel_fail)
qtx_qdma_fail = qtx_prx.index("fail_qdma_rx:", qtx_pdma_fail)
assert qtx_prx.index("fe_hw_lro_deinit(dev);", qtx_accel_fail) < qtx_pdma_fail
assert qtx_prx.index("fe_rss_4ring_deinit(dev);", qtx_accel_fail) < qtx_pdma_fail
assert qtx_prx.index("fe_rss_2ring_deinit(dev);", qtx_accel_fail) < qtx_pdma_fail
assert qtx_prx.index("fe_pdma_rx_dma_deinit(dev);", qtx_pdma_fail) < qtx_qdma_fail
assert qtx_prx.index("fe_qdma_rx_dma_deinit(dev);", qtx_qdma_fail) < qtx_prx.index("return err;", qtx_qdma_fail)

qtx_qrx = function_body(RAETHER_SOURCE, "int ei_init_qtx_qrx(")
assert qtx_qrx.index("fe_qdma_rx_dma_init(dev)") < qtx_qrx.index("fe_qdma_tx_dma_init(dev)")
qtx_qrx_compact = " ".join(qtx_qrx.split())
assert "if (err) goto fail_qdma_rx;" in qtx_qrx_compact
assert qtx_qrx.index("fe_qdma_rx_dma_deinit(dev);") > qtx_qrx.index("fail_qdma_rx:")
assert qtx_qrx.index("return err;", qtx_qrx.index("fail_qdma_rx:")) > qtx_qrx.index("fe_qdma_rx_dma_deinit(dev);")
print("RAETH PDMA/QDMA initialization now unwinds previously completed RX stages on later failures: PASS")

open_body = function_body(RAETHER_SOURCE, "int ei_open(")
dma_init_at = open_body.index("err = ei_init_dma(dev);")
dma_init_fail = open_body.index("if (err)", dma_init_at)
dma_init_put = open_body.index("module_put(THIS_MODULE);", dma_init_fail)
dma_init_return = open_body.index("return err;", dma_init_put)
assert dma_init_at < dma_init_fail < dma_init_put < dma_init_return

for create in ("kthread_create(fe_reset_thread", "kthread_create(phy_polling_thread"):
    create_at = open_body.index(create)
    reset_at = open_body.index("fe_reset();", create_at)
    deinit_at = open_body.index("ei_deinit_dma(dev);", reset_at)
    put_at = open_body.index("module_put(THIS_MODULE);", deinit_at)
    return_at = open_body.index("return err;", put_at)
    assert create_at < reset_at < deinit_at < put_at < return_at

switch_init_at = open_body.index("err = fe_sw_init();")
switch_deinit_at = open_body.index("ei_deinit_dma(dev);", switch_init_at)
switch_put_at = open_body.index("module_put(THIS_MODULE);", switch_deinit_at)
assert switch_init_at < switch_deinit_at < switch_put_at
assert open_body.index("kthread_create(phy_polling_thread") < open_body.index("netif_start_queue(dev)")
assert open_body.index("netif_start_queue(dev)") < open_body.index("fe_int_enable(dev)")

close_body = function_body(RAETHER_SOURCE, "int ei_close(")
assert "IS_ERR_OR_NULL(ei_local->kreset_task)" in close_body
assert "IS_ERR_OR_NULL(ei_local->kphy_poll_task)" in close_body
assert "ei_local->kreset_task = NULL;" in close_body
assert "ei_local->kphy_poll_task = NULL;" in close_body
print("RAETH open/close balances module refs and DMA on early setup failures, with optional thread guards: PASS")

irq_free = function_body(RAETHER_SOURCE, "static void fe_int_free_requested_irqs(")
for mask in ("RAETH_IRQ_REQ_IRQ0", "RAETH_IRQ_REQ_IRQ1",
             "RAETH_IRQ_REQ_IRQ2", "RAETH_IRQ_REQ_IRQ3",
             "RAETH_IRQ_REQ_ESW", "RAETH_IRQ_REQ_GSW"):
    assert mask in irq_free
assert "mii_mgr_write(31, 0x7008, 0);" in irq_free
assert irq_free.index("mii_mgr_write(31, 0x7008, 0);") < irq_free.index("free_irq(ei_local->gsw_irq, NULL);")
assert "ei_local->irq_requested = 0;" in irq_free

irq_enable = function_body(RAETHER_SOURCE, "static int fe_int_enable(")
assert "if (err) goto fail_irqs;" in " ".join(irq_enable.split())
assert "put_device(&pdev->dev);" in irq_enable
assert "ei_local->irq_requested |= RAETH_IRQ_REQ_GSW;" in irq_enable
assert "fe_int_free_requested_irqs(dev);" in irq_enable
irq_disable = function_body(RAETHER_SOURCE, "static int fe_int_disable(")
assert "fe_int_free_requested_irqs(dev);" in irq_disable
assert "free_irq(ei_local->irq0, dev);" not in irq_disable

open_irq_at = open_body.index("err = fe_int_enable(dev);")
open_irq_cleanup = open_body.index("fe_int_free_requested_irqs", open_irq_at) if "fe_int_free_requested_irqs" in open_body[open_irq_at:] else -1
open_reset = open_body.index("fe_reset();", open_irq_at)
open_deinit = open_body.index("ei_deinit_dma(dev);", open_reset)
open_module_put = open_body.index("module_put(THIS_MODULE);", open_deinit)
open_return = open_body.index("return err;", open_module_put)
assert open_irq_at < open_reset < open_deinit < open_module_put < open_return
assert "kthread_stop(ei_local->kphy_poll_task);" in open_body[open_irq_at:open_reset]
assert "kthread_stop(ei_local->kreset_task);" in open_body[open_irq_at:open_reset]
print("RAETH IRQ acquisition now tracks each request, frees partial sets on error, and propagates IRQ setup failures: PASS")

# Keep initial RX buffers large enough for skb_reserve() and the DMA payload
# mapped after NET_SKB_PAD. Runtime replacement paths already include it.
def check_rx_buffer_size(source: str, signature: str, payload_length: str) -> None:
    init = function_body(source, signature)
    compact = " ".join(init.split())
    expected = (
        f"skb_size = SKB_DATA_ALIGN({payload_length} + NET_IP_ALIGN + "
        "NET_SKB_PAD) + SKB_DATA_ALIGN(sizeof(struct skb_shared_info));"
    )
    assert expected in compact, (
        f"{signature} must reserve NET_SKB_PAD before the DMA payload"
    )

check_rx_buffer_size(SOURCE, "int fe_pdma_rx_dma_init(", "MAX_RX_LENGTH")
check_rx_buffer_size(QDMA_SOURCE, "int fe_qdma_rx_dma_init(", "MAX_RX_LENGTH")
check_rx_buffer_size(RSS_SOURCE, "int fe_rss_4ring_init(", "MAX_RX_LENGTH")
check_rx_buffer_size(RSS_SOURCE, "int fe_rss_2ring_init(", "MAX_RX_LENGTH")
check_rx_buffer_size(LRO_SOURCE, "int fe_hw_lro_init(", "MAX_LRO_RX_LENGTH")
print("RAETH PDMA/QDMA/RSS/HW-LRO RX buffers include NET_SKB_PAD headroom: PASS")

SWITCH_SOURCE = Path(
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/ra_switch.c"
).read_text()
switch_deinit = function_body(SWITCH_SOURCE, "void fe_sw_deinit(")
np_check = switch_deinit.index("if (!np)")
pdev_lookup = switch_deinit.index("of_find_device_by_node(np)")
pdev_check = switch_deinit.index("if (!pdev)", pdev_lookup)
drvdata = switch_deinit.index("platform_get_drvdata(pdev)")
gsw_check = switch_deinit.index("if (!gsw)", drvdata)
release = switch_deinit.index("put_device(&pdev->dev);")
assert np_check < pdev_lookup < pdev_check < drvdata < gsw_check < release
assert "goto out_put_device;" in switch_deinit[gsw_check:release]
assert switch_deinit.count("put_device(&pdev->dev);") == 1
for map_failure in (
    "failed to map MT7622 shutdown GPIO registers",
    "failed to map Leopard shutdown GPIO registers",
    "failed to map Leopard GPIO mode registers",
):
    fail_at = switch_deinit.index(map_failure)
    unwind_at = switch_deinit.index("goto out_put_device;", fail_at)
    assert fail_at < unwind_at < release
assert "return;" not in switch_deinit[gsw_check:]
print("Switch shutdown platform-device validation and reference cleanup: PASS")

UNINIT_SOURCE = RAETHER_SOURCE
uninit_body = function_body(UNINIT_SOURCE, "static void ei_uninit(")
assert "unregister_netdev(dev);" not in uninit_body
assert "free_netdev(dev);" not in uninit_body
assert "struct net_device *pseudo_dev = ei_local->pseudo_dev;" in uninit_body
pseudo_check = uninit_body.index("if ((ei_local->features & FE_GE2_SUPPORT) && pseudo_dev)")
pseudo_unregister = uninit_body.index("unregister_netdev(pseudo_dev);", pseudo_check)
pseudo_free = uninit_body.index("free_netdev(pseudo_dev);", pseudo_unregister)
assert pseudo_check < pseudo_unregister < pseudo_free
assert "ei_local->pseudo_dev = NULL;" in uninit_body[pseudo_check:pseudo_unregister]

pseudo_init = function_body(UNINIT_SOURCE, "void raeth_init_pseudo(")
pseudo_register = pseudo_init.index("err = register_netdev(dev);")
pseudo_fail = pseudo_init.index("if (err)", pseudo_register)
pseudo_clear = pseudo_init.index("p_ad->pseudo_dev = NULL;", pseudo_fail)
pseudo_free_failed = pseudo_init.index("free_netdev(dev);", pseudo_clear)
pseudo_return = pseudo_init.index("return;", pseudo_free_failed)
assert pseudo_register < pseudo_fail < pseudo_clear < pseudo_free_failed < pseudo_return
assert pseudo_init.count("register_netdev(dev);") == 1
assert pseudo_init[pseudo_register:].startswith("err = register_netdev(dev);")
print("RAETH ndo_uninit no longer recursively frees the primary netdev; failed pseudo registration is released safely: PASS")

napi_uninit = function_body(RAETHER_SOURCE, "static void ei_uninit(")
napi_remove = napi_uninit.index("if (ei_local->features & FE_INT_NAPI)")
napi_remove_end = napi_uninit.index("if ((ei_local->features & FE_GE2_SUPPORT) && pseudo_dev)")
for napi_name in (
    "napi_rx", "napi_tx", "napi_rx_rss0", "napi_rx_rss1",
    "napi_rx_rss2", "napi_rx_rss3", "napi",
):
    assert f"netif_napi_del(&ei_local->{napi_name});" in napi_uninit[napi_remove:napi_remove_end]
assert "netif_napi_del(" not in napi_uninit[napi_remove_end:]
assert "unregister_netdev(dev);" not in napi_uninit
assert "free_netdev(dev);" not in napi_uninit
assert napi_remove < napi_remove_end
print("RAETH ndo_uninit unregisters every configured NAPI instance before its dummy netdev is freed: PASS")

# The MT7622 switch's regulator/pin state is paired with open even when
# PHY setup or IRQ registration fails after the switch has been initialized.
switch_init_branch = open_body.index("err = fe_sw_init();")
switch_init_cleanup = open_body.index("fe_sw_deinit(ei_local);", switch_init_branch)
switch_init_return = open_body.index("return err;", switch_init_cleanup)
assert switch_init_branch < switch_init_cleanup < switch_init_return
irq_open_branch = open_body.index("err = fe_int_enable(dev);")
irq_open_cleanup = open_body.index("fe_sw_deinit(ei_local);", irq_open_branch)
irq_open_return = open_body.index("return err;", irq_open_cleanup)
assert irq_open_branch < irq_open_cleanup < irq_open_return
assert open_body.count("fe_sw_deinit(ei_local);") == 2
print("MT7622 switch deinit is paired with switch and IRQ setup failures: PASS")

close_order = function_body(RAETHER_SOURCE, "int ei_close(")
reset_stop = close_order.index("kthread_stop(ei_local->kreset_task);")
phy_stop = close_order.index("kthread_stop(ei_local->kphy_poll_task);")
queue_stop = close_order.index("netif_stop_queue(dev);")
radio_stop = close_order.index("ra2880stop(ei_local);")
hardware_reset = close_order.index("fe_reset();")
interrupt_disable = close_order.index("fe_int_disable(dev);")
dma_release = close_order.index("ei_deinit_dma(dev);")
assert reset_stop < queue_stop < radio_stop < hardware_reset
assert phy_stop < queue_stop < radio_stop < hardware_reset
assert hardware_reset < interrupt_disable < dma_release
print("RAETH close stops asynchronous workers and TX before resetting hardware: PASS")

switch_init = function_body(SWITCH_SOURCE, "int fe_sw_init(")
clock_enable = switch_init.index("status = clk_prepare_enable(ei_local->clks[MTK_CLK_GP0]);")
clock_check = switch_init.index("if (status)", clock_enable)
clock_failure = switch_init.index("goto mdio_error;", clock_check)
clocked_switch_access = switch_init.index(
    "sys_reg_write(ETHDMASYS_ETH_MAC_BASE + 0xC, 0x1)", clock_enable
)
assert clock_enable < clock_check < clock_failure < clocked_switch_access
assert switch_init.count("clk_prepare_enable(ei_local->clks[MTK_CLK_GP0]);") == 1
assert switch_init.count("clk_disable_unprepare(ei_local->clks[MTK_CLK_GP0]);") == 4
print("MT7621 switch setup aborts before MMIO when GP0 clock enable fails: PASS")

# reset_task was a dead, never-initialized work_struct: the only operation
# on it was cancel_work_sync(), with no INIT_WORK() or queueing path.
assert "reset_task" not in RAETHER_SOURCE
RAETHER_HEADER = Path(
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether.h"
).read_text()
assert "reset_task" not in RAETHER_HEADER
disable_body = function_body(RAETHER_SOURCE, "static int fe_int_disable(")
assert "cancel_work_sync(&ei_local->reset_task)" not in disable_body
assert "cancel_work_sync(&ei_local->rx_wq)" in disable_body
print("RAETH no longer cancels the uninitialized and never-queued reset_task work item: PASS")
