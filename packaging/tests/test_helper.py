"""Exercise the actual helper's command boundary without privileges or DNF writes."""
import os
import pathlib
import subprocess
import tempfile
import unittest

HELPER = pathlib.Path(__file__).parents[2] / 'data/helpers/ro-control-helper.sh.in'


class HelperCommandTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        dnf = pathlib.Path(self.temp.name) / 'dnf'
        dnf.write_text('#!/bin/sh\nprintf "locale=%s\\n" "$LANG"\nprintf "arg=%s\\n" "$@"\nexit "${TEST_DNF_EXIT:-0}"\n')
        dnf.chmod(0o755)
        self.env = dict(os.environ, PATH=self.temp.name + ':' + os.defpath, LANG='tr_TR.UTF-8')

    def run_helper(self, *args, stdin=None):
        return subprocess.run(['sh', str(HELPER), *args], input=stdin,
                              capture_output=True, text=True, env=self.env)

    def test_exact_installer_batch_reaches_dnf_with_c_locale(self):
        result = self.run_helper('--batch', stdin='env\tLANG=C\tdnf\tinstall\t-y\tpackage with spaces\n')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('locale=C\n', result.stdout)
        self.assertIn('arg=package with spaces\n', result.stdout)

    def test_direct_invocation_and_exit_status_are_preserved(self):
        self.env['TEST_DNF_EXIT'] = '42'
        result = self.run_helper('env', 'LANG=C', 'dnf', '--version')
        self.assertEqual(result.returncode, 42)
        self.assertIn('locale=C\n', result.stdout)

    def test_arbitrary_env_commands_and_path_overrides_are_rejected(self):
        for args in [('env',), ('env', 'LANG=C'),
                     ('env', 'LANG=C', 'sh', '-c', 'true'),
                     ('env', 'PATH=/tmp', 'dnf'),
                     ('env', 'LANG=tr_TR', 'dnf'),
                     ('env', 'LANG=C', 'PATH=/tmp', 'dnf'),
                     ('sh', '-c', 'true')]:
            with self.subTest(args=args):
                result = self.run_helper(*args)
                self.assertEqual(result.returncode, 64)
                self.assertEqual(result.stdout, '')

    def test_failed_batch_does_not_run_following_command(self):
        result = self.run_helper('--batch', stdin='env\tLANG=C\tsh\n' + 'dnf\t--version\n')
        self.assertEqual(result.returncode, 64)
        self.assertEqual(result.stdout, '')
