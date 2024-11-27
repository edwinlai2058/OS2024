#!/bin/bash

testcases=("hw4_partII_a" "hw4_partII_b" "hw4_partIII")

mkdir -p .tmp

for testcase in "${testcases[@]}"; do
    ./"$testcase.sh" > ".tmp/$testcase.txt"
    diff ".tmp/$testcase.txt" "./hw4_ans/${testcase}_ans.txt"
    
    if [ $? -eq 0 ]; then
        echo -e "\e[92m$testcase Succeed.\e[0m"
    else
        echo -e "\e[91m$testcase Failed.\e[0m"
    fi
done

rm -r .tmp
