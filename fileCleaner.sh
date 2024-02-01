#!/bin/bash

if [ -d ./bibData_old ]; then
    if [ -d ./bibData ]; then
        rm -r ./bibData
    fi
    mv ./bibData_old ./bibData
fi

cp -r bibData/ bibData_old/

for file in ./bibData/*.txt; do
    cat $file 
    stringa_trim=$(cat "$file" | sed -r 's/[\ ]+/ /g' | sed -r 's/\ ,/,/g' | sed -r 's/\ ;/;/g' | sed -r 's/\ :/:/g')
    echo "$stringa_trim" > "$file" 
done