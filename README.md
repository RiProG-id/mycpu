# mycpu v1.0 by RiProG-id

## Description
`mycpu` is a command-line utility written in C for managing CPU frequencies on Linux systems. It provides options to check and adjust CPU frequency settings easily.

## Usage
### Available Options
- `--check`: Displays the current minimum and maximum frequencies for each CPU.
- `--default`: Sets CPU frequencies to their default minimum and maximum values.
- `--forcemin`: Forces both minimum and maximum CPU frequencies to the lowest available frequency.
- `--forcemax`: Forces both minimum and maximum CPU frequencies to the highest available frequency.
- `--help`: Shows the help message with usage instructions.

## Example Usage
```bash
git clone https://github.com/RiProG-id/mycpu.git
cd mycpu
clang -O3 mycpu.c -o mycpu
chmod +x mycpu
./mycpu
```
