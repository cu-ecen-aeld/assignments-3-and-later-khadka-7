#!/bin/bash

# Check if required arguments are provided
if [ $# -ne 2 ]; then
    echo "Error: Please provide both the directory path and the search string."
    exit 1
fi

filesdir="$1"
searchstr="$2"

# Check if filesdir exists and is a directory
if [ ! -d "$filesdir" ]; then
    echo "Error: '$filesdir' is not a valid directory."
    exit 1
fi

# Function to recursively search for matching lines in files
search_files() {
    local files=("$1"/*)
    local count_files=0
    local count_matches=0

    for file in "${files[@]}"; do
        if [ -f "$file" ]; then
            ((count_files++))
            matches=$(grep -c "$searchstr" "$file")
            ((count_matches += matches))
            echo "File: $file - Matches: $matches"
        elif [ -d "$file" ]; then
            search_files "$file"
        fi
    done

    echo "Total files found: $count_files"
    echo "Total matching lines: $count_matches"
}

# Start searching files
search_files "$filesdir"

