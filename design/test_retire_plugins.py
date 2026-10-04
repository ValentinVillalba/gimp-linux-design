# SPDX-License-Identifier: GPL-3.0-or-later
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('retire', Path(__file__).with_name('retire-online-plugins.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class RetirementTests(unittest.TestCase):
    def test_preserves_other_plugins_and_is_repeatable(self):
        with tempfile.TemporaryDirectory() as folder:
            prefix = Path(folder)
            (prefix / 'bin').mkdir()
            (prefix / 'bin/gimp-3.2').touch()
            plugins = prefix / 'lib/x86_64-linux-gnu/gimp/3.0/plug-ins'
            for name in ('mail', 'web-browser', 'file-psd'):
                (plugins / name).mkdir(parents=True)
                (plugins / name / name).write_bytes(b'original binary')
            moves = module.retire(prefix)
            self.assertEqual(len(moves), 2)
            for source, destination in moves:
                self.assertFalse(source.exists())
                self.assertEqual((destination / source.name).read_bytes(), b'original binary')
            self.assertTrue((plugins / 'file-psd/file-psd').exists())
            self.assertEqual(module.retire(prefix), [])

    def test_refuses_unrelated_directory(self):
        with tempfile.TemporaryDirectory() as folder:
            with self.assertRaises(ValueError):
                module.retire(folder)

    @unittest.skipUnless(__import__('os').name == 'posix', 'Symlink permissions differ on Windows')
    def test_rejects_external_path_before_any_move(self):
        with tempfile.TemporaryDirectory() as folder, tempfile.TemporaryDirectory() as outside:
            prefix = Path(folder)
            (prefix / 'bin').mkdir()
            (prefix / 'bin/gimp-3.2').touch()
            plugins = prefix / 'lib/gimp/3.0/plug-ins'
            (plugins / 'mail').mkdir(parents=True)
            (plugins / 'web-browser').symlink_to(outside, target_is_directory=True)
            with self.assertRaises(ValueError):
                module.retire(prefix)
            self.assertTrue((plugins / 'mail').is_dir())
            self.assertFalse((prefix / '_retired-plugins').exists())


if __name__ == '__main__':
    unittest.main()
