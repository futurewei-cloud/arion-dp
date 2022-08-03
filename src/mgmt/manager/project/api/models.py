# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020-2022 The Authors.
# Authors: Bin Liang  <@liangbin>
#          Wei Yue    <@w-yue>
#
# Summary: Arion DB schema for NBI API test
#
import datetime

from flask import current_app
from sqlalchemy.sql import func

from project import db
from hazelcast.serialization.api import IdentifiedDataSerializable


class ArionNode(IdentifiedDataSerializable):
    def __init__(self,zgc_id=None, description=None, name=None, ip_control=None, id_control=None, pwd_control=None, inf_tenant=None, mac_tenant=None, inf_zgc=None, mac_zgc=None):
        self.zgc_id = zgc_id
        self.description = description
        self.name = name
        self.ip_control = ip_control
        self.id_control = id_control
        self.pwd_control = pwd_control
        self.inf_tenant = inf_tenant
        self.mac_tenant = mac_tenant
        self.inf_zgc = inf_zgc
        self.mac_zgc = mac_zgc
    
    def get_type_id(self):
        return 3
    
    def get_class_id(self):
        return 3
    
    def get_factory_id(self):
        return 1
    
    def write_data(self, object_data_output):
        object_data_output.write_string(self.zgc_id)
        object_data_output.write_string(self.description)
        object_data_output.write_string(self.name)
        object_data_output.write_string(self.ip_control)
        object_data_output.write_string(self.id_control)
        object_data_output.write_string(self.pwd_control)
        object_data_output.write_string(self.inf_tenant)
        object_data_output.write_string(self.mac_tenant)
        object_data_output.write_string(self.inf_zgc)
        object_data_output.write_string(self.mac_zgc)
    
    def read_data(self, object_data_input):
        self.zgc_id = object_data_input.read_string()
        self.description = object_data_input.read_string()
        self.name = object_data_input.read_string()
        self.ip_control = object_data_input.read_string()
        self.id_control = object_data_input.read_string()
        self.pwd_control = object_data_input.read_string()
        self.inf_tenant = object_data_input.read_string()
        self.mac_tenant = object_data_input.read_string()
        self.inf_zgc = object_data_input.read_string()
        self.mac_zgc = object_data_input.read_string()

class VPC(IdentifiedDataSerializable):
    def __init__(self, vpc_id=None, vni=None):
        self.vpc_id = vpc_id
        self.vni = vni
    
    def get_type_id(self):
        return 4
    
    def get_class_id(self):
        return 4
    
    def get_factory_id(self):
        return 1
    
    def write_data(self, object_data_output):
        object_data_output.write_string(self.vpc_id)
        object_data_output.write_int(self.vni)
    
    def read_data(self, object_data_input):
        self.vpc_id = object_data_input.read_string()
        self.vni = object_data_input.read_int()
class ArionGatewayCluster(IdentifiedDataSerializable):
    def __init__(self, name=None, description=None, ip_start=None, ip_end=None, port_ibo=None, overlay_type=None):
        self.name = name
        self.description = description
        self.ip_start = ip_start
        self.ip_end = ip_end
        self.port_ibo = port_ibo
        self.overlay_type = overlay_type
        
    def get_type_id(self):
        return 2
    
    def get_class_id(self):
        return 2
    
    def get_factory_id(self):
        return 1
    
    def write_data(self, object_data_output):
        object_data_output.write_string(self.name)
        object_data_output.write_string(self.description)
        object_data_output.write_string(self.ip_start)
        object_data_output.write_string(self.ip_end)
        object_data_output.write_int(self.port_ibo)
        object_data_output.write_string(self.overlay_type)
    
    def read_data(self, object_data_input):
        self.name = object_data_input.read_string()
        self.description = object_data_input.read_string()
        self.ip_start = object_data_input.read_string()
        self.ip_end = object_data_input.read_string()
        self.port_ibo = object_data_input.read_int()
        self.overlay_type = object_data_input.read_string()

class RoutingRule(IdentifiedDataSerializable):
    def __init__(self, id=None, mac=None, ariongroup=None, hostmac=None, hostip=None, ip=None, vni=None, version=None):
        self.id = id
        self.mac = mac
        self.ariongroup = ariongroup
        self.hostmac = hostmac
        self.hostip = hostip
        self.ip = ip
        self.vni = vni
        self.version = version

    def get_type_id(self):
        return 1

    def get_class_id(self):
        return 1

    def get_factory_id(self):
        return 1

    def write_data(self, object_data_output):
        object_data_output.write_string(self.id)
        object_data_output.write_string(self.mac)
        object_data_output.write_string(self.ariongroup)
        object_data_output.write_string(self.hostmac)
        object_data_output.write_string(self.hostip)
        object_data_output.write_string(self.ip)
        object_data_output.write_int(self.vni)
        object_data_output.write_int(self.version)

    def read_data(self, object_data_input):
        self.id = object_data_input.read_string()
        self.mac = object_data_input.read_string()
        self.ariongroup = object_data_input.read_string()
        self.hostmac = object_data_input.read_string()
        self.hostip = object_data_input.read_string()
        self.ip = object_data_input.read_string()
        self.vni = object_data_input.read_int()
        self.version = object_data_input.read_int()


class Book(db.Model):

    __tablename__ = 'books'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    title = db.Column(db.String(255), nullable=False)
    author = db.Column(db.String(255), nullable=False)
    read = db.Column(db.Boolean(), default=False, nullable=False)

    def __init__(self, title, author, read):
        self.title = title
        self.author = author
        self.read = read

    def to_json(self):
        return {
            'id': self.id,
            'title': self.title,
            'author': self.author,
            'read': self.read
        }


class Zgc(db.Model):

    __tablename__ = 'zgcs'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    zgc_id = db.Column(db.String(64), unique=True, nullable=False)
    name = db.Column(db.String(128), unique=True, nullable=False)
    description = db.Column(db.String(255), nullable=True)
    ip_start = db.Column(db.String(16), nullable=False)
    ip_end = db.Column(db.String(16), nullable=False)
    port_ibo = db.Column(db.String, default='8300')
    overlay_type = db.Column(db.String(16), default='vxlan')
    nodes = db.relationship("Node", backref="zgcs")
    vpcs = db.relationship("Vpc", backref="zgcs")

    def to_json(self):
        return {
            'id': self.id,
            'zgc_id': self.zgc_id,
            'name': self.name,
            'description': self.description,
            'ip_start': self.ip_start,
            'ip_end': self.ip_end,
            'port_ibo': self.port_ibo,
            'overlay_type': self.overlay_type,
            'nodes': [node.to_json() for node in self.nodes],
            'vpcs': [vpc.to_json() for vpc in self.vpcs]
        }


class Gw(db.Model):

    __tablename__ = 'gws'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    node_id = db.Column(db.String(64), db.ForeignKey('nodes.node_id'),
                       nullable=False)
    ip = db.Column(db.String, nullable=False)
    mac = db.Column(db.String, nullable=False)

    def to_json(self):
        return {
            'id': self.id,
            'ip': self.ip,
            'mac': self.mac,
            'node_id': self.node_id
        }


class Node(db.Model):

    __tablename__ = 'nodes'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    zgc_id = db.Column(db.String(64), db.ForeignKey('zgcs.zgc_id'),
                       nullable=False)
    node_id = db.Column(db.String(64), unique=True, nullable=False)
    name = db.Column(db.String(128), unique=True, nullable=False)
    description = db.Column(db.String(255), nullable=True)
    ip_control = db.Column(db.String(16), nullable=False)
    id_control = db.Column(db.String(64), nullable=False)
    pwd_control = db.Column(db.String(64), nullable=False)
    inf_tenant = db.Column(db.String(16), nullable=False)
    mac_tenant = db.Column(db.String(18), nullable=False)
    inf_zgc = db.Column(db.String(16), nullable=False)
    mac_zgc = db.Column(db.String(18), nullable=False)
    gws = db.relationship("Gw", backref="node")

    def to_json(self):
        return {
            'id': self.id,
            'zgc_id': self.zgc_id,
            'node_id': self.node_id,
            'name': self.name,
            'description': self.description,
            'ip_control': self.ip_control,
            'inf_tenant': self.inf_tenant,
            'inf_zgc': self.inf_zgc,
            'gws': [gw.to_json() for gw in self.gws]
        }



class Vpc(db.Model):

    __tablename__ = 'vpcs'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    zgc_id = db.Column(db.String(64), db.ForeignKey('zgcs.zgc_id'),
                       nullable=False)
    vpc_id = db.Column(db.String(64), unique=True, nullable=False)
    vni = db.Column(db.Integer, nullable=False)
    ports = db.relationship("Port", backref="vpcs", lazy=True)

    def to_json(self):
        return {
            'id': self.id,
            'zgc_id': self.zgc_id,
            'vpc_id': self.vpc_id,
            'vni': self.vni,
            'ports': [port.to_json() for port in self.ports]
        }


class Host(db.Model):

    __tablename__ = 'hosts'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    host_id = db.Column(db.String(64), unique=True, nullable=False)
    mac_node = db.Column(db.String(18), nullable=False)
    ip_node = db.Column(db.String(16), nullable=False)
    ports = db.relationship("Port", backref="hosts", lazy=True)

    def to_json(self):
        return {
            'id': self.id,
            'host_id': self.host_id,
            'mac_node': self.mac_node,
            'ip_node': self.ip_node,
            'ports': [port.to_json() for port in self.ports]
        }


class Port(db.Model):

    __tablename__ = 'ports'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    port_id = db.Column(db.String(64), unique=True, nullable=False)
    mac_port = db.Column(db.String(18), nullable=False)
    vpc_id = db.Column(db.String(64), db.ForeignKey(
        'vpcs.vpc_id'), nullable=False)
    host_id = db.Column(db.String(64), db.ForeignKey(
        'hosts.host_id'), nullable=False)
    eps = db.relationship("EP", backref="ports", lazy=True)

    def to_json(self):
        return {
            'id': self.id,
            'port_id': self.port_id,
            'mac_port': self.mac_port,
            'vpc_id': self.vpc_id,
            'host_id': self.host_id,
            'eps': [ep.to_json() for ep in self.eps]
        }


class EP(db.Model):

    __tablename__ = 'eps'

    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    ip = db.Column(db.String(16), nullable=False)
    vip = db.Column(db.String(16), nullable=False)
    port_id = db.Column(db.String(64), db.ForeignKey(
        'ports.port_id'), nullable=False)

    def to_json(self):
        return {
            'id': self.id,
            'ip': self.ip,
            'vip': self.vip,
            'port_id': self.port_id
        }
