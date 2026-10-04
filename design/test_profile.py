# SPDX-License-Identifier: GPL-3.0-or-later
"""Safety and compatibility checks; these do not replace a real GTK run."""
import importlib.util
from pathlib import Path
import re
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("design_launch", ROOT / "design/launch.py")
launch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(launch)


class ProfileTests(unittest.TestCase):
    def test_preserves_custom_preferences(self):
        with tempfile.TemporaryDirectory() as directory:
            profile = Path(directory) / "profile"
            launch.prepare(profile)
            custom = b'# user preference\n(check-updates no)\n'
            (profile / "gimprc").write_bytes(custom)
            launch.prepare(profile)
            self.assertEqual((profile / "gimprc").read_bytes(), custom)

    def test_refuses_existing_personal_profile(self):
        with tempfile.TemporaryDirectory() as directory:
            profile = Path(directory)
            original = profile / "gimprc"
            original.write_text("personal preferences")
            with self.assertRaises(ValueError):
                launch.prepare(profile)
            self.assertEqual(original.read_text(), "personal preferences")
            self.assertEqual(list(profile.iterdir()), [original])

    def test_rejects_redirected_destination(self):
        with tempfile.TemporaryDirectory() as directory:
            profile = Path(directory) / "profile"
            launch.prepare(profile)
            destination = profile / "toolrc"
            destination.unlink()
            destination.mkdir()
            with self.assertRaises(ValueError):
                launch.prepare(profile)

    def test_shortcuts_resolve_to_real_actions_without_duplicates(self):
        text = (ROOT / "etc/shortcutsrc").read_text()
        entries = re.findall(r'^\(action "([^"]+)" "([^"]+)"\)$', text, re.M)
        self.assertGreater(len(entries), 30)
        actions = [action for action, _ in entries]
        accelerators = [accel.lower() for _, accel in entries]
        self.assertEqual(len(actions), len(set(actions)))
        self.assertEqual(len(accelerators), len(set(accelerators)))
        sources = "\n".join(p.read_text(encoding="utf-8")
                            for p in (ROOT / "app/actions").glob("*-actions.c"))
        tools = "\n".join(p.read_text(encoding="utf-8")
                          for p in (ROOT / "app/tools").glob("*.c"))
        for action in actions:
            if action.startswith("tools-") and f'"{action}"' not in sources:
                identifier = 'gimp-' + action[len('tools-'):] + '-tool'
                self.assertIn(f'"{identifier}"', tools)
            else:
                self.assertIn(f'"{action}"', sources)

    def test_tool_profile_has_all_upstream_tools_once(self):
        tools = re.findall(r'\(GimpToolInfo "([^"]+)"',
                           (ROOT / "etc/toolrc").read_text())
        self.assertEqual(len(tools), len(set(tools)))
        # The baseline tool list is retained as a versioned fixture.
        expected = (ROOT / "design/upstream-tools.txt").read_text().splitlines()
        self.assertEqual(set(tools), set(expected))

    def test_config_delimiters(self):
        for path in launch.FILES.values():
            # Ignore strings and comments before checking balanced nested forms.
            text = path.read_text()
            text = re.sub(r'"(?:\\.|[^"\\])*"|#[^\n]*', '', text)
            depth = 0
            for character in text:
                if character == '(':
                    depth += 1
                elif character == ')':
                    depth -= 1
                    self.assertGreaterEqual(depth, 0, str(path))
            self.assertEqual(depth, 0, str(path))

    def test_preferences_use_current_config_properties(self):
        schema = "\n".join(p.read_text(encoding="utf-8")
                           for p in (ROOT / "app/config").glob("*.c"))
        preferences = (ROOT / "design/profile/gimprc").read_text()
        for name in re.findall(r'^\(([a-z-]+)', preferences, re.M):
            self.assertIn(f'"{name}"', schema)


if __name__ == "__main__":
    unittest.main()
