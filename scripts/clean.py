# clean.py
import os
import glob

files_to_delete = [
    "./src/compiler/*.o",
    "./src/utils/*.o",
    "./cli/*.o",
    "./src/main.o",
    "./src/bin/main.exe",
]

for pattern in files_to_delete:
    for file in glob.glob(pattern, recursive=True):
        try:
            os.remove(file)
            print(f"Deleted {file}")
        except OSError as e:
            print(f"Error deleting {file}: {e.strerror}")
