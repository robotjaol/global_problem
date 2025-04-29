#include <iostream>
#include <fstream>
#include <array>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MAX_CMDS 10000
#define MAX_LEN 256
#define MAX_PROCS 4

// Static RAM-only command buffer
std::array<std::array<char, MAX_LEN>, MAX_CMDS> cmds;
std::array<int, MAX_PROCS> active_pids = {0};

uint32_t crc32(const char* data, size_t len) {
    uint32_t crc = ~0u;
    while (len--)
        crc ^= (unsigned char)*data++,
        crc = (crc >> 8) ^ (0xEDB88320 & -(crc & 1));
    return ~crc;
}

int load_commands(const char* path) {
    std::ifstream file(path);
    if (!file) return -1;
    int count = 0;
    std::string line;
    while (std::getline(file, line) && count < MAX_CMDS) {
        auto sep = line.find_last_of(',');
        if (sep == std::string::npos) continue;
        std::string cmd = line.substr(0, sep);
        uint32_t checksum = std::stoul(line.substr(sep + 1));
        if (crc32(cmd.c_str(), cmd.size()) != checksum) continue;
        strncpy(cmds[count].data(), cmd.c_str(), MAX_LEN - 1);
        count++;
    }
    return count;
}

void reap_children() {
    for (int i = 0; i < MAX_PROCS; ++i) {
        if (active_pids[i] > 0) {
            if (waitpid(active_pids[i], nullptr, WNOHANG) > 0)
                active_pids[i] = 0;
        }
    }
}

void run_commands(int count) {
    for (int i = 0; i < count; ++i) {
        reap_children();
        bool slot_found = false;
        for (int j = 0; j < MAX_PROCS; ++j) {
            if (active_pids[j] == 0) {
                pid_t pid = fork();
                if (pid == 0) {
                    execl("/bin/sh", "sh", "-c", cmds[i].data(), nullptr);
                    _exit(1); // exec failed
                } else {
                    active_pids[j] = pid;
                    slot_found = true;
                    break;
                }
            }
        }
        if (!slot_found) {
            --i; // Retry 
            usleep(1000);
        }
    }
    for (int i = 0; i < MAX_PROCS; ++i) {
        if (active_pids[i] > 0)
            waitpid(active_pids[i], nullptr, 0);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./exec <file>\n";
        return 1;
    }
    int count = load_commands(argv[1]);
    if (count < 0) {
        std::cerr << "Failed to load.\n";
        return 1;
    }
    run_commands(count);
    return 0;
}
