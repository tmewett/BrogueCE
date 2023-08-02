#!/bin/bash

# Function to recursively find source files and run clang-format
function format_source_files() {
    local root_dir="$1"
    local exclude_list=("$@")

    find "$root_dir" -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" \) | while read -r file; do
        # Check if the file should be excluded
        if [[ ! " ${exclude_list[@]} " =~ " ${file} " ]]; then
            echo "Formatting: $file"
            clang-format -i "$file"
            # sed command to ensure files end with a newline
            sed -i -e '$!b' -e '/^$/!a\' "$file"
        else
            echo "Excluding: $file"
        fi
    done
}

# Check if the correct number of arguments is provided
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <directory_root> <exclude_file1> <exclude_file2> ... <exclude_fileN>"
    exit 1
fi

directory_root="$1"
shift # Remove the first argument (directory_root) from the arguments list

# Call the format_source_files function with the provided arguments
format_source_files "$directory_root" "$@"
