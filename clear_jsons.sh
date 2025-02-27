#!/bin/bash

PATTERN="*.json"
EXCEPT_FILE="serverconfig.json"

for file in $PATTERN; do
    if [[ "$file" != "$EXCEPT_FILE" ]]; then
        echo "Deleting: $file"
        rm "$file"
    fi
done