from pymote import *
from pymote.utils.tree import get_root_node
from numpy import array, average, arange
import struct
import re
from networkx.drawing.nx_pylab import draw_networkx_edges
import matplotlib
import matplotlib.pyplot as plt

MAX_NODES = 16
MAX_NEIGHBORS = 7
MAX_MEASUREMENTS = 112

payload_header = {
    "fields": ["type", "src_id"],
    "format": "BB",
}

payload_body = {
    0: {"name": "Spontaneously"},
    1: {"name": "Token"},
    2: {"name": "Return"},
    3: {"name": "Backedge"},
    4: {"name": "Forwardedge"},
    5: {"name": "NeighborDetect"},
    6: {"name": "NeighborConfirm"},
    }


def int2hex(i):
    return "{h:0>2}".format(h=hex(i)[2:])


def log_package_function(jeelink, data):
    # data = jeelink.parse_received_information("OKX 04020401008B7AEF42")
    # package_log_function(jeelink, data)
    # AoAresponse [2 -> all]: 1, 0, 34.7697067261
    if "type" in data:
        name = jeelink.payload_body[data["type"]]["name"]
        fields = jeelink.payload_body[data["type"]].get("fields", [])
    src = str(data.get("src_id", ""))
    dst = "all" if data["hdr_dst_id"] is None else str(data["hdr_dst_id"])
    payload_body = ""
    if data["hdr_is_ack"]:
        name = "Ack"
    payload_body = ": " + payload_body if payload_body else ""
    log = "%s [%s -> %s]%s" % (name, src, dst, payload_body)
    jeelink.logger.info(log)
