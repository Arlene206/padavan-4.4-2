/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All related IEEE802.11f IAPP + IEEE802.11r/11k IAPP extension.

***************************************************************************/

/* Include */

#include "rt_config.h"
#include "rtmpiapp.h"
#include "iappdefs.h"
#include "ft_client_hash_policy.h"

VOID os_alloc_mem(
	UCHAR *pAd, 
	UCHAR **ppMem, 
	UINT32 Size)
{
	*ppMem = (UCHAR *)malloc(Size); 
}

VOID os_free_mem(UCHAR *pAd, VOID *pMem)
{
	free(pMem); 
}

BOOLEAN mt_iapp_get_wifi_iface_mac(
	RTMP_IAPP		*pCtrlBK)
{
#ifdef IAPP_OS_LINUX
	INT idx;
	for (idx = 0; idx < pCtrlBK->IfNameWlanCount; idx ++) {
		struct ifreq		ReqIf;

		NdisZeroMemory(pCtrlBK->IfNameWlanMAC[idx], ETH_ALEN);
		NdisCopyMemory(ReqIf.ifr_name, pCtrlBK->IfNameWlanIoctl[idx], IFNAMSIZ);
		/* get mac address of the interface */
		if (ioctl(pCtrlBK->SocketIoctl, SIOCGIFHWADDR, &ReqIf) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, "iapp> Fail to get MAC of IfName[%d]: %s\n", idx, pCtrlBK->IfNameWlanIoctl[idx]);
			return FALSE;
		}
		else {
			NdisCopyMemory(pCtrlBK->IfNameWlanMAC[idx], &ReqIf.ifr_ifru.ifru_hwaddr.sa_data[0], ETH_ALEN);
			DBGPRINT(RT_DEBUG_OFF, "iapp> IfName[%d]: %s\n", idx, pCtrlBK->IfNameWlanIoctl[idx]);
			//IAPP_HEX_DUMP("MAC", pCtrlBK->IfNameWlanMAC[idx], ETH_ALEN);
		}
	}
	return TRUE;
#else
	return FALSE;
#endif
}

INT32 mt_iapp_find_ifidx_by_mac(
	RTMP_IAPP		*pCtrlBK,
	UCHAR			*WifiMAC)
{
	INT32 idx = 0;
	
	if (WifiMAC == NULL) {
		DBGPRINT(RT_DEBUG_OFF, "iapp> %s - WifiMAC is null.\n", __FUNCTION__);
		return -1;
	}	
	
	for (idx = 0; idx < pCtrlBK->IfNameWlanCount; idx++) {
		if (NdisCompareMemory(WifiMAC, pCtrlBK->IfNameWlanMAC[idx], ETH_ALEN) == 0) {
			return idx;
		}
	}

	return -1;
}

VOID mt_iapp_set_daemon_information(
	RTMP_IAPP		*pCtrlBK,
	pid_t 			*pPidAuth)
{
	INT32 ComLen = 0;
	INT32 idx = 0;

	for (idx = 0; idx < pCtrlBK->IfNameWlanCount; idx++) {
		ComLen = sizeof(INT32);
		IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP,
						pPidAuth, &ComLen, idx,
						RT_SET_IAPP_PID | OID_GET_SET_TOGGLE);
#ifdef FT_KDP_KEY_FROM_DAEMON		
		ComLen = strlen(pCtrlBK->CommonKey);
		IAPP_IOCTL_TO_WLAN(pCtrlBK, RT_IOCTL_IAPP,
							pCtrlBK->CommonKey, &ComLen, idx,
							RT_FT_KEY_SET | OID_GET_SET_TOGGLE);
#endif /* FT_KDP_KEY_FROM_DAEMON */
	}
	return;
}

VOID mt_iapp_ft_client_table_init(
	RTMP_IAPP		*pCtrlBK)
{
	INT i;
	NdisZeroMemory(&pCtrlBK->SelfFtStaTable, sizeof(pCtrlBK->SelfFtStaTable));
	for (i=0; i< FT_CLIENTS_HASH_TABLE_SIZE; i++)
		pCtrlBK->SelfFtStaTable.hash[i] = NULL;
	return;
}

FT_CLIENT_INFO *mt_iapp_ft_client_look_up(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR *pAddr)
{
	int clientnum = 0;
	ULONG HashIdx;
	FT_CLIENT_INFO *ft_entry = NULL;

	if (pFtTable == NULL || pAddr == NULL)
		return NULL;

	HashIdx = FT_MAC_ADDR_HASH_INDEX(pAddr);
	ft_entry = pFtTable->hash[HashIdx];

	while (ft_entry)
	{
		clientnum++;
		if (clientnum > FT_MAX_NUM_OF_CLIENT) {
			DBGPRINT(RT_DEBUG_TRACE, "iapp> %s - FT client not found or FT client table full. (clientnum=%d)\n", __FUNCTION__, clientnum);
			ft_entry = NULL;
			break;
		}

		if (NdisCompareMemory(ft_entry->sta_mac, pAddr, ETH_ALEN) == 0)
			break;
		else
			ft_entry = ft_entry->next;
	}

	return ft_entry;
}

FT_CLIENT_INFO *mt_iapp_ft_client_insert(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR 			*pStaAddr,
	UCHAR 			*pApAddr,
	INT32			ApIfIdx)
{
	UCHAR HashIdx;
	FT_CLIENT_INFO *ft_entry = NULL;

	if (pFtTable == NULL || pStaAddr == NULL)
		return NULL;

	ft_entry = mt_iapp_ft_client_look_up(pFtTable, pStaAddr);
	if (ft_entry) {
		/* Update information */
		if (pApAddr)
			NdisCopyMemory(ft_entry->ap_mac, pApAddr, ETH_ALEN);
		ft_entry->if_idx = ApIfIdx;
		ft_entry->used = 1;
		return ft_entry;
	}

	if (pFtTable->ft_sta_table_size < 0 ||
	    pFtTable->ft_sta_table_size >= FT_MAX_NUM_OF_CLIENT) {
		DBGPRINT(RT_DEBUG_ERROR, "iapp> %s - FT client table is full or inconsistent (FtStaTableSize=%d)\n",
				__FUNCTION__, pFtTable->ft_sta_table_size);
		return NULL;
	}

	/*
	 * A deleted slot can retain a stale next pointer. Clear it before the
	 * slot is reused, and do not proceed if the usage count and free slots
	 * disagree.
	 */
	ft_entry = ft_client_hash_find_free_slot(pFtTable->ft_sta_info,
						 FT_MAX_NUM_OF_CLIENT);
	if (ft_entry == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "iapp> %s - no free FT client slot found\n",
				__FUNCTION__);
		return NULL;
	}

	HashIdx = (UCHAR)FT_MAC_ADDR_HASH_INDEX(pStaAddr);
	if ((size_t)HashIdx >=
	    sizeof(pFtTable->hash) / sizeof(pFtTable->hash[0])) {
		DBGPRINT(RT_DEBUG_ERROR, "iapp> %s - invalid FT hash index\n",
				__FUNCTION__);
		return NULL;
	}

	NdisCopyMemory(ft_entry->sta_mac, pStaAddr, ETH_ALEN);
	if (pApAddr)
		NdisCopyMemory(ft_entry->ap_mac, pApAddr, ETH_ALEN);
	else
		NdisZeroMemory(ft_entry->ap_mac, ETH_ALEN);
	ft_entry->if_idx = ApIfIdx;
	ft_entry->hash_idx = HashIdx;

	if (!ft_client_hash_append(&pFtTable->hash[HashIdx], ft_entry,
				   FT_MAX_NUM_OF_CLIENT)) {
		DBGPRINT(RT_DEBUG_ERROR, "iapp> %s - cannot append FT client to hash chain\n",
				__FUNCTION__);
		NdisZeroMemory(ft_entry->sta_mac, ETH_ALEN);
		NdisZeroMemory(ft_entry->ap_mac, ETH_ALEN);
		ft_entry->if_idx = -1;
		ft_entry->hash_idx = 0;
		ft_entry->used = 0;
		ft_entry->next = NULL;
		return NULL;
	}

	ft_entry->used = 1;
	pFtTable->ft_sta_table_size++;
	return ft_entry;
}

VOID mt_iapp_ft_client_delete(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR 			*pStaAddr)
{
	UCHAR HashIdx;
	FT_CLIENT_INFO *ft_entry = NULL;

	if (pFtTable == NULL || pStaAddr == NULL)
		return;

	ft_entry = mt_iapp_ft_client_look_up(pFtTable, pStaAddr);
	if (ft_entry == NULL) {
		DBGPRINT(RT_DEBUG_TRACE, "iapp> %s - cannot find this entry (%02x:%02x:%02x:%02x:%02x:%02x)\n",
				__FUNCTION__,
				pStaAddr[0], pStaAddr[1], pStaAddr[2],
				pStaAddr[3], pStaAddr[4], pStaAddr[5]);
		return;
	}

	HashIdx = (UCHAR)FT_MAC_ADDR_HASH_INDEX(pStaAddr);
	if ((size_t)HashIdx >=
	    sizeof(pFtTable->hash) / sizeof(pFtTable->hash[0]) ||
	    !ft_client_hash_unlink(&pFtTable->hash[HashIdx], ft_entry,
				   FT_MAX_NUM_OF_CLIENT)) {
		DBGPRINT(RT_DEBUG_ERROR, "iapp> %s - FT client hash unlink failed; preserving slot\n",
				__FUNCTION__);
		return;
	}

	/* Unlink first so the released slot cannot retain a cross-bucket link. */
	ft_entry->used = 0;
	NdisZeroMemory(ft_entry->ap_mac, ETH_ALEN);
	NdisZeroMemory(ft_entry->sta_mac, ETH_ALEN);
	ft_entry->if_idx = -1;
	ft_entry->hash_idx = 0;

	if (pFtTable->ft_sta_table_size > 0)
		pFtTable->ft_sta_table_size--;
	else
		DBGPRINT(RT_DEBUG_ERROR, "iapp> %s - FT client count was already zero\n",
				__FUNCTION__);
}

INT32 mt_iapp_find_ifidx_by_sta_mac(
	FT_CLIENT_TABLE		*pFtTable,
	UCHAR			*pStaMAC)
{
	FT_CLIENT_INFO *ft_entry = NULL;
	ft_entry = mt_iapp_ft_client_look_up(pFtTable, pStaMAC);
	if (ft_entry)
		return ft_entry->if_idx;
	else
		return -1;
}
/* End of mt_iapp_util.c */
