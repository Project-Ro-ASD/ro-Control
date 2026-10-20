import copy
import importlib.util
import pathlib
import unittest

path = pathlib.Path(__file__).parents[1] / "scripts/build-ro-repo-manifest.py"
spec = importlib.util.spec_from_file_location("manifest", path)
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)


class ReleaseIdentityTests(unittest.TestCase):
    def setUp(self):
        self.artifacts = [
            dict(filename=f"ro-control-1.3.1-1.fc44.{arch}.rpm", architecture=arch,
                 version="1.3.1", epoch=0, release="1.fc44",
                 source_rpm=None if arch == "src" else "ro-control-1.3.1-1.fc44.src.rpm")
            for arch in ("x86_64", "aarch64", "src")
        ]
        self.identity = ["Project-Ro-ASD/ro-Control", "a" * 40, "v1.3.1", "123", "456"]

    def test_matching_identity(self):
        manifest.validate_identity(self.artifacts, *self.identity)

    def test_rejects_mixed_evr_and_wrong_source(self):
        for field, value in (("version", "1.3.0"), ("epoch", 1),
                             ("release", "1"), ("filename", "renamed.rpm"),
                             ("source_rpm", "other.src.rpm")):
            with self.subTest(field=field), self.assertRaises(SystemExit):
                artifacts = copy.deepcopy(self.artifacts)
                artifacts[1][field] = value
                manifest.validate_identity(artifacts, *self.identity)

    def test_rejects_invalid_producer_identity(self):
        for index, value in ((0, "other/repo"), (1, "main"), (2, "v1.3.1;bad"),
                             (3, "0"), (4, "-1")):
            with self.subTest(index=index), self.assertRaises(SystemExit):
                identity = self.identity.copy()
                identity[index] = value
                manifest.validate_identity(self.artifacts, *identity)


if __name__ == "__main__":
    unittest.main()
