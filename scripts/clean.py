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
import glob
from datetime import datetime

files_to_delete = [
    "./src/*.o",
    "./src/tests/*.o",
    "./src/compiler/*.o",
    "./src/utils/*.o",
    "./src/cli/*.o",
    "./src/cli/commands/*.o",
    
    "./src/bin/*.exe",
    "./src/bin/*.ilk",
    "./src/bin/*.pdb",
    "./src/bin/*.o",
    "./src/bin/.o/*.exe",
    "./src/bin/.o/*.ilk",
    "./src/bin/.o/*.pdb",
    "./src/bin/.o/*.o",
    "./src/bin/debug/*.exe",
    "./src/bin/debug/*.ilk",
    "./src/bin/debug/*.pdb",
    "./src/bin/debug/*.o",
    "./src/bin/cleaned.txt",
]

# Function to delete files
def delete_files():
    for pattern in files_to_delete:
        for file in glob.glob(pattern, recursive=True):
            try:
                os.remove(file)
                print(f"Deleted {file}")
            except OSError as e:
                print(f"Error deleting {file}: {e.strerror}")

# Function to create cleaned.txt log file
def create_cleaned_log():
    cleaned_file_path = "./src/bin/cleaned.txt"
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    compiler_version = os.popen("clang --version").read().strip().split('\n')[0]

    with open(cleaned_file_path, "w") as f:
        f.write(f"Last cleaned: {current_time}\n")
        f.write(f"Compiler version: {compiler_version}\n")

if __name__ == "__main__":
    delete_files()
    create_cleaned_log()
    print("Cleaned successfully.")