#!/usr/bin/env python3
"""Read-only booted-host probes. Does not install drivers or qualify a release."""
import argparse
import datetime
import json
import pathlib
import platform
import subprocess


def probe(argv):
    try:
        result = subprocess.run(argv, capture_output=True, text=True, timeout=45)
        return dict(command=argv, exit_code=result.returncode,
                    stdout=result.stdout.strip(), stderr=result.stderr.strip())
    except (OSError, subprocess.TimeoutExpired) as error:
        return dict(command=argv, exit_code=None, error=str(error))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=pathlib.Path)
    args = parser.parse_args()
    os_release = pathlib.Path('/etc/os-release')
    os_data = os_release.read_text() if os_release.exists() else ''
    pid_one = pathlib.Path('/proc/1/comm')
    booted = pid_one.exists() and pid_one.read_text().strip() == 'systemd'
    probes = {
        'rpm': ['rpm', '-q', 'ro-control'],
        'files': ['rpm', '-V', 'ro-control'],
        'diagnostics': ['ro-control', 'diagnostics', '--json'],
        'pci': ['lspci', '-nnk'],
        'secure_boot': ['mokutil', '--sb-state'],
        'gpu': ['nvidia-smi', '--query-gpu=name,driver_version', '--format=csv,noheader'],
        'module_vermagic': ['modinfo', '-F', 'vermagic', 'nvidia'],
        'module_signer': ['modinfo', '-F', 'signer', 'nvidia'],
        'user_service': ['systemctl', '--user', 'is-active', 'ro-control.service'],
        'system_service': ['systemctl', 'is-active', 'ro-control.service'],
        'session': ['loginctl', 'show-session', 'self', '-p', 'Type', '-p', 'Active'],
    }
    results = {name: probe(argv) for name, argv in probes.items()}
    evidence = {
        'kind': 'fedora45-booted-host-probes',
        'collected_at': datetime.datetime.now(datetime.timezone.utc).isoformat(),
        'os_release': os_data, 'kernel': platform.release(),
        'architecture': platform.machine(), 'systemd_is_pid_one': booted,
        'probes': results,
        'release_qualified': False,
        'manual_evidence_required': [
            'actual Fedora 44 to 45 OS upgrade and graphical login',
            'selected NVIDIA installer path with complete result log',
            'Polkit authorization and cancellation',
            'Secure Boot state and enrollment/signature verification when enabled',
            'post-reboot module loading and GPU operation',
        ],
    }
    with args.output.open('x') as stream:
        json.dump(evidence, stream, indent=2)
        stream.write('\n')
    print(args.output)
    # Fail clearly if this is not a Fedora 45 booted host with a working GPU.
    distro = dict(line.split('=', 1) for line in os_data.splitlines() if '=' in line)
    valid_host = distro.get('VERSION_ID', '').strip('"') == '45' and booted
    raise SystemExit(0 if valid_host and results['gpu']['exit_code'] == 0 else 1)


if __name__ == '__main__':
    main()
