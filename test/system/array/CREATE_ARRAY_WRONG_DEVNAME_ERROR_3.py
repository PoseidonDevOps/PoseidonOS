#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import ibofos
import cli
import test_result

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    clear_result()
    ibofos.start_ibofos()
    cli.scan_device()
    out = cli.create_array("uram0", "wrong-0,unvme-ns-1,unvme-ns-2", "wrong-3", "", "")
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()