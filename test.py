import json

from pylogmon import *

def find_sub_object(obj, key):
    """Recursively search for a sub-object containing the key."""
    if isinstance(obj, dict):
        if key in obj:
            return obj[key]
        for v in obj.values():
            result = find_sub_object(v, key)
            if result is not None:
                return result
    return None

def append(path, segment):
    return path + '.' + segment if path else segment

def compare(dct, obj, path=None):
    if isinstance(dct, dict):
        for k, v in dct.items():
            s = find_sub_object(obj, k)
            if not s:
                return False, path, "element: " + k + " not found"
            r, p, m = compare(v, s, append(path, k))
            if not r:
                return r, p, m
        return True, None, None
    return dct == obj, path, "expecting: " + str(dct) + ", got: " + str(obj)

exp = {"EnterOrder":{"Quantity":1602, "TimeInForce": {"Value": "3"}, "Capacity": "4", "CrossType": {"Value": "C"}}}
#exp = {"EnterOrder":{"Quantity":1602, "TimeInForce":7}}
#exp = {"EnterOrder":{"Quantity":1602, "TimeInForce": {"Value": "3"}, "Dummy": 4}}

def cb(str):
    data = json.loads(str.strip())
    res, path, msg = compare(exp, data)
    if not res:
        print("At <" + path + ">, " + msg)

m = LogMonitor(['logs/log1.log', 'logs/log2.log'])
m.callback = cb

while True:
    user_input = input("Enter text (or 'q' to quit): ")
    if user_input.lower() == 'q':
        print("Exiting...")
        break

