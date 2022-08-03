// SPDX-License-Identifier: GPL-2.0
// The following code is originally derived from zeta project
/**
 * @file trn_rpc_protocol_handlers_1.c
 * @author Wei Yue           (@w-yue)
 *
 * @brief RPC handlers. Primarly allocate and populate data structs,
 * and update the ebpf maps through user space APIs.
 *
 * @copyright Copyright (c) 2022 The Authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <search.h>
#include <stdlib.h>
#include <stdint.h>

#include "trn_transitd.h"

void rpc_transit_remote_protocol_1(struct svc_req *rqstp,
				   register SVCXPRT *transp);

int *update_ep_1_svc(rpc_trn_endpoint_batch_t *batch, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc, ctx;

	trn_ep_t *ep = (trn_ep_t *)batch->rpc_trn_endpoint_batch_t_val;

	TRN_LOG_DEBUG("update_ep_1 batch size: %d", batch->rpc_trn_endpoint_batch_t_len);

	ctx = trn_update_endpoints_get_ctx();
	if (ctx < 0) {
		TRN_LOG_ERROR("Failed to get update endpoints context");
		result = RPC_TRN_ERROR;
		goto error;
	}

	for (__u32 i = 0; i < batch->rpc_trn_endpoint_batch_t_len; i++, ep++) {
		rc = trn_update_endpoint(ctx, &ep->xdp_ep.key, &ep->xdp_ep.val);
		if (rc) {
			TRN_LOG_ERROR("Failed to update endpoint %d %08x",
				ep->xdp_ep.key.vni, ep->xdp_ep.key.ip);
			result = RPC_TRN_ERROR;
			goto error;
		}
	}
	result = 0;

error:
	return &result;
}

int *delete_ep_1_svc(rpc_endpoint_key_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;

	TRN_LOG_DEBUG("delete_ep_1 ep vni: %ld, ip: 0x%x",
		      argp->vni, argp->ip);

	rc = trn_delete_endpoint((endpoint_key_t *)argp);

	if (rc != 0) {
		TRN_LOG_ERROR("Failure deleting ep %d - %d", argp->vni, argp->ip);
		result = RPC_TRN_ERROR;
		goto error;
	}

	result = 0;
error:
	return &result;
}

rpc_trn_endpoint_t *get_ep_1_svc(rpc_endpoint_key_t *argp,
				 struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static trn_ep_t result;
	int rc;
	
	TRN_LOG_DEBUG("get_ep_1 ep vni: %ld, ip: 0x%x",
		      argp->vni, argp->ip);

	rc = trn_get_endpoint((endpoint_key_t *)argp, &result.xdp_ep.val);

	if (rc != 0) {
		TRN_LOG_ERROR(
			"Cannot find ep %d - %d from XDP map",
			argp->vni, argp->ip);
		goto error;
	}

	result.xdp_ep.key.vni = argp->vni;
	result.xdp_ep.key.ip = argp->ip;

	return &result.rpc_ep;

error:
	return NULL;
}

int *update_droplet_1_svc(rpc_trn_droplet_t *droplet, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;
	struct tunnel_iface_t itf;
	trn_iface_t *eth;

	eth = trn_get_itf_context(droplet->interface);
	if (!eth) {
		TRN_LOG_ERROR("Failed to get droplet interface context %s",
			droplet->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}

	for (unsigned int i = 0; i < droplet->num_entrances; i++) {
		itf.entrances[i].ip = droplet->entrances[i].ip;
		memcpy(itf.entrances[i].mac, droplet->entrances[i].mac,
			sizeof(droplet->entrances[i].mac));
		itf.entrances[i].announced = 0;
	}
	itf.num_entrances = droplet->num_entrances;
	itf.iface_index = eth->iface_index;
	itf.ibo_port = eth->ibo_port;
	itf.role = eth->role;
	itf.protocol = eth->protocol;

	rc = trn_update_itf_config(&itf);
	if (rc) {
		TRN_LOG_ERROR("Failed to update droplet %s", droplet->interface);
		result = RPC_TRN_ERROR;
		goto error;
	}
	result = 0;

error:
	return &result;
}

/* RPC backend to load transit XDP and attach to interfaces */
int *load_transit_xdp_1_svc(rpc_trn_xdp_intf_t *xdp_intf, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	bool debug = xdp_intf->debug_mode == 0? false:true;

	if (trn_transit_xdp_load(xdp_intf->interfaces, xdp_intf->ibo_port, debug)) {
		TRN_LOG_ERROR("Failed to load transit XDP");
		result = RPC_TRN_FATAL;
	} else {
		result = 0;
	}

	return &result;
}

/* RPC backend to unload transit XDP from interfaces */
int *unload_transit_xdp_1_svc(rpc_trn_xdp_intf_t *xdp_intf, struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;

	if (trn_transit_xdp_unload(xdp_intf->interfaces)) {
		TRN_LOG_ERROR("Failed to unload transit XDP");
		result = RPC_TRN_FATAL;
	} else {
		result = 0;
	}

	return &result;
}

int *load_transit_xdp_ebpf_1_svc(rpc_trn_ebpf_prog_t *argp, struct svc_req *rqstp)
{
	UNUSED(rqstp);

	static int result;
	int rc;

	rc = trn_transit_ebpf_load(argp->prog_idx);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to insert XDP stage %d", argp->prog_idx);
		result = RPC_TRN_ERROR;
	} else {
		result = 0;
	}

	return &result;
}

int *unload_transit_xdp_ebpf_1_svc(rpc_trn_ebpf_prog_t *argp,
					     struct svc_req *rqstp)
{
	UNUSED(rqstp);
	static int result;
	int rc;

	rc = trn_transit_ebpf_unload(argp->prog_idx);

	if (rc != 0) {
		TRN_LOG_ERROR("Failed to remove XDP stage %d", argp->prog_idx);
		result = RPC_TRN_ERROR;
	} else {
		result = 0;
	}

	return &result;
}
