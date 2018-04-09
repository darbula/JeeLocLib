from numpy import array
import re


MAX_NODES = 16
MAX_NEIGHBORS = 7
MAX_MEASUREMENTS = 112

payload_header = {
    "fields": ["type", "src_id"],
    "format": "BB",
    }

payload_body = {
    0: {"name": "Reset"},
    1: {
        "name": "Token",
        "fields": ["counter", ],
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
