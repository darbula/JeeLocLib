from pymote import *
from pymote.utils.tree import get_root_node
from numpy import array, average, arange
import struct
import re
from networkx.drawing.nx_pylab import draw_networkx_edges
import matplotlib
import matplotlib.pyplot as plt

MAX_NODES        = 16
MAX_NEIGHBORS    = 7
MAX_MEASUREMENTS = 112

payload_header = {
    "fields": ["type", "src_id"],
    "format": "BB",
    }

payload_body = {
    0: {"name": "Reset"},
    1: {
        "name": "Token",
        "fields": ["donep1",],
        "format": "B",
        },
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


def get_scale_translation(pos, dim=(1,598)):
    x_min = x_max = float(pos[1])
    y_min = y_max = float(pos[2])

    for nid, x, y in zip(*(iter(pos),) * 3):
        x = float(x)
        y = float(y)
        if x<x_min: x_min=x
        if x>x_max: x_max=x
        if y<y_min: y_min=y
        if y>y_max: y_max=y
    delta_x = x_max-x_min
    delta_y = y_max-y_min
    if delta_x==0 or delta_y==0:
        raise Exception("Pogresne pozicije: %s" % str(pos))
    if delta_x>delta_y:
        s = dim[1]/delta_x
    else:
        s = dim[1]/delta_y
    tx = -x_min*s+dim[0]
    ty = -y_min*s+dim[0]
    return (s, array([tx, ty]))


def get_log_data(name, logfile='rf12demo.log'):
    """ Get all adjacent log lines with given name from the end. """
    with open(logfile, 'r') as f:
        log_lines = f.readlines()
    started = 0
    lines = []
    for line in reversed(log_lines):
        if name in line.split():
            lines.insert(0, line)
            started = 1
        if started and name not in line.split():
            break
    data = map(lambda (i, d): (int(i), d.split(", ")),
            [re.search("\[(\d*).*\]: (.*)", line).groups() for line in lines])
    return data


