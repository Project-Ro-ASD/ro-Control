#!/usr/bin/env python3
"""Exercise the installed GUI's single-instance lifecycle in a private session."""
import os
from pathlib import Path
import subprocess
import tempfile
import time


def stop(process):
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=5)


with tempfile.TemporaryDirectory(prefix='ro-control-gui-') as directory:
    root = Path(directory)
    environment = dict(os.environ, QT_QPA_PLATFORM='offscreen',
                       QT_QUICK_BACKEND='software', QT_QUICK_CONTROLS_STYLE='Basic',
                       XDG_RUNTIME_DIR=str(root), XDG_CONFIG_HOME=str(root / 'config'),
                       XDG_CACHE_HOME=str(root / 'cache'))
    log_path = Path('/tmp/ro-control-gui.log')
    with log_path.open('w') as log:
        primary = subprocess.Popen(['ro-control'], env=environment, stdout=log, stderr=log)
        try:
            deadline = time.monotonic() + 15
            while not (root / 'ro-control-instance/gui.lock').exists():
                assert primary.poll() is None, 'Primary GUI exited during startup'
                if time.monotonic() > deadline:
                    raise RuntimeError('GUI instance lock was not created')
                time.sleep(0.05)
            duplicates = []
            try:
                for _ in range(4):
                    duplicates.append(subprocess.Popen(['ro-control'], env=environment,
                                                       stdout=log, stderr=log))
                for duplicate in duplicates:
                    assert duplicate.wait(timeout=10) == 0, 'Duplicate GUI did not activate owner'
            finally:
                for duplicate in duplicates:
                    stop(duplicate)
            assert primary.poll() is None, 'Primary GUI exited after activation'
            subprocess.run(['ro-control', '--version'], env=environment, check=True,
                           stdout=log, stderr=log, timeout=10)
            try:
                primary.wait(timeout=5)
                raise RuntimeError('Primary GUI exited before the startup check completed')
            except subprocess.TimeoutExpired:
                pass
            primary.kill()
            primary.wait(timeout=5)
            # A killed owner leaves its lock/socket behind; a new GUI must recover.
            primary = subprocess.Popen(['ro-control'], env=environment, stdout=log, stderr=log)
            try:
                primary.wait(timeout=5)
                raise RuntimeError('GUI did not recover after the previous owner was killed')
            except subprocess.TimeoutExpired:
                pass
        finally:
            stop(primary)
    output = log_path.read_text()
    print(output)
    for marker in ('failed to load component', 'is not installed', 'cannot load library'):
        assert marker not in output.lower(), output
print('Installed GUI startup, concurrent relaunch, CLI coexistence and crash recovery passed')
