#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import ibofos
import cli
import test_result
import json
import MOUNT_ARRAY_NO_SPARE_3

SPARE_DEV = MOUNT_ARRAY_NO_SPARE_3.REMAINING_DEV

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(out):
    data = json.loads(out)
    list = []
    for item in data['Response']['result']['data']['devicelist']:
        if item['type'] == "SPARE" and item['name'] == SPARE_DEV :
            return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    if code == 0:
        out = cli.array_info("")
        result = check_result(out)
    else:
        result = "fail"
        out = detail

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_ARRAY_NO_SPARE_3.execute()
    out = cli.add_device(SPARE_DEV, "")
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()