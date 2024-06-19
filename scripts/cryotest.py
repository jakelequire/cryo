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

var_test_cases = [
    "./tests/cryo_src/var_tests/*.cryo",
]

print("Running tests...")

def run_tests():
    for pattern in var_test_cases:
        for file in glob.glob(pattern, recursive=True):
            try:
                # Print the files listed
                print(f"Running test {file}")
                # Run the test
                # Output the result to /tests/logs/results.txt
                os.system(f"cryo build -s {file} > ./tests/logs/results.txt")
            except OSError as e:
                print(f"Error running test {file}: {e.strerror}")

def main():
    run_tests()

if __name__ == "__main__":
    main()