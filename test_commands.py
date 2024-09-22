#!/usr/bin/env python3

import shlex
import subprocess
import sys
import time

from colorama import Fore, init
from tqdm import tqdm

init(autoreset=True)


def start_server():
    """Starts a server process.

    Returns:
        the server process.
    """
    process = subprocess.Popen(
        ["bazel", "run", "//server:server"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    time.sleep(1)
    return process


CASES = r"""
$ bazel run //client:client -- zscore asdf n1
(nil)
$ bazel run //client:client -- zquery xxx 1 asdf 1 10
(arr) len=0
(arr) end
$ bazel run //client:client -- zadd zset 1 n1
(int) 1
$ bazel run //client:client -- zadd zset 2 n2
(int) 1
$ bazel run //client:client -- zadd zset 1.1 n1
(int) 0
$ bazel run //client:client -- zscore zset n1
(double) 1.1
$ bazel run //client:client -- zquery zset 1 "" 0 10
(arr) len=4
(str) n1
(double) 1.1
(str) n2
(double) 2
(arr) end
$ bazel run //client:client -- zquery zset 1.1 "" 1 10
(arr) len=2
(str) n2
(double) 2
(arr) end
$ bazel run //client:client -- zquery zset 1.1 "" 2 10
(arr) len=0
(arr) end
$ bazel run //client:client -- zrem zset adsf
(int) 0
$ bazel run //client:client -- zrem zset n1
(int) 1
$ bazel run //client:client -- zquery zset 1 "" 0 10
(arr) len=2
(str) n2
(double) 2
(arr) end
"""


server_process = start_server()

commands = []
outputs = []
lines = CASES.splitlines()
if not lines[0].strip():
    lines.pop(0)  # empty first line

for x in lines:
    x = x.strip()
    if not x:
        continue
    if x.startswith("$"):
        commands.append(x[2:])
        outputs.append("")
    else:
        outputs[-1] += x + "\n"

try:
    assert len(commands) == len(outputs)
    for cmd, expected_output in tqdm(zip(commands, outputs), total=len(commands)):
        actual_output = subprocess.check_output(shlex.split(cmd)).decode("utf-8")

        if expected_output == actual_output:
            print(Fore.GREEN + f"✔ Test passed: {cmd}")

        assert (
            expected_output == actual_output
        ), f"command: {cmd} returned: {actual_output} but expected {expected_output}"

except Exception as e:
    print(Fore.RED + f"✖ Test failed: {e}")
    sys.exit(1)  # make sure the build fails

finally:
    server_process.terminate()
