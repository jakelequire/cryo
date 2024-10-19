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
from datetime import datetime

object_files = []

def recursive_file_names():
    folder = "./bin/.o"
    file_list = []
    for root, dirs, files in os.walk(folder):
        for file in files:
            file_list.append(os.path.join(root, file))
    
    object_files.append(file_list)
    return file_list

def delete_folder():
    folder = "./bin/.o"
    try:
        # Force delete folder and all contents
        os.system(f"rm -rf {folder}")
        print(f"Deleted {folder}")
    except OSError as e:
        print(f"Error deleting {folder}: {e.strerror}")

# Function to create cleaned.txt log file
def create_cleaned_log(list):
    cleaned_file_path = "./bin/cleaned.txt"
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    compiler_version = os.popen("clang --version").read().strip().split('\n')[0]
    print("Cleaned Files List: ")
    for file in list:
        print(file)

    with open(cleaned_file_path, "w") as f:
        f.write(f"Last cleaned: {current_time}\n")
        f.write(f"Compiler version: {compiler_version}\n")

if __name__ == "__main__":
    object_files = recursive_file_names()
    delete_folder()
    create_cleaned_log(object_files)
    print("Cleaned successfully.")
