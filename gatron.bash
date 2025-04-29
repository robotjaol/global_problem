#!/bin/bash

LIMIT_MB=1
CMD_FILE="/tmp/taskq.txt"

monitor_memory() {
    MEM=$(ps -o rss= -p $$)
    MEM_MB=$((MEM / 1024))
    if (( MEM_MB > LIMIT_MB )); then
        echo "[KILL] Exceeded RAM: $MEM_MB MB"
        kill -9 $$
    fi
}

generate_queue() {
    echo "echo 'secret-1',"$(crc32 <<< "echo 'secret-1'") > $CMD_FILE
    echo "date,"$(crc32 <<< "date") >> $CMD_FILE
}

crc32() {
    cksum | awk '{print $1}'
}

main() {
    generate_queue
    ./executor "$CMD_FILE" &
    while kill -0 $! 2>/dev/null; do
        monitor_memory
        sleep 0.1
    done
}

main
