import shutil
import subprocess
import time
import os
import re

# Change directory to the current directory of the __file__
current_dir = os.getcwd()
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

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

# * Clean up any build artifacts that might interfere
dist_dir = os.path.join(script_dir, 'dist')
build_dir = os.path.join(script_dir, 'build')
if os.path.exists(dist_dir):
    shutil.rmtree(dist_dir, ignore_errors=True)
if os.path.exists(build_dir):
    shutil.rmtree(build_dir, ignore_errors=True)

# * Publish to pypi
# Use --no-use-vcs to skip VCS checks
subprocess.run(['flit', 'publish', '--repository', 'pypi', '--pypirc', os.path.expanduser("~")+'/.pypirc', '--no-use-vcs'], check=True)

# wait for pypi to update
#print("waiting 5 seconds for pypi to update")
#time.sleep(5)

# Update metaffi-api pip package
# subprocess.run(['py', '-m', 'pip', 'install', 'metaffi-api', '--upgrade'], check=True)

# Change back to the previous current directory
os.chdir(current_dir)

print("done updating package")
