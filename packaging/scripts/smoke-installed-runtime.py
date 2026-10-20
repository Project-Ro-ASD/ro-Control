#!/usr/bin/env python3
"""Exercise the installed package, not the build tree, without hardware writes."""
import ast
import json
import pathlib
import subprocess
import time
import xml.etree.ElementTree as ET


def run(*args):
    return subprocess.check_output(args, text=True, timeout=60)


# Pure queries and no privileged driver/package modifications.
for command in ('status', 'diagnostics'):
    assert isinstance(json.loads(run('ro-control', command, '--json')), dict)
helper = '/usr/libexec/ro-control-helper'
run(helper, 'env', 'LANG=C', 'dnf', '--version')
policy = ET.parse('/usr/share/polkit-1/actions/io.github.ProjectRoASD.rocontrol.policy')
action = policy.find("action[@id='io.github.ProjectRoASD.rocontrol.manage-drivers']")
assert action is not None
assert action.find("annotate[@key='org.freedesktop.policykit.exec.path']").text == helper
for scope in ('system', 'user'):
    unit = pathlib.Path(f'/usr/lib/systemd/{scope}/ro-control.service').read_text()
    assert 'ExecStart=/usr/bin/ro-control --daemon' in unit

# The parent supplies a private dbus-run-session; wait for the real installed
# daemon to answer, rather than accepting a startup banner as proof of IPC.
with pathlib.Path('/tmp/ro-control-daemon.log').open('w') as log:
    daemon = subprocess.Popen(['ro-control', '--daemon'], stdout=log, stderr=log)
    try:
        deadline = time.monotonic() + 30
        while True:
            result = subprocess.run([
                'gdbus', 'call', '--session',
                '--dest', 'io.github.ProjectRoASD.rocontrol',
                '--object-path', '/io/github/ProjectRoASD/rocontrol',
                '--method', 'io.github.ProjectRoASD.rocontrol.GetTelemetry',
            ], capture_output=True, text=True, timeout=5)
            if result.returncode == 0:
                assert isinstance(json.loads(ast.literal_eval(result.stdout)[0]), dict)
                break
            if daemon.poll() is not None or time.monotonic() > deadline:
                raise RuntimeError(pathlib.Path('/tmp/ro-control-daemon.log').read_text())
            time.sleep(0.25)
    finally:
        daemon.terminate()
        daemon.wait(timeout=10)
print('Installed CLI JSON, helper locale wrapper, policy paths, unit paths and live D-Bus telemetry passed')
