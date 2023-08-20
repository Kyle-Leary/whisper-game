#!/bin/bash
# this script is really dumb and only grabs functions with the exact sig void function_name();

target="testmain.c"

echo "// generated file, do not edit" > ${target}

# generate the header function defs.
for file in src/*.test.c; do
    grep -oP '(?<=^void )\w+' "$file" | sed -e 's/^/void /' -e 's/$/();/' >> ${target}
done

echo "int main() {" >> ${target}

# Iterate through each .test.c file
# make the testmain.c file for testing from all the .test.c files in this directory.
for file in src/*.test.c; do
    # Use grep and sed to extract function names
    grep -oP 'void \w+\(\)' "$file" | sed -e 's/void //' -e 's/$/;/' >> ${target}
done

echo "}" >> ${target}
