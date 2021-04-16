#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import pos
import cli
import test_result
import json
import ADD_SPARE_BASIC

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    ADD_SPARE_BASIC.execute()
    out = cli.add_device(ADD_SPARE_BASIC.SPARE_DEV, ADD_SPARE_BASIC.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()