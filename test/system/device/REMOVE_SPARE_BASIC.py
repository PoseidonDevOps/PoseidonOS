#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import test_result
import json
import MOUNT_ARRAY_BASIC
SPARE = MOUNT_ARRAY_BASIC.SPARE

def check_result():
    out = cli.get_pos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "NORMAL":
        list = cli.array_info("")
        data = json.loads(list)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == SPARE :
                return "fail", list
        return "pass", out
    return "fail", out

def set_result(detail):
    code = json_parser.get_response_code(detail)
    if code == 0:
        result, out = check_result()
    else:
        result = "fail"
        out = detail

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_ARRAY_BASIC.execute()
    out = cli.remove_device(SPARE, "")
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()