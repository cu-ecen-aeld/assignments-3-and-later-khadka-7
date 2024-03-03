#!/bin/bash
#"sujan khadka"

# Check if required arguments are provided
if [ $# -ne 2 ]; then
    echo "Error: Please provide both the file path and the text string."
    exit 1
fi

writefile="$1"
writestr="$2"

# Create the directory path if it doesn't exist
mkdir -p "$(dirname "$writefile")"

# Write the text string to the file
echo "$writestr" > "$writefile"

# Check if the file was created successfully
if [ $? -ne 0 ]; then
    echo "Error: Failed to create the file."
    exit 1
fi

echo "File '$writefile' successfully created with content:"
echo "$writestr"

