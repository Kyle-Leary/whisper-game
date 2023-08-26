#!/bin/bash
# this script is really dumb and only grabs functions with the exact sig void function_name();

target="testmain.c"

echo "// generated file, do not edit" > ${target}

# generate the header function defs.
for file in $(find src -name '*.test.c'); do
    grep -oP '(?<=^void )\w+' "$file" | sed -e 's/^/void /' -e 's/$/();/' >> ${target}
done

echo "int main() {" >> ${target}

# Iterate through each .test.c file
# make the testmain.c file for testing from all the .test.c files in this directory.
for file in $(find src -name '*.test.c'); do
    # Use grep and sed to extract function names
    fn_call=$(grep -oP 'void \w+\(\)' "$file" | sed -e 's/void //' -e 's/$/;/')
    echo "printf(\"Running: '${fn_call}'\\n\");" >> ${target}
    echo ${fn_call} >> ${target}
    echo "printf(\"Successfully ran: '${fn_call}'\\n\");" >> ${target}
done

echo "}" >> ${target}
