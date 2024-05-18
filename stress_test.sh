#!/bin/bash

# 目标 URL
URL="http://localhost:8099/"

test_data=(  
    "500 10" 
    "1000 20" 
    "5000 10"
    "10000 5"
    "10000 60"
)  

test_file_dir="test_result_files"

mkdir -p ${test_file_dir}

# 进行压力测试
for test in "${test_data[@]}"; do
    # 获取对应的测试时间
    concurrency=${test%% *}
    duration=${test#* }
    file="${test_file_dir}/result_${concurrency}_${duration}.txt"  

    # 使用 Webbench 进行压力测试
    webbench -c "$concurrency" -t "$duration" $URL > "${file}"

    sleep "$duration"

    echo "=================================================="
    echo "Starting test with $concurrency concurrent connections for $duration seconds"
    echo "Concurrency is ${concurrency}, duration is ${duration}" 
    echo "--------------------------------------------------" 
    tail -n 2 "${file}"
    echo "Test with [${concurrency} ${duration}s] completed"
    echo "=================================================="
done

echo "All tests completed."