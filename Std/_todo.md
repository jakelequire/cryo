# Cryo Standard Library

This is the standard library for Cryo. It contains a collection of modules that are available to all Cryo programs. The standard library is designed to be as minimal as possible, with only the most essential modules included. This is to keep the size of the runtime small and to make it easier to understand and maintain.

## Modules

The compiler is still in development, so the standard library is also a work in progress. These are the following modules that need to be implemented:

- [cBridge](#cBridge) - Interact with C libraries and functions. This is the interface between Cryo and C. This is mostly an internal module, but it is exposed to the user for advanced use cases.

- [FS](#FS) - File system operations. This module allows you to read and write files, create directories, and more.

- [IO](#IO) - Input and output operations. This module allows you to read from and write to the console, files, and other streams.

- [Math](#Math) - Mathematical operations. This module provides functions for basic arithmetic, trigonometry, and other mathematical operations.

- [OS](#OS) - Operating system operations. This module allows you to interact with the operating system, such as running commands, getting environment variables, and more.

- [String](#String) - String operations. This module provides functions for manipulating strings, such as concatenation, substring, and more.

*Note: There will be more modules added in the future. This is just the initial set of modules that are planned and under development.*

---

## cBridge

This module specifically gets compiled together with the C code. This allows C to call Cryo functions and vice versa. This is mostly an internal module, but it is exposed to the user for advanced use cases.

### Functions

// TODO: Implement

---

## FS

This module provides file system operations. This module allows you to read and write files, create directories, and more.

### Functions

- `readFile(path: string): string` - Read the contents of a file at the given path and return it as a string.

- `writeFile(path: string, contents: string): void` - Write the given contents to a file at the given path.

- `createDir(path: string): void` - Create a directory at the given path.

- `removeDir(path: string): void` - Remove the directory at the given path.

- `removeFile(path: string): void` - Remove the file at the given path.

- `listDir(path: string): string[]` - List the contents of the directory at the given path and return them as an array of strings.

- `fileExists(path: string): boolean` - Check if a file exists at the given path.

- `dirExists(path: string): boolean` - Check if a directory exists at the given path.

---

## IO

This module provides input and output operations. This module allows you to read from and write to the console, files, and other streams.

### Functions

- `print(value: any): void` - Print the given value to the console. (TODO: Implement)

- `println(value: any): void` - Print the given value to the console followed by a newline. (TODO: Implement)

- `readLine(): string` - Read a line of input from the console and return it as a string. (TODO: Implement)

---

## Math

This module provides mathematical operations. This module provides functions for basic arithmetic, trigonometry, and other mathematical operations.

### Functions

- `abs(n: number): number` - Return the absolute value of the given number.

- `pow(base: number, exponent: number): number` - Return the result of raising the base to the exponent.

- `sqrt(n: number): number` - Return the square root of the given number.

- `max(a: number, b: number): number` - Return the larger of the two given numbers.

- `min(a: number, b: number): number` - Return the smaller of the two given numbers.

- `round(n: number): number` - Return the nearest integer to the given number.

- `floor(n: number): number` - Return the largest integer less than or equal to the given number.

- `ceil(n: number): number` - Return the smallest integer greater than or equal to the given number.

- `random(): number` - Return a random number between 0 and 1.

---

## OS

This module provides operating system operations. This module allows you to interact with the operating system, such as running commands, getting environment variables, and more.

### Functions

- `runCommand(command: string): string` - Run the given command in the shell and return the output as a string.

- `getEnv(name: string): string` - Get the value of the environment variable with the given name.

- `setEnv(name: string, value: string): void` - Set the value of the environment variable with the given name.

- `exit(code: number): void` - Exit the program with the given exit code.

- `sleep(milliseconds: number): void` - Sleep for the given number of milliseconds.

- `time(): number` - Return the current time in milliseconds since the Unix epoch.

---

## String

This module provides string operations. This module provides functions for manipulating strings, such as concatenation, substring, and more.

### Functions

- `concat(a: string, b: string): string` - Concatenate the two given strings.

- `substring(s: string, start: number, end: number): string` - Return the substring of the given string starting at the start index and ending at the end index.

- `length(s: string): number` - Return the length of the given string.

- `charAt(s: string, index: number): string` - Return the character at the given index in the string.

- `indexOf(s: string, sub: string): number` - Return the index of the first occurrence of the substring in the string, or -1 if the substring is not found.

- `contains(s: string, sub: string): boolean` - Return true if the string contains the substring, false otherwise.

- `startsWith(s: string, sub: string): boolean` - Return true if the string starts with the substring, false otherwise.

- `endsWith(s: string, sub: string): boolean` - Return true if the string ends with the substring, false otherwise.

- `toLowerCase(s: string): string` - Return the string converted to lowercase.

- `toUpperCase(s: string): string` - Return the string converted to uppercase.

---
