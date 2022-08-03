# SPDX-License-Identifier: MIT
# Copyright (c) 2022 The Authors.
#
# Authors: Rio Zhu <@zzxgzgz>
#
# Summary: default setup API
#


import os
import uuid
import logging
import time

from flask import (
    Blueprint, jsonify, request
)
from psycopg2 import connect
from project import db
from project.api.models import Port, Host, EP, RoutingRule, ArionGatewayCluster, ArionNode, VPC
from project.api.settings import node_ips, vnis, hazelcast_ip_port
from project.api.utils import ip_to_int, mac_to_int
from project import db
import hazelcast
from project.api.zgcs import set_up_cluster_from_hazelcast
from project.api.vpcs import set_up_vpc_from_hazelcast
from project.api.nodes import set_up_node_from_hazelcast
from project.api.ports import set_up_ports_in_the_same_vpc_from_hazelcast

logger = logging.getLogger()

# used in creating the hazelcast client, add more elements to the inner map when there is a new added class, the key is the class_id.
global hazelcast_serialization_factory
hazelcast_serialization_factory = {1: {1: RoutingRule, 2: ArionGatewayCluster, 3: ArionNode, 4:VPC}}

hazelcast_client = None

arion_gateway_cluster_map = None

routing_rule_map = None

arion_nodes_map = None

vpc_map = None

default_setup_blueprint = Blueprint('default_setup', __name__)

# setup a default zgc cluster with data GET from Hazelcast
@default_setup_blueprint.route('/default_setup', methods=['GET'])
def setup():
    logger.debug(f'Start getting data from Hazelcast')

    gateway_list = None

    get_data_start_time = time.time()
    connect_to_hazelcast(hazelcast_ip_port)    

    arion_gateway_cluster_set = arion_gateway_cluster_map.entry_set().result()
    logger.debug(f'GOT {len(arion_gateway_cluster_set)} ArionGatewayClusters from Hazelcast')
    
    arion_nodes_set = arion_nodes_map.entry_set().result()
    logger.debug(f'GOT {len(arion_nodes_set)} ArionNodes from Hazelcast')
    
    vpc_set = vpc_map.entry_set().result()
    logger.debug(f'GOT {len(vpc_set)} VPCs from Hazelcast')
    
    routing_rule_set = routing_rule_map.entry_set().result()
    logger.debug(f'GOT {len(routing_rule_set)} RoutingRules from Hazelcast')
    
    get_data_end_time = time.time()
    logger.debug(f'Finished getting data from Hazelcast, it took {get_data_end_time - get_data_start_time} seconds, now start setting up with the data.')
    
    for cluster_key, cluster in arion_gateway_cluster_set:
        set_up_cluster_from_hazelcast(cluster)
        time.sleep(10)
    
    for node_key, node in arion_nodes_set:
        set_up_node_from_hazelcast(node)
        time.sleep(10)
    
    for vpc_key, vpc_value in vpc_set:
        current_vpc : VPC = vpc_value
        vpc_response = set_up_vpc_from_hazelcast(current_vpc)
        response_object = vpc_response["gws"]
        routing_rules_in_current_vpc = []
        time.sleep(10)
        for routing_rule_key, routing_rule_value  in routing_rule_set:
            rule: RoutingRule = routing_rule_value
            if rule.vni == current_vpc.vni:
                routing_rules_in_current_vpc.append(rule)
        set_up_ports_in_the_same_vpc_from_hazelcast(routing_rules_in_current_vpc, current_vpc.vpc_id)
        time.sleep(10)
        
    
    setup_finish_time = time.time()
    
    logger.debug(f'Finished setting up, it took {setup_finish_time - get_data_end_time} seconds..')

    return jsonify(response_object), 201

def connect_to_hazelcast(hazelcast_ip_port):
    global hazelcast_client
    hazelcast_client = hazelcast.HazelcastClient(
        cluster_members=[hazelcast_ip_port], 
        data_serializable_factories=hazelcast_serialization_factory)
    # GET all exsiting maps from Hazelcast
    global arion_gateway_cluster_map
    arion_gateway_cluster_map = hazelcast_client.get_map('com.futurewei.common.model.ArionGatewayCluster')
    
    global arion_nodes_map
    arion_nodes_map = hazelcast_client.get_map('com.futurewei.common.model.ArionNode')
    
    global vpc_map
    vpc_map = hazelcast_client.get_map('com.futurewei.common.model.VPC')
    
    global routing_rule_map
    routing_rule_map = hazelcast_client.get_map('com.futurewei.common.model.NeighborRule')
    
    def added(event):
        number_of_entries_in_map = len(routing_rule_map.entry_set().result())
        logger.info(
            f'Entry is added to the map, now the map has {number_of_entries_in_map} entries')

    def removed(event):
        number_of_entries_in_map = len(routing_rule_map.entry_set().result())
        logger.info(
            f'Entry is removed from the map, now the map has {number_of_entries_in_map} entries')

    # add listeners to the map, listeners will be called when an entries is added/removed from the map
    arion_gateway_cluster_map.add_entry_listener(include_value=True, added_func=added)
    arion_gateway_cluster_map.add_entry_listener(include_value=True, removed_func=removed)

    arion_nodes_map.add_entry_listener(include_value=True, added_func=added)
    arion_nodes_map.add_entry_listener(include_value=True, removed_func=removed)

    vpc_map.add_entry_listener(include_value=True, added_func=added)
    vpc_map.add_entry_listener(include_value=True, removed_func=removed)

    routing_rule_map.add_entry_listener(include_value=True, added_func=added)
    routing_rule_map.add_entry_listener(include_value=True, removed_func=removed)
    logger.info('Finished setting up with Hazelcast.')
    return

