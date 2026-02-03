#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "Core.h"

class Simulator {
public:
    Simulator();
    void run();
    void cycle();
    void loadInstructions();

private:
    uint32_t current_cycle = 0;
    bool is_running = true;
    std::vector<std::unique_ptr<Core>> cores;
};