import shutil
import subprocess
import time
import os
from typing import final
import re

# Change directory to the current directory of the __file__
current_dir = os.getcwd()
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

git_changed = False
tests_moved = False
unittest_moved = False

try:
	tests_src = os.path.join(script_dir, 'tests')
	tests_dst = os.path.join(script_dir, '..', 'tests')
	if os.path.exists(tests_src) and len(os.listdir(tests_src)) > 0 and (not os.path.exists(tests_dst) or len(os.listdir(tests_dst)) == 0):
		shutil.rmtree(tests_dst, ignore_errors=True)
		shutil.move(tests_src, tests_dst)
		tests_moved = True

	unittest_src = os.path.join(script_dir, 'unittest')
	unittest_dst = os.path.join(script_dir, '..', 'unittest')
	if os.path.exists(unittest_src) and len(os.listdir(unittest_src)) > 0 and (not os.path.exists(unittest_dst) or len(os.listdir(unittest_dst)) == 0):
		shutil.rmtree(unittest_dst, ignore_errors=True)
		shutil.move(unittest_src, unittest_dst)
		unittest_moved = True

	# * ---- Update the package version ----
	# Read the current version from the file
	with open("./metaffi/__init__.py", "r") as file:
		content = file.read()

	# Extract the current version number using regex
	pattern = r"__version__ = \"(\d+\.\d+\.)(\d+)\""
	match = re.search(pattern, content)
	if match:
		major_minor = match.group(1)
		patch = int(match.group(2))
		new_patch = patch + 1
		new_version = f"{major_minor}{new_patch}"
		# Replace the old version with the new version using regex
		content = re.sub(pattern, f"__version__ = \"{new_version}\"", content)

	# Write the modified content back to the file
	with open("./metaffi/__init__.py", "w") as file:
		file.write(content)
	
	# * Git commit the code for publishing to pypi
	subprocess.run(['git', 'add', '*'], check=True)
	subprocess.run(['git', 'commit', '-m', '.'], check=True)

	git_changed = True

	# * Publish to pypi
	subprocess.run(['flit', 'publish', '--repository', 'pypi', '--pypirc', os.path.expanduser("~")+'/.pyirc'], check=True)

	# wait for pypi to update
	print("waiting 5 seconds for pypi to update")
	time.sleep(5)

	# Update metaffi-api pip package
	subprocess.run(['py', '-m', 'pip', 'install', 'metaffi-api', '--upgrade'], check=True)

	# Change back to the previous current directory
	os.chdir(current_dir)

	print("done updating package")

finally:
	if tests_moved:
		if os.path.exists(tests_src) and len(os.listdir(tests_src)) == 0:
			shutil.rmtree(tests_src)
		shutil.move(tests_dst, tests_src)

	if unittest_moved:
		if os.path.exists(unittest_src) and len(os.listdir(unittest_src)) == 0:
			shutil.rmtree(unittest_src)
		shutil.move(unittest_dst, unittest_src)

	if git_changed:
		subprocess.run(['git', 'add', '*'], check=True)
		subprocess.run(['git', 'commit', '-m', '.'], check=True)

	# Change back to the previous current directory
	os.chdir(current_dir)
