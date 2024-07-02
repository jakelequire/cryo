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
import os
import glob
from datetime import datetime

cryo_lib_files = [
    "./src/.o/cryolib.o",
]


source_file = "./output.ll"
bc_file = "./output.bc"
o_file = "./output.o"


# 
# llvm-as ./output.ll -o ./output.bc
# llc -filetype=obj ./output.bc -o ./output.o
#


def cryo_init():
    # Delete old files
    for file in cryo_lib_files:
        try:
            os.remove(file)
            print(f"Deleted {file}")
        except OSError as e:
            print(f"Error deleting {file}: {e.strerror}")

    # Compile cryolib
    os.system("llc -filetype=obj ./src/cryolib.ll -o ./src/.o/cryolib.o")
    print("Compiled cryolib")

    # Compile source file
    os.system(f"llvm-as {source_file} -o {bc_file}")
    print("Compiled source file")

    # Compile source file to object file
    os.system(f"llc -filetype=obj {bc_file} -o {o_file}")
    print("Compiled source file to object file")

    # Link object files
    os.system(f"clang -o ./output {o_file} ./src/.o/cryolib.o")
    print("Linked object files")

    # Run linked file
    os.system("./output")
    print("Ran linked file")

if __name__ == "__main__":
    cryo_init()
    print("Cryo initialized")