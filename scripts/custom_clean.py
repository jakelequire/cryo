#*******************************************************************************
#  Copyright 2024 Jacob LeQuire                                                *
#  SPDX-License-Identifier: Apache-2.0                                         *
#    Licensed under the Apache License, Version 2.0 (the "License");           *
#    you may not use this file except in compliance with the License.          *
#    You may obtain a copy of the License at                                   *
#                                                                              *
#    http://www.apache.org/licenses/LICENSE-2.0                                *
#                                                                              *
#    Unless required by applicable law or agreed to in writing, software       *
#    distributed under the License is distributed on an "AS IS" BASIS,         *
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
#    See the License for the specific language governing permissions and       *
#    limitations under the License.                                            *
#                                                                              *
#*******************************************************************************/
# clean.py
import os
import sys
from datetime import datetime

# /home/phock/Programming/apps/cryo/scripts
current_script_dir = os.path.dirname(os.path.abspath(__file__))
# We want: /home/phock/Programming/apps/cryo
root_dir = os.path.abspath(os.path.join(current_script_dir, os.pardir))
# The parent folder to all options is: /home/phock/Programming/apps/cryo/bin/.o/{option}
parent_option_dir = os.path.join(root_dir, "bin/.o")

options = {
    "codegen": "codegen",
    "common": "common",
    "compiler": "compiler",
    "diagnostics": "diagnostics",
    "frontend": "frontend",
    "linker": "linker",
    "runtime": "runtime",
    "settings": "settings",
    "symbolTable": "symbolTable",
    "tools": "tools",
    "main": "main"
}

def print_usage():
    print("Usage: python clean.py <option>")
    print("Available options:")
    for option in options.keys():
        print(f"  - {option}")

# Function to clean the specified directory
def clean_option(option):
    if option not in options:
        print(f"Error: Invalid option '{option}'")
        print_usage()
        return False
    
    option_dir = os.path.join(parent_option_dir, options[option])
    try:
        # Force delete folder and all contents
        os.system(f"rm -rf {option_dir}")
        print(f"Deleted {option_dir}")
        return True
    except OSError as e:
        print(f"Error deleting {option_dir}: {e.strerror}")
        return False

# Function to create cleaned.txt log file
def create_cleaned_log(cleaned_option):
    cleaned_file_path = os.path.join(root_dir, "bin/cleaned.txt")
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    compiler_version = os.popen("clang --version").read().strip().split('\n')[0]
    
    print(f"Cleaned File: {cleaned_option}")

    with open(cleaned_file_path, "w") as f:
        f.write(f"Last cleaned: {current_time}\n")
        f.write(f"Compiler version: {compiler_version}\n")
        f.write(f"Cleaned directory: {cleaned_option}\n")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Error: Exactly one argument is required")
        print_usage()
        sys.exit(1)

    option = sys.argv[1]
    if clean_option(option):
        create_cleaned_log(option)
        print("Cleaned successfully.")
    else:
        sys.exit(1)
