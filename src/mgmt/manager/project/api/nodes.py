# SPDX-License-Identifier: MIT
# Copyright (c) 2020-2022 The Authors.
#
# Authors: Bin Liang <@liangbin>
#          Wei Yue   <@w-yue>
#
# Summary: Arion Network node table for NBI API
#


import os
import uuid
import logging
import json
import operator
import time
from flask import (
    Blueprint, jsonify, request
)
from kubernetes.client.rest import ApiException
from kubernetes import client, config
from project.api.models import ArionNode, Node, Zgc, Gw
from project import db
from project.api.utils import getGWsFromIpRange, get_mac_from_ip
from project.api.settings import activeZgc, zgc_cidr_range, node_ips
from common.rpc import TrnRpc

logger = logging.getLogger()
config.load_incluster_config()
obj_api = client.CustomObjectsApi()
nodes_blueprint = Blueprint('nodes', __name__)


def update_droplet(droplet):
    try:
        droplet['metadata'].pop('creationTimestamp', None)
        # droplet['metadata'].pop('resourceVersion', None)
        droplet['metadata'].pop('selfLink', None)
        droplet['metadata'].pop('uid', None)
        
        update_response = obj_api.replace_namespaced_custom_object(group='arion.com',
                                                                   version='v1',
                                                                   plural='droplets',
                                                                   namespace='default', 
                                                                   name=droplet['metadata']['name'],
                                                                   body=droplet)
        logger.info('Response for update droplet: {}'.format(update_response))
        droplet_update_itf_config(droplet['metadata']['labels']['node_ip'], droplet)
    except ApiException as e:
        logger.error('Exception when updating existing droplet: {}'.format(e))

def delete_droplet(name):
    try:
        delete_response = obj_api.delete_namespaced_custom_object(group='arion.com',
                                                                  version='v1',
                                                                  namespace='default',
                                                                  plural='droplets', 
                                                                  name=name,
                                                                  body=client.V1DeleteOptions(),
                                                                  propagation_policy="Orphan")
        logger.info('Response for delete droplet: {}'.format(delete_response))
    except ApiException as e:
        logger.error('Exception when deleting droplet: {}\n{}'.format(name, e))

def create_droplet(name, ip, mac, itf, node_ip, network, zgc_id):
    droplet_body = dict()
    meta_data = dict()
    meta_data['name'] = name
    meta_data['labels'] = dict()
    meta_data['labels']['zgc_id'] = zgc_id
    meta_data['labels']['node_ip'] = node_ip
    meta_data['labels']['network'] = network
    spec = dict()
    spec['ip'] = ip
    spec['mac'] = mac
    spec['status'] = 'Init'
    spec['itf'] = itf
    spec['network'] = network
    spec['zgc_id'] = zgc_id
    droplet_body['metadata'] = meta_data
    droplet_body['spec'] = spec
    droplet_body['apiVersion'] = 'arion.com/v1'
    droplet_body['kind'] = 'Droplet'
    try:
        create_response = obj_api.create_namespaced_custom_object(group='arion.com',
                                                                  version='v1',
                                                                  namespace='default',
                                                                  plural='droplets', 
                                                                  body=droplet_body)
        logger.info('Response for create droplet: {}'.format(create_response))
        droplet_update_itf_config(node_ip, droplet_body)
    except ApiException as e:
        logger.error('Exception when Creating droplets: {}'.format(e))

def droplet_update_itf_config(node_ip, droplet):
    entrances = []
    ips = droplet.get('spec', {}).get('ip')
    macs = droplet.get('spec', {}).get('mac')
    itf = None

    for idx, ip in enumerate(ips):
        entrance = {'ip': ip, 'mac': macs[idx]}
        entrances.append(entrance)
        if itf is None:
            itf = droplet.get('spec',{}).get('itf')
            logger.info(f'Current itf: {itf}')

    itf_conf = {
        'interface': itf,
        'num_entrances': len(ips),
        'entrances': entrances
    }
    logger.info('Sending RPC: {}-{}'.format(node_ip, itf_conf))
    rpc = TrnRpc(node_ip)
    rpc.update_droplet(itf_conf)
    del rpc

def node_load_transit_xdp(ip, inf_tenant, inf_zgc):
    node_ips[ip] = ip
    logger.info('Sending RPC: {}-{} {}'.format(ip, inf_tenant, inf_zgc))
    rpc = TrnRpc(ip)
    rpc.load_transit_xdp(inf_tenant, inf_zgc, int(activeZgc["port_ibo"]))
    del rpc


def node_unload_transit_xdp(ip, itf_tenant, itf_zgc):
    node_ips.pop(ip, None)
    rpc = TrnRpc(ip)
    rpc.unload_transit_xdp(ip, itf_tenant, itf_zgc)
    del rpc


def update_existing_nodes_gw_ips_macs(node_ip : str, ips : list, macs : list  ):
    node_to_update = Node.query.filter_by(ip_control=node_ip).first()
    print(f'Found this node: {node_to_update.to_json()}')
    if node_to_update is None:
        print(f'Failed to find Node with ip_control {node_ip}!!!!!!!')
        return
    else:
        for gw in node_to_update.gws:
            print(f'Deleting this GW: {gw.to_json()}')
            db.session.delete(gw)
        db.session.commit()
        for i in range(len(ips)):
            gw_to_add = Gw(node_id=node_to_update.node_id, ip=ips[i], mac=macs[i])
            print(f'Adding this GW: {gw_to_add.to_json()}')
            node_to_update.gws.append(gw_to_add)
        print(f'Update this node with following info: {node_to_update.to_json()}')
        db.session.commit()
        print(f'Updated Node with ip_control {node_ip} with updated gw ips and macs.')
    return


def set_up_node_from_hazelcast(arion_node: ArionNode):
    start_time = time.time()
    logger.debug('From Hazelcast: Start to make one node and one droplet for this node.')
    new_node = Node()
    node_id = str(uuid.uuid4())
    new_node.node_id = node_id
    # Set the node's zgc_id to active_zgc
    new_node.zgc_id = activeZgc["zgc_id"]#arion_node.zgc_id
    new_node.name = arion_node.name
    new_node.description = arion_node.description
    new_node.ip_control = arion_node.ip_control
    new_node.id_control = arion_node.id_control
    new_node.pwd_control = arion_node.pwd_control
    new_node.inf_tenant = arion_node.inf_tenant
    new_node.mac_tenant = arion_node.mac_tenant
    new_node.inf_zgc = arion_node.inf_zgc
    new_node.mac_zgc = arion_node.mac_zgc
    node_load_transit_xdp(new_node.ip_control, new_node.inf_tenant, new_node.inf_zgc)
    # Try to get the existing droplets        
    all_droplets_in_zgc = obj_api.list_cluster_custom_object(group='arion.com',
                                                                version='v1',
                                                                plural='droplets',
                                                                label_selector='zgc_id='+new_node.zgc_id+',network=tenant'
                                                            )['items']
    zgc_ip_list = new_node.ip_control.split('.')
        
    ip_range = zgc_cidr_range.split('/')
    cidr_list = ip_range[0].split('.')

    # Only zgc_cidr postfix 8, 16 and 24 are supported
    for i in range(int(ip_range[1])//8):
        zgc_ip_list[i] = cidr_list[i]

    zgc_ip = '.'.join(zgc_ip_list)
    zgc_mac = new_node.mac_zgc

    if len(all_droplets_in_zgc)>0:
        total_ip = 0
        droplet_that_can_give_ip_mac = []
        for droplet in all_droplets_in_zgc:
            droplet_spec = droplet['spec']
            droplet_ip_list = droplet_spec['ip']
            droplet_mac_list = droplet_spec['mac']
            total_ip = total_ip + len(droplet_ip_list)
            if len(droplet_ip_list) > 1 and len(droplet_mac_list) > 1:
                droplet_that_can_give_ip_mac.append(droplet)
        number_of_droplets = len(all_droplets_in_zgc)
        number_of_ip_new_droplet_gets = total_ip // (number_of_droplets + 1) 
        logger.info('number of ip new droplets gets {}-{}-{}'.format(number_of_ip_new_droplet_gets, total_ip, number_of_droplets))
        ip_for_new_droplet = []
        modified_droplets = dict()
        if number_of_ip_new_droplet_gets > 0 : # each droplet should have 1 or more ip, assign ip / mac from existing droplets
            current_length_of_ip_list = len(ip_for_new_droplet)
            # sort these droplets in descending order, so droplet with the most IPs will be in the front.
            droplet_that_can_give_ip_mac.sort(key=lambda x : len(x['spec']['ip']), reverse=True)
            # only one IP/mac for other droplets
            while len(ip_for_new_droplet) < 1:
                for droplet in droplet_that_can_give_ip_mac:
                    droplet_spec = droplet['spec']
                    droplet_ip_list = droplet_spec['ip']
                    if len(droplet_ip_list) > 1:
                        droplet_name = droplet['metadata']['name']
                        if droplet_name not in modified_droplets:
                            modified_droplets[droplet_name] = droplet
                        popped_ip = droplet_ip_list.pop()
                        ip_for_new_droplet.append(popped_ip)
                        droplet_mac_list = droplet_spec['mac']
                        mac_from_ip =  get_mac_from_ip(popped_ip)
                        if mac_from_ip in droplet_mac_list:
                            droplet_mac_list.remove(mac_from_ip)
                        else:
                            droplet_mac_list.pop()
                        droplet['spec']['ip'] = droplet_ip_list
                        droplet['spec']['mac'] = droplet_mac_list
                if current_length_of_ip_list == len(ip_for_new_droplet):
                    logger.info('Breaking the while loop as there is no new ip added')
                    break
                else:
                    current_length_of_ip_list = len(ip_for_new_droplet)
            macs_for_new_droplet = [get_mac_from_ip(ip) for ip in ip_for_new_droplet]

            # Each wing node has one droplet, so I need to update the Node with the updated IPs && MACs
            for droplet_name in modified_droplets:
                modified_node_physical_ip = modified_droplets[droplet_name]['metadata']['labels']['node_ip']
                modified_node_gws_ip = modified_droplets[droplet_name]['spec']['ip']
                modified_node_gws_mac = modified_droplets[droplet_name]['spec']['mac']
                update_existing_nodes_gw_ips_macs(modified_node_physical_ip, modified_node_gws_ip,
                                                  modified_node_gws_mac)
                update_droplet(modified_droplets[droplet_name])

            create_droplet(name='droplet-tenant-' + new_node.name.replace('_', "-").lower(), 
                            ip=ip_for_new_droplet, 
                            mac=macs_for_new_droplet,
                            itf=new_node.inf_tenant, 
                            node_ip= new_node.ip_control,
                            network='tenant', 
                            zgc_id=new_node.zgc_id)

            logger.info('Finished updating and adding droplets.')
            if len(new_node.gws) == 0:
                for i in range(len(ip_for_new_droplet)):
                    new_node.gws.append(Gw(node_id=node_id, ip=ip_for_new_droplet[i], mac=macs_for_new_droplet[i]))
        else:
            logger.error('Not enough ip to assign for the new droplet.')
    else:
        logger.info("First nodes in the ZGC cluster, it gets all the IPs")
        zgc = Zgc.query.filter_by(zgc_id=new_node.zgc_id).first()
        if zgc is not None:
            gws = getGWsFromIpRange(zgc.ip_start, zgc.ip_end)
            if len(new_node.gws) == 0:
                for gw in gws:
                    new_node.gws.append(Gw(node_id=node_id, ip=gw['ip'], mac=gw['mac']))
            create_droplet(name='droplet-tenant-' + new_node.name.replace('_', "-").lower(), 
                            ip=[gw['ip'] for gw in gws],
                            mac=[gw['mac'] for gw in gws],
                            itf=new_node.inf_tenant,
                            node_ip= new_node.ip_control,
                            network='tenant',
                            zgc_id=new_node.zgc_id)
        else:
            logger.error("There's no zgc with zgc_id: {} in the database!".format(new_node.zgc_id))
            return jsonify({'error':'No such zgc'})
    # commit change to data at last
    db.session.add(new_node)
    db.session.commit()

    end_time = time.time()
    logger.debug(f'Arion took {end_time - start_time} seconds to make a node and its two droplets.')

@nodes_blueprint.route('/nodes', methods=['GET', 'POST'])
def all_nodes():
    status_code = 200
    if request.method == 'POST':
        logger.debug('Start to make one node and one droplet for this node.')
        start_time = time.time()
        post_data = request.get_json()
        node_id = str(uuid.uuid4())
        post_data['node_id'] = node_id

        node_load_transit_xdp(post_data['ip_control'], post_data['inf_tenant'], post_data['inf_zgc'])

        # Try to get the existing droplets        
        all_droplets_in_zgc = obj_api.list_cluster_custom_object(group='arion.com',
                                                                 version='v1',
                                                                 plural='droplets',
                                                                 label_selector='zgc_id='+post_data['zgc_id']+',network=tenant'
                                                                )['items']
        zgc_ip_list = post_data['ip_control'].split('.')
        
        ip_range = zgc_cidr_range.split('/')
        cidr_list = ip_range[0].split('.')

        # Only zgc_cidr postfix 8, 16 and 24 are supported
        for i in range(int(ip_range[1])//8):
            zgc_ip_list[i] = cidr_list[i]

        zgc_ip = '.'.join(zgc_ip_list)
        zgc_mac = post_data['mac_zgc']

        if len(all_droplets_in_zgc)>0:
            total_ip = 0
            droplet_that_can_give_ip_mac = []
            for droplet in all_droplets_in_zgc:
                droplet_spec = droplet['spec']
                droplet_ip_list = droplet_spec['ip']
                droplet_mac_list = droplet_spec['mac']
                total_ip = total_ip + len(droplet_ip_list)
                if len(droplet_ip_list) > 1 and len(droplet_mac_list) > 1:
                    droplet_that_can_give_ip_mac.append(droplet)
            number_of_droplets = len(all_droplets_in_zgc)
            number_of_ip_new_droplet_gets = total_ip // (number_of_droplets + 1) 
            logger.info('number of ip new droplets gets {}-{}-{}'.format(number_of_ip_new_droplet_gets, total_ip, number_of_droplets))
            ip_for_new_droplet = []
            modified_droplets = dict()
            if number_of_ip_new_droplet_gets > 0 : # each droplet should have 1 or more ip, assign ip / mac from existing droplets
                current_length_of_ip_list = len(ip_for_new_droplet)
                # sort these droplets in descending order, so droplet with the most IPs will be in the front.
                droplet_that_can_give_ip_mac.sort(key=lambda x : len(x['spec']['ip']), reverse=True)
                # only one IP/mac for other droplets
                while len(ip_for_new_droplet) < 1:
                    for droplet in droplet_that_can_give_ip_mac:
                        droplet_spec = droplet['spec']
                        droplet_ip_list = droplet_spec['ip']
                        if len(droplet_ip_list) > 1:
                            droplet_name = droplet['metadata']['name']
                            if droplet_name not in modified_droplets:
                                modified_droplets[droplet_name] = droplet
                            popped_ip = droplet_ip_list.pop()
                            ip_for_new_droplet.append(popped_ip)
                            droplet_mac_list = droplet_spec['mac']
                            mac_from_ip =  get_mac_from_ip(popped_ip)
                            if mac_from_ip in droplet_mac_list:
                                droplet_mac_list.remove(mac_from_ip)
                            else:
                                droplet_mac_list.pop()
                            droplet['spec']['ip'] = droplet_ip_list
                            droplet['spec']['mac'] = droplet_mac_list
                    if current_length_of_ip_list == len(ip_for_new_droplet):
                        logger.info('Breaking the while loop as there is no new ip added')
                        break
                    else:
                        current_length_of_ip_list = len(ip_for_new_droplet)
                macs_for_new_droplet = [get_mac_from_ip(ip) for ip in ip_for_new_droplet]

                # Each wing node has one droplet, so I need to update the Node with the updated IPs && MACs
                for droplet_name in modified_droplets:
                    modified_node_physical_ip = modified_droplets[droplet_name]['metadata']['labels']['node_ip']
                    modified_node_gws_ip = modified_droplets[droplet_name]['spec']['ip']
                    modified_node_gws_mac = modified_droplets[droplet_name]['spec']['mac']
                    update_existing_nodes_gw_ips_macs(modified_node_physical_ip, modified_node_gws_ip, modified_node_gws_mac)
                    update_droplet(modified_droplets[droplet_name])

                create_droplet(name='droplet-tenant-' + post_data['name'].replace('_', "-").lower(), 
                               ip=ip_for_new_droplet, 
                               mac=macs_for_new_droplet,
                               itf=post_data['inf_tenant'], 
                               node_ip=post_data['ip_control'],
                               network='tenant', 
                               zgc_id=post_data['zgc_id'])
                if 'gws' not in post_data:
                    post_data['gws'] = list()
                for i in range(len(ip_for_new_droplet)):
                    post_data['gws'].append(Gw(node_id=node_id, ip=ip_for_new_droplet[i], mac=macs_for_new_droplet[i]))

                logger.info('Finished updating and adding droplets.')
            else:
                logger.error('Not enough ip to assign for the new droplet.')
        else:
            logger.info("First nodes in the ZGC cluster, it gets all the IPs")
            zgc = Zgc.query.filter_by(zgc_id=post_data['zgc_id']).first()
            if zgc is not None:
                gws = getGWsFromIpRange(zgc.ip_start, zgc.ip_end)
                if 'gws' not in post_data:
                    post_data['gws'] = list()
                for gw in gws:
                    post_data['gws'].append(Gw(node_id=node_id, ip=gw['ip'], mac=gw['mac']))
                create_droplet(name='droplet-tenant-' + post_data['name'].replace('_', "-").lower(),
                               ip=[gw['ip'] for gw in gws],
                               mac=[gw['mac'] for gw in gws],
                               itf=post_data['inf_tenant'],
                               node_ip=post_data['ip_control'],
                               network='tenant', 
                               zgc_id=post_data['zgc_id'])
            else:
                logger.error("There's no zgc with zgc_id: {} in the database!".format(post_data['zgc_id']))
                return jsonify({'error':'No such zgc'})
        # commit change to data at last
        db.session.add(Node(**post_data))
        db.session.commit()

        response_object = post_data
        if 'gws' in response_object:
            response_object['gws'] = [gw.to_json() for gw in response_object['gws']]
        end_time = time.time()
        logger.debug(f'Arion took {end_time - start_time} seconds to make a node and its droplets.')
        status_code = 201
    else:
        response_object = [node.to_json() for node in Node.query.all()]
    return jsonify(response_object), status_code

@nodes_blueprint.route('/nodes/ping', methods=['GET'])
def ping_nodes():
    return jsonify({
        'status': 'success',
        'message': 'pong!',
        'container_id': os.uname()[1]
    })


@nodes_blueprint.route('/nodes/<node_id>', methods=['GET', 'PUT', 'DELETE'])
def single_node(node_id):
    node = Node.query.filter_by(node_id=node_id).first()
    status_code = 200
    if request.method == 'GET':
        response_object = node.to_json()
    elif request.method == 'PUT':
        post_data = request.get_json()
        node.zgc_id = post_data.get('zgc_id')
        node.description = post_data.get('description')
        node.ip_control = post_data.get('ip_control')
        node.id_control = post_data.get('id_control')
        node.pwd_control = post_data.get('pwd_control')
        node.inf_tenant = post_data.get('inf_tenant')
        node.mac_tenant = post_data.get('mac_tenant')
        node.inf_zgc = post_data.get('inf_zgc')
        node.mac_zgc = post_data.get('mac_zgc')
        db.session.commit()
        response_object = node.to_json()
    elif request.method == 'DELETE':
        post_data = request.get_json()
        node_name = node.name
        zgc_id = node.zgc_id
        all_droplets_in_zgc = obj_api.list_cluster_custom_object(group='arion.com',
                                                                 version='v1',
                                                                 plural='droplets',
                                                                 label_selector='zgc_id='+zgc_id+',network=tenant'
                                                                )['items']
        droplet_to_remove = None

        tenant_droplet_name = 'droplet-tenant-'+node_name.replace('_', "-").lower()
        zgc_droplet_name = 'droplet-zgc-'+node_name.replace('_', "-").lower()

        for droplet in all_droplets_in_zgc:
            if droplet['metadata']['name'] == tenant_droplet_name:
                droplet_to_remove = droplet
                all_droplets_in_zgc.remove(droplet)
                break

        if droplet_to_remove is None:
            logger.error('Cannot find that droplet with name: {}, thus not deleting anything'.format(tenant_droplet_name))
            return jsonify({'error': 'Droplet not found'})
        else:
            ip_to_assign = droplet_to_remove['spec']['ip']

            ip_amount = len(ip_to_assign)

            number_of_droplets_to_assign = len(all_droplets_in_zgc)

            # Sort these droplets in ascending order, so the droplet with the least IPs will be in the front.
            all_droplets_in_zgc.sort(key=lambda x : len(x['spec']['ip']), reverse=False)
            modified_droplets = dict()
            for index in range(ip_amount):
                ip = ip_to_assign[index]
                mac = get_mac_from_ip(ip)
                all_droplets_in_zgc[index % number_of_droplets_to_assign]['spec']['ip'].append(ip)
                all_droplets_in_zgc[index % number_of_droplets_to_assign]['spec']['mac'].append(mac)
                modified_droplet_name = all_droplets_in_zgc[index % number_of_droplets_to_assign]['metadata']['name']
                if modified_droplet_name not in modified_droplets:
                    modified_droplets[modified_droplet_name] = all_droplets_in_zgc[index % number_of_droplets_to_assign]

            delete_droplet(name=tenant_droplet_name)
            delete_droplet(name=zgc_droplet_name)

            for droplet_name in modified_droplets:
                update_droplet(modified_droplets[droplet_name])

        node_unload_transit_xdp(node.ip_control, node.inf_tenant, node.zgc_id)

        db.session.delete(node)
        db.session.commit()
        response_object = {}
        status_code = 204
    return jsonify(response_object), status_code


if __name__ == '__main__':
    app.run()
