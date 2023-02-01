// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file transit_maps.h
 * @author Sherif Abdelwahab (@zasherif)
 *         Wei  Yue          (@w-yue)
 * 
 * @brief Defines ebpf maps of XDP
 *
 * @copyright Copyright (c) 2020-2022 The Authors.
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
#pragma once

#include <linux/bpf.h>

#include "extern/bpf_helpers.h"
#include "extern/xdpcap_hook.h"

#include "trn_datamodel.h"

struct bpf_map_def SEC("maps") jmp_table = {
	.type = BPF_MAP_TYPE_PROG_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(__u32),
	.max_entries = TRAN_MAX_PROG,
};
BPF_ANNOTATE_KV_PAIR(jmp_table, __u32, __u32);

struct bpf_map_def SEC("maps") endpoints_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(endpoint_key_t),
	.value_size = sizeof(endpoint_t),
	.max_entries = TRAN_MAX_NEP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(endpoints_map, endpoint_key_t, endpoint_t);

struct bpf_map_def SEC("maps") xsks_map = {
        .type = BPF_MAP_TYPE_XSKMAP,
        .key_size = sizeof(int),
        .value_size = sizeof(int),
        .max_entries = 64, /* Assume netdev has no more than 64 queues */
};
#if connTrack
struct bpf_map_def SEC("maps") contrack_map = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(contrack_key_t),
	.value_size = sizeof(contrack_t),
	.max_entries = TRAN_MAX_NEP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(contrack_map, contrack_key_t, contrack_t);
#endif

#if sgSupport
struct bpf_map_def SEC("maps") sg_cidr_map = {
	.type = BPF_MAP_TYPE_LPM_TRIE,
	.key_size = sizeof(sg_cidr_key_t),
	.value_size = sizeof(security_group_t),
	.max_entries = TRAN_MAX_CIDRS,
	.map_flags = BPF_F_NO_PREALLOC,
};
BPF_ANNOTATE_KV_PAIR(sg_cidr_map, sg_cidr_key_t, security_group_t);

struct bpf_map_def SEC("maps") security_group_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(security_group_key_t),
	.value_size = sizeof(security_group_t),
	.max_entries = TRAN_MAX_NEP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(security_group_map, security_group_key_t, security_group_t);

struct bpf_map_def SEC("maps") port_range_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(port_range_key_t),
	.value_size = sizeof(port_range_t),
	.max_entries = TRAN_MAX_NEP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(port_range_map, port_range_key_t, port_range_t);
#endif

#if turnOn
struct bpf_map_def SEC("maps") hosted_eps_if = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(endpoint_key_t),
	.value_size = sizeof(int),
	.max_entries = TRAN_MAX_NEP,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(hosted_eps_if, endpoint_key_t, int);
#endif

struct bpf_map_def SEC("maps") if_config_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(struct tunnel_iface_t),
	.max_entries = TRAN_MAX_ITF,
	.map_flags = 0,
};
BPF_ANNOTATE_KV_PAIR(if_config_map, __u32, struct tunnel_iface_t);

/* Host specific interface map used for packet redirect */
struct bpf_map_def SEC("maps") interfaces_map = {
	.type = BPF_MAP_TYPE_DEVMAP,
	.key_size = sizeof(int),
	.value_size = sizeof(int),
	.max_entries = TRAN_ITF_MAP_MAX,
};
BPF_ANNOTATE_KV_PAIR(interface_map, int, int);

#if turnOn
struct bpf_map_def SEC("maps") oam_queue_map = {
	.type = BPF_MAP_TYPE_QUEUE,
	.key_size = 0,
	.value_size = sizeof(flow_ctx_t),
	.max_entries = TRAN_OAM_QUEUE_LEN,
};
BPF_ANNOTATE_KV_PAIR_QUEUESTACK(oam_queue_map, flow_ctx_t);
#endif

#if cnOn
struct bpf_map_def SEC("maps") oam_queue_map = {
	.type = BPF_MAP_TYPE_QUEUE,
	.key_size = 0,
	.value_size = sizeof(flow_ctx_t),
	.max_entries = TRAN_OAM_QUEUE_LEN,
};
BPF_ANNOTATE_KV_PAIR_QUEUESTACK(oam_queue_map, flow_ctx_t);
#endif

#if turnOn
struct bpf_map_def SEC("maps") fwd_flow_cache = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(ipv4_flow_t),
	.value_size = sizeof(struct scaled_endpoint_remote_t),
	.max_entries = TRAN_MAX_CACHE_SIZE,
};
BPF_ANNOTATE_KV_PAIR(fwd_flow_cache, ipv4_flow_t,
		     struct scaled_endpoint_remote_t);

struct bpf_map_def SEC("maps") rev_flow_cache = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(ipv4_flow_t),
	.value_size = sizeof(struct scaled_endpoint_remote_t),
	.max_entries = TRAN_MAX_CACHE_SIZE,
};
BPF_ANNOTATE_KV_PAIR(rev_flow_cache, ipv4_flow_t,
		     struct scaled_endpoint_remote_t);

struct bpf_map_def SEC("maps") host_flow_cache = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(ipv4_flow_t),
	.value_size = sizeof(struct remote_endpoint_t),
	.max_entries = TRAN_MAX_CACHE_SIZE,
};
BPF_ANNOTATE_KV_PAIR(host_flow_cache, ipv4_flow_t,
		     struct remote_endpoint_t);

struct bpf_map_def SEC("maps") ep_host_cache = {
	.type = BPF_MAP_TYPE_LRU_HASH,
	.key_size = sizeof(endpoint_key_t),
	.value_size = sizeof(struct remote_endpoint_t),
	.max_entries = TRAN_MAX_CACHE_SIZE,
};
BPF_ANNOTATE_KV_PAIR(ep_host_cache, endpoint_key_t,
		     struct remote_endpoint_t);
#endif

struct bpf_map_def SEC("maps") xdpcap_hook = XDPCAP_HOOK();
