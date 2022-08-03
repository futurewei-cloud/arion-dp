// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file test_dmn.c
 * @authors Wei Yue           (@w-yue)
 *
 * @brief dmn unit tests
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
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <sys/resource.h>

#include "dmn/trn_transitd.h"

struct test_bpf_map_t {
	char *name;
};

static struct test_bpf_map_t maps[] = {
	{.name="jmp_table"},
	{.name="endpoints_map"},
	{.name="hosted_eps_if"},
	{.name="if_config_map"},
	{.name="interfaces_map"},
#if turnOn
	{.name="oam_queue_map"},
	{.name="fwd_flow_cache"},
	{.name="rev_flow_cache"},
	{.name="host_flow_cache"},
	{.name="ep_host_cache"},
#endif
	{.name="xdpcap_hook"},
};

int __wrap_trn_transit_map_get_fd(char *map)
{
	UNUSED(map);
	return 1;
}

struct bpf_object *__wrap_bpf_object__open_file(const char *path,
				const struct bpf_object_open_opts *opts)
{
	UNUSED(path);
	UNUSED(opts);
	return (struct bpf_object *)1;
}

int __wrap_bpf_map__reuse_fd(struct bpf_map *map, int fd)
{
	UNUSED(map);
	UNUSED(fd);
	return 0;
}

int __wrap_bpf_obj_get(const char *pathname)
{
	UNUSED(pathname);
	return -1;
}

void __wrap_bpf_map__set_ifindex(struct bpf_map *map, __u32 ifindex)
{
	UNUSED(map);
	UNUSED(ifindex);
}

const char *__wrap_bpf_map__name(const struct bpf_map *map)
{
	return ((struct test_bpf_map_t *)map)->name;
}

const char *__wrap_bpf_object__name(const struct bpf_object *obj)
{
	UNUSED(obj);
	return "obj";
}

void __wrap_bpf_program__set_type(struct bpf_program *prog, enum bpf_prog_type type)
{
	UNUSED(prog);
	UNUSED(type);
}

void __wrap_bpf_program__set_ifindex(struct bpf_program *prog, __u32 ifindex)
{
	UNUSED(prog);
	UNUSED(ifindex);
}

int __wrap_close(int _fd)
{
	UNUSED(_fd);
	return 0;
}

int __wrap_setrlimit(int resource, const struct rlimit *rlim)
{
	UNUSED(resource);
	UNUSED(rlim);
	return 0;
}

int __wrap_bpf_map_update_elem(void *map, void *key, void *value,
			       unsigned long long flags)
{
	UNUSED(map);
	UNUSED(key);
	UNUSED(value);
	UNUSED(flags);
	function_called();
	return 0;
}

int __wrap_bpf_map_lookup_elem(void *map, void *key, void *value)
{
	UNUSED(map);
	UNUSED(key);
	endpoint_t *endpoint = mock_ptr_type(endpoint_t *);
	struct ftn_t *ftn = mock_ptr_type(struct ftn_t *);
	struct chain_t *chain = mock_ptr_type(struct chain_t *);
	struct dft_t *dft = mock_ptr_type(struct dft_t *);
	function_called();
	if (endpoint != NULL)
		memcpy(value, endpoint, sizeof(*endpoint));
	else if (dft != NULL)
		memcpy(value, dft, sizeof(*dft));
	else if (chain != NULL)
		memcpy(value, chain, sizeof(*chain));
	else if (ftn != NULL)
		memcpy(value, ftn, sizeof(*ftn));
	else
		return 1;
	return 0;
}

int __wrap_bpf_map_delete_elem(void *map, void *key)
{
	UNUSED(map);
	UNUSED(key);

	bool valid = mock_type(bool);
	function_called();

	if (valid)
		return 0;
	return 1;
}

int __wrap_bpf_map_lookup_and_delete_elem(void *map, const void *key, void *value)
{
	UNUSED(map);
	UNUSED(key);
	UNUSED(value);
	return 0;
}

int __wrap_bpf_prog_load_xattr(const struct bpf_prog_load_attr *attr,
			       struct bpf_object **pobj, int *prog_fd)
{
	UNUSED(attr);
	UNUSED(pobj);

	*prog_fd = 1;
	return 0;
}

int __wrap_bpf_xdp_attach(int ifindex, int fd, __u32 flags, const struct bpf_xdp_attach_opts *opts)
{
	UNUSED(ifindex);
	UNUSED(fd);
	UNUSED(flags);
	UNUSED(opts);
	return 0;
}

int __wrap_bpf_obj_get_info_by_fd(int prog_fd, void *info, __u32 *info_len)
{
	UNUSED(prog_fd);
	UNUSED(info);
	UNUSED(info_len);
	return 0;
}

struct bpf_map *__wrap_bpf_map__next(struct bpf_map *map,
				     struct bpf_object *obj)
{
	int num_maps = sizeof(maps) / sizeof(maps[0]);
	struct test_bpf_map_t *testmap = (struct test_bpf_map_t *)map;

	UNUSED(obj);
	if (testmap == NULL) {
		return (struct bpf_map *)maps;
	}
	testmap++;
	if (testmap >= &maps[num_maps]) {
		return NULL;
	}
	return (struct bpf_map *)testmap;
}

int __wrap_bpf_map__fd(struct bpf_map *map)
{
	UNUSED(map);
	return 1;
}

int __wrap_bpf_map__pin(struct bpf_map *map, const char *path)
{
	UNUSED(map);
	UNUSED(path);
	return 0;
}

int __wrap_bpf_map__unpin(struct bpf_map *map, const char *path)
{
	UNUSED(map);
	UNUSED(path);
	return 0;
}

int __wrap_bpf_xdp_query_id(int ifindex, int flags, unsigned int *prog_id)
{
	UNUSED(ifindex);
	UNUSED(flags);
	UNUSED(prog_id);
	return 0;	
}

unsigned int __wrap_if_nametoindex(const char *ifname)
{
	UNUSED(ifname);
	return 1;
}

const char *__wrap_if_indextoname(unsigned int ifindex, char *buf)
{
	UNUSED(buf);
	UNUSED(ifindex);
	if (ifindex != 1)
		return NULL;
	else
		return "lo";
}

struct bpf_object *__wrap_bpf_object__open(const char *path)
{
	UNUSED(path);
	return (struct bpf_object *)1;
}

int __wrap_bpf_create_map(enum bpf_map_type map_type, int key_size,
			  int value_size, int max_entries)
{
	UNUSED(map_type);
	UNUSED(key_size);
	UNUSED(value_size);
	UNUSED(max_entries);
	return 0;
}

int __wrap_bpf_program__fd(const struct bpf_program *prog)
{
	UNUSED(prog);
	return 1;
}

int __wrap_bpf_object__load(struct bpf_object *obj)
{
	UNUSED(obj);
	return 0;
}

struct bpf_map *
__wrap_bpf_object__find_map_by_name(const struct bpf_object *obj,
				    const char *name)
{
	UNUSED(obj);
	UNUSED(name);
	return (struct bpf_map *)1;
}
int __wrap_bpf_map__set_inner_map_fd(struct bpf_map *map, int fd)
{
	UNUSED(map);
	UNUSED(fd);
	return 0;
}

int __wrap_bpf_program__set_xdp(struct bpf_program *prog)
{
	UNUSED(prog);
	return 0;
}

struct bpf_program *__wrap_bpf_object__next_program(const struct bpf_object *obj,
						struct bpf_program *prev)
{
	UNUSED(obj);
	if (prev == NULL) {
		return (struct bpf_program *)1;
	} else
		return NULL;
}

struct bpf_map *__wrap_bpf_object__next_map(const struct bpf_object *obj,
						const struct bpf_map *map)
{
	UNUSED(obj);
	UNUSED(map);
	return (struct bpf_map *)1;
}

void __wrap_bpf_object__close(struct bpf_object *object)
{
	UNUSED(object);
	return;
}
static inline int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

static int check_ep_equal(rpc_trn_endpoint_t *s, rpc_trn_endpoint_t *d)
{
	trn_ep_t *ep = (trn_ep_t *)s;
	trn_ep_t *c_ep = (trn_ep_t *)d;

	assert_int_equal(ep->xdp_ep.key.vni, c_ep->xdp_ep.key.vni);

	assert_int_equal(ep->xdp_ep.key.ip, c_ep->xdp_ep.key.ip);

	assert_int_equal(ep->xdp_ep.val.hip, c_ep->xdp_ep.val.hip);

	assert_memory_equal(ep->xdp_ep.val.mac, c_ep->xdp_ep.val.mac,
		sizeof(char) * 6);

	assert_memory_equal(ep->xdp_ep.val.hmac, c_ep->xdp_ep.val.hmac,
		sizeof(char) * 6);

	return true;
}

static void do_lo_xdp_load(void)
{
	rpc_trn_xdp_intf_t xdp_intf;
	xdp_intf.interfaces[0] = "lo";
	xdp_intf.interfaces[1] = "lo";
	xdp_intf.debug_mode = 1;

	int *rc;
	/* Expect map update for jmp_table + interfaces_map */
	expect_function_calls(__wrap_bpf_map_update_elem, TRAN_MAX_PROG + 1);
	rc = load_transit_xdp_1_svc(&xdp_intf, NULL);
	assert_int_equal(*rc, 0);
}

static void do_lo_xdp_unload(void)
{
	rpc_trn_xdp_intf_t xdp_intf;
	xdp_intf.interfaces[0] = "lo";
	xdp_intf.interfaces[1] = "lo";
	xdp_intf.debug_mode = 1;

	int *rc;
	rc = unload_transit_xdp_1_svc(&xdp_intf, NULL);
	assert_int_equal(*rc, 0);
}

static void test_update_ep_1_svc(void **state)
{
	UNUSED(state);

	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char hmac[6] = { 6, 5, 4, 3, 2, 1 };

	trn_ep_t ep1 = {
		.xdp_ep.key.vni = 123,
		.xdp_ep.key.ip = 0x100000a,
		.xdp_ep.val.hip = 0x200000a,
	};
	memcpy(ep1.xdp_ep.val.mac, mac, sizeof(char) * 6);
	memcpy(ep1.xdp_ep.val.hmac, hmac, sizeof(char) * 6);

	rpc_trn_endpoint_batch_t epb = {
		.rpc_trn_endpoint_batch_t_len = 1,
		.rpc_trn_endpoint_batch_t_val = &ep1.rpc_ep,
	};

	int *rc;
	expect_function_calls(__wrap_bpf_map_update_elem, 1);
	rc = update_ep_1_svc(&epb, NULL);
	assert_int_equal(*rc, 0);
}

static void test_get_ep_1_svc(void **state)
{
	UNUSED(state);

	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char hmac[6] = { 6, 5, 4, 3, 2, 1 };

	trn_ep_t ep1 = {
		.xdp_ep.key.vni = 123,
		.xdp_ep.key.ip = 0x100000a,
		.xdp_ep.val.hip = 0x200000a,
	};
	memcpy(ep1.xdp_ep.val.mac, mac, sizeof(char) * 6);
	memcpy(ep1.xdp_ep.val.hmac, hmac, sizeof(char) * 6);

	rpc_trn_endpoint_batch_t epb = {
		.rpc_trn_endpoint_batch_t_len = 1,
		.rpc_trn_endpoint_batch_t_val = &ep1.rpc_ep,
	};

	rpc_endpoint_key_t ep_key1 = {
		.vni = 123,
		.ip = 0x100000a,
	};

	endpoint_t ep_val;
	ep_val.hip = 0x200000a;
	memcpy(ep_val.mac, ep1.xdp_ep.val.mac, sizeof(ep1.xdp_ep.val.mac));
	memcpy(ep_val.hmac, ep1.xdp_ep.val.hmac, sizeof(ep1.xdp_ep.val.hmac));

	/* Test get_ep with valid ep_key */
	struct rpc_trn_endpoint_t *retval;
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_true(check_ep_equal(retval, &ep1.rpc_ep));

	/* Test get_ep with bad return code from bpf_lookup */
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	retval = get_ep_1_svc(&ep_key1, NULL);
	assert_null(retval);
}

static void test_delete_ep_1_svc(void **state)
{
	UNUSED(state);

	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char hmac[6] = { 6, 5, 4, 3, 2, 1 };

	rpc_endpoint_key_t ep_key = {
		.vni = 123,
		.ip = 0x100000a,
	};

	endpoint_t ep_val;
	ep_val.hip = 0x200000a;
	memcpy(ep_val.hmac, hmac, sizeof(hmac));
	memcpy(ep_val.mac, mac, sizeof(mac));

	int *rc;

	/* Test delete_ep_1 with valid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_delete_elem, TRUE);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, 0);

	/* Test delete_ep_1 with invalid ep_key */
	will_return(__wrap_bpf_map_lookup_elem, &ep_val);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_lookup_elem, NULL);
	will_return(__wrap_bpf_map_delete_elem, FALSE);
	expect_function_call(__wrap_bpf_map_lookup_elem);
	expect_function_call(__wrap_bpf_map_delete_elem);
	rc = delete_ep_1_svc(&ep_key, NULL);
	assert_int_equal(*rc, RPC_TRN_ERROR);
}

/**
 * This is run once before all group tests
 */
static int groupSetup(void **state)
{
	UNUSED(state);
	TRN_LOG_INIT("transitd_unit");
	do_lo_xdp_load();
	return 0;
}

/**
 * This is run once after all group tests
 */
static int groupTeardown(void **state)
{
	UNUSED(state);
	do_lo_xdp_unload();
	TRN_LOG_CLOSE();
	return 0;
}

int main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_update_ep_1_svc),
		cmocka_unit_test(test_delete_ep_1_svc),
		cmocka_unit_test(test_get_ep_1_svc)
	};

	int result = cmocka_run_group_tests(tests, groupSetup, groupTeardown);

	return result;
}
