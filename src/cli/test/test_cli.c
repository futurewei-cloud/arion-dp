// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file test_cli.c
 * @author Wei Yue           (@w-yue)
 *
 * @brief cli unit tests
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
#include <stdio.h>
#include <stdbool.h>
#include <sys/resource.h>

#include "cli/trn_cli.h"

static int clnt_perror_called = 0;

int *__wrap_update_ep_1(rpc_trn_endpoint_batch_t *epb, CLIENT *clnt)
{
	check_expected_ptr(epb);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_load_transit_xdp_1(rpc_trn_xdp_intf_t *itf, CLIENT *clnt)
{
	UNUSED(itf);
	UNUSED(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

int *__wrap_unload_transit_xdp_1(rpc_trn_xdp_intf_t *itf, CLIENT *clnt)
{
	UNUSED(itf);
	UNUSED(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

rpc_trn_endpoint_t *__wrap_get_ep_1(rpc_endpoint_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	rpc_trn_endpoint_t *retval = mock_ptr_type(rpc_trn_endpoint_t *);
	function_called();
	return retval;
}

int *__wrap_delete_ep_1(rpc_endpoint_key_t *argp, CLIENT *clnt)
{
	check_expected_ptr(argp);
	check_expected_ptr(clnt);
	int *retval = mock_ptr_type(int *);
	function_called();
	return retval;
}

static inline int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

static int check_ep_equal(const LargestIntegralType value,
			  const LargestIntegralType check_value_data)
{
	trn_ep_t *ep = (trn_ep_t *)value;
	trn_ep_t *c_ep = (trn_ep_t *)check_value_data;

	assert_int_equal(ep->xdp_ep.key.vni, c_ep->xdp_ep.key.vni);

	assert_int_equal(ep->xdp_ep.key.ip, c_ep->xdp_ep.key.ip);

	assert_int_equal(ep->xdp_ep.val.hip, c_ep->xdp_ep.val.hip);

	assert_memory_equal(ep->xdp_ep.val.mac, c_ep->xdp_ep.val.mac,
		sizeof(ep->xdp_ep.val.mac));

	assert_memory_equal(ep->xdp_ep.val.hmac, c_ep->xdp_ep.val.hmac,
		sizeof(ep->xdp_ep.val.hmac));

	return true;
}

static int check_ep_batch_equal(const LargestIntegralType value,
			  const LargestIntegralType check_value_data)
{
	rpc_trn_endpoint_batch_t *epb = (rpc_trn_endpoint_batch_t *)value;
	rpc_trn_endpoint_batch_t *c_epb = (rpc_trn_endpoint_batch_t *)check_value_data;

	assert_int_equal(epb->rpc_trn_endpoint_batch_t_len, 
		c_epb->rpc_trn_endpoint_batch_t_len);

	trn_ep_t *ep = (trn_ep_t *)epb->rpc_trn_endpoint_batch_t_val;
	trn_ep_t *c_ep = (trn_ep_t *)c_epb->rpc_trn_endpoint_batch_t_val;
	for (unsigned int i = 0; i < epb->rpc_trn_endpoint_batch_t_len; i++) {
		assert_true(check_ep_equal((LargestIntegralType)ep, (LargestIntegralType)c_ep));
	}
	return true;
}

static int check_ep_key_equal(const LargestIntegralType value,
			      const LargestIntegralType check_value_data)
{
	rpc_endpoint_key_t *ep_key =
		(rpc_endpoint_key_t *)value;
	rpc_endpoint_key_t *c_ep_key =
		(rpc_endpoint_key_t *)check_value_data;

	assert_int_equal(ep_key->vni, c_ep_key->vni);

	assert_int_equal(ep_key->ip, c_ep_key->ip);

	return true;
}

static int check_arion_key_equal(const LargestIntegralType value,
				const LargestIntegralType check_value_data)
{
	rpc_trn_arion_key_t *arion_key =
		(rpc_trn_arion_key_t *)value;
	rpc_trn_arion_key_t *c_arion_key =
		(rpc_trn_arion_key_t *)check_value_data;

	assert_int_equal(arion_key->id, c_arion_key->id);

	return true;
}

static void test_trn_cli_update_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 3;
	int update_ep_1_ret_val = 0;

	/* Test data with all fields correct */
	char *argv1[] = { "update-ep", "-j", QUOTE({
				"size": 1,
				 "eps": [
					{
						"vni": 3,
						"ip": 2425262700,
						"hip": 2425262701,
						"mac": 17730434519136,
						"hmac": 105897790545936
					}
				]
			  	}) };

	/* Test data with wrong field type: vni */
	char *argv2[] = { "update-ep", "-j", QUOTE({
				"size": 1,
				 "eps": [
					{
						"vni": "3",
						"ip": 2425262700,
						"hip": 2425262701,
						"mac": 17730434519136,
						"hmac": 105897790545936
					}
				]
			  	}) };

	/* Test data with missing field: ip */
	char *argv3[] = { "update-ep", "-j", QUOTE({
				"size": 1,
				 "eps": [
					{
						"vni": 3,
						"hip": 2425262701,
						"mac": 17730434519136,
						"hmac": 105897790545936
					}
				]
			  	}) };

	/* Test data with malformed json */
	char *argv4[] = { "update-ep", "-j", QUOTE({
				"size": 1,
				 "eps": [
					{
						"vni": 3,
						"ip": "10.0.0.1",
						"hip": 2425262701,
						"mac": 17730434519136,
						"hmac": 105897790545936
					}
				]
			  	}) };

	char hmac[6] = { 0x60, 0x50, 0x40, 0x30, 0x20, 0x10 };
	char mac[6] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60 };
	trn_ep_t ep = {
		.xdp_ep.key.vni = 3,
		.xdp_ep.key.ip = 0x6c928E90,
		.xdp_ep.val.hip = 0x6D928E90,
	};
	memcpy(ep.xdp_ep.val.mac, mac, sizeof(mac));
	memcpy(ep.xdp_ep.val.hmac, hmac, sizeof(hmac));

	rpc_trn_endpoint_batch_t exp_epb = {
		.rpc_trn_endpoint_batch_t_len = 1,
		.rpc_trn_endpoint_batch_t_val = &ep.rpc_ep,
	};

	/* Test call update_ep successfully */
	TEST_CASE("update_ep should succeed with well formed endpoint json input");
	update_ep_1_ret_val = 0;
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, &update_ep_1_ret_val);
	expect_check(__wrap_update_ep_1, epb, check_ep_batch_equal, &exp_epb);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("update_ep should fail if called with wrong field type");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("update_ep is not called with missing required field");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("update_ep is not called malformed json");
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_ep_1 return error*/
	TEST_CASE("update-ep subcommand fails if update_ep_1 returns error");
	update_ep_1_ret_val = -EINVAL;
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, &update_ep_1_ret_val);
	expect_any(__wrap_update_ep_1, epb);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call update_ep_1 return NULL*/
	TEST_CASE("update-ep subcommand fails if update_ep_1 returns NULl");
	expect_function_call(__wrap_update_ep_1);
	will_return(__wrap_update_ep_1, NULL);
	expect_any(__wrap_update_ep_1, epb);
	expect_any(__wrap_update_ep_1, clnt);
	rc = trn_cli_update_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_load_transit_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 3;
	int load_transit_xdp_ret_val = 0;

	/* Test data for nornal requesr */
	char *argv1[] = { "load-transit-xdp", "-j", QUOTE({
				"itf_tenant": "eth0",
				"itf_zgc": "eth1",
				"ibo_port": 8888,
				"debug_mode": 1
			  	}) };

	/* test data with missing field */
	char *argv2[] = { "load-transit-xdp", "-j", QUOTE({
				"itf_zgc": "eth1"
				 }) };

	/* test data with missing optional debug_mode field */
	char *argv3[] = { "load-transit-xdp", "-j", QUOTE({
				"itf_tenant": "eth0",
				"itf_zgc": "eth1",
				"ibo_port": 8888
			  	}) };

	/* Test call load_transit_xdp_1 successfully */
	TEST_CASE("load_transit_xdp should succeed with well formed input");
	load_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	TEST_CASE("load_transit_xdp should fail if itf_tenant field is missing");
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("load_transit_xdp should succeed if optional debug_mode is missing");
	load_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, 0);

	TEST_CASE("load_transit_xdp should fail if rpc returns Error");
	load_transit_xdp_ret_val = -EINVAL;
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, &load_transit_xdp_ret_val);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test call load_transit_xdp return NULL*/
	TEST_CASE("load_transit_xdp should  fail if rpc returns NULl");
	expect_function_call(__wrap_load_transit_xdp_1);
	will_return(__wrap_load_transit_xdp_1, NULL);
	rc = trn_cli_load_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_unload_transit_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 3;
	int unload_transit_xdp_ret_val = 0;

	/* Test cases */
	char *argv1[] = { "load-transit-xdp", "-j", QUOTE({
				"itf_tenant": "eth0",
				"itf_zgc": "eth1",
				"ibo_port": 8888,
				"debug_mode": 1
				}) };

	/* Test call unload_transit_xdp_1 successfully */
	TEST_CASE("unload_transit_xdp should succeed with well formed input");
	unload_transit_xdp_ret_val = 0;
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, &unload_transit_xdp_ret_val);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test call unload_transit_xdp return error */
	TEST_CASE(
		"unload_transit_xdp should fail if rpc returns error");
	unload_transit_xdp_ret_val = -EINVAL;
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, &unload_transit_xdp_ret_val);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call unload_transit_xdp_! return NULL */
	TEST_CASE("unload_transit_xdp should fail if rpc returns NULl");
	expect_function_call(__wrap_unload_transit_xdp_1);
	will_return(__wrap_unload_transit_xdp_1, NULL);
	rc = trn_cli_unload_transit_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_get_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 3;

	char mac[6] = { 1, 2, 3, 4, 5, 6 };
	char hmac[6] = { 6, 5, 4, 3, 2, 1 };

	trn_ep_t get_ep_1_ret_val = {
		.xdp_ep.key.vni = 3,
		.xdp_ep.key.ip = 0x100000a,
		.xdp_ep.val.hip = 0x200000a,
	};
	memcpy(get_ep_1_ret_val.xdp_ep.val.mac, mac, sizeof(char) * 6);
	memcpy(get_ep_1_ret_val.xdp_ep.val.hmac, hmac, sizeof(char) * 6);

	/* Test cases */
	char *argv1[] = { "get-ep", "-j",
			  QUOTE({ "vni": 3, "ip": "10.0.0.1" }) };

	char *argv2[] = { "get-ep", "-j",
			  QUOTE({ "vni": "3", "ip": "10.0.0.1" }) };

	char *argv3[] = { "get-ep", "-j",
			  QUOTE({ "vni": 3 }) };

	char *argv4[] = { "get-ep", "-j",
			  QUOTE({ "vni": 3, "ip": "10.0.0.1", }) };

	rpc_endpoint_key_t exp_ep_key = {
		.ip = 0x100000a,
		.vni = 3,
	};

	/* Test call get_ep successfully */
	TEST_CASE("get_ep should succeed with well formed endpoint json input");
	expect_function_call(__wrap_get_ep_1);
	will_return(__wrap_get_ep_1, &get_ep_1_ret_val);
	expect_check(__wrap_get_ep_1, argp, check_ep_key_equal, &exp_ep_key);
	expect_any(__wrap_get_ep_1, clnt);
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("get_ep should fail if called wrong vni type");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("get_ep should fail if called with missing field");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("get_ep should fail if called with malformed json");
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call get_ep_1 return NULL*/
	TEST_CASE("get-ep subcommand fails if get_ep_1 returns NULl");
	expect_function_call(__wrap_get_ep_1);
	will_return(__wrap_get_ep_1, NULL);
	expect_any(__wrap_get_ep_1, argp);
	expect_any(__wrap_get_ep_1, clnt);
	rc = trn_cli_get_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}

static void test_trn_cli_delete_ep_subcmd(void **state)
{
	UNUSED(state);
	int rc;
	int argc = 3;

	/* Test cases */
	char *argv1[] = { "delete-ep", "-j",
			  QUOTE({ "vni": 3, "ip": "10.0.0.1" }) };

	char *argv2[] = { "delete-vni", "-j",
			  QUOTE({ "vni": "3", "ip": "10.0.0.1" }) };

	char *argv3[] = { "delete-ep", "-j",
			  QUOTE({ "vni": 3 }) };

	char *argv4[] = { "delete-ep", "-j",
			  QUOTE({ "vni": 3, "ip": [10.0.0.2] }) };

	rpc_endpoint_key_t exp_ep_key = {
		.ip = 0x100000a,
		.vni = 3,
	};

	int delete_ep_1_ret_val = 0;
	/* Test call delete_ep_1 successfully */
	TEST_CASE("delete_ep_1 should succeed with well formed endpoint json input");
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, &delete_ep_1_ret_val);
	expect_check(__wrap_delete_ep_1, argp, check_ep_key_equal, &exp_ep_key);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, 0);

	/* Test parse ep input error*/
	TEST_CASE("delete_ep_1 should fail if called with wrong vni type");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv2);
	assert_int_equal(rc, -EINVAL);

	TEST_CASE("delete_ep_1 should fail if called with missing required field");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv3);
	assert_int_equal(rc, -EINVAL);

	/* Test parse ep input error 2*/
	TEST_CASE("delete_ep_1 should fail if called with malformed json");
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv4);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ep_1 return error*/
	TEST_CASE("delete-ep should fail if rpc returns error");
	delete_ep_1_ret_val = -EINVAL;
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, &delete_ep_1_ret_val);
	expect_any(__wrap_delete_ep_1, argp);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);

	/* Test call delete_ep_1 return NULL*/
	TEST_CASE("delete-ep should fail if rpc returns NULL");
	expect_function_call(__wrap_delete_ep_1);
	will_return(__wrap_delete_ep_1, NULL);
	expect_any(__wrap_delete_ep_1, argp);
	expect_any(__wrap_delete_ep_1, clnt);
	rc = trn_cli_delete_ep_subcmd(NULL, argc, argv1);
	assert_int_equal(rc, -EINVAL);
}


int main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_trn_cli_update_ep_subcmd),
		cmocka_unit_test(test_trn_cli_load_transit_subcmd),
		cmocka_unit_test(test_trn_cli_unload_transit_subcmd),
		cmocka_unit_test(test_trn_cli_get_ep_subcmd),
		cmocka_unit_test(test_trn_cli_delete_ep_subcmd),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
