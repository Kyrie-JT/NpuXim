#include "Simulator.h"

#include <iostream>
constexpr uint64_t MAX_CYCLE = 100000000;
Simulator::Simulator() : current_cycle(0), is_running(true) {
    // 初始化一个 Core，ID 为 0
    cores.push_back(std::make_unique<Core>(0, 256));
}

void Simulator::run() {
    // 先装载指令
    loadInstructions();

    while (is_running) {
        cycle();
        bool all_cores_idle = true;
        for (auto& core : cores) {
            if (core->IsRunning()) {
                all_cores_idle = false;
                break;
            }
        }
        if (all_cores_idle) {
            is_running = false;

            std::cout << "所有指令已经计算完毕！" << std::endl;
            std::cout << "总共消耗" << current_cycle << "个时钟周期" << std::endl;
        }
        if (current_cycle > MAX_CYCLE) {
            std::cout << "达到最大周期数，退出仿真。" << std::endl;
            break;
        }
    }
}

void Simulator::cycle() {
    current_cycle++;
    // std::cout << "Cycle: " << current_cycle << std::endl;

    // 驱动所有 Core 运行一个周期
    for (auto& core : cores) {
        core->Cycle();
    }
}

// void Simulator::loadInstructions() {
//     // 假设我们只操作第一个核
//     if (cores.empty()) return;
//     auto& target_core = cores[0];

//     // 参数设定
//     const int NUM_TILES = 4;
//     const int INSTR_PER_TILE = 128;
//     const uint32_t MATRIX_SIZE = 256;  // 256*256

//     for (int t = 0; t < NUM_TILES; ++t) {
//         auto tile = std::make_unique<Tile>();
//         for (int i = 0; i < INSTR_PER_TILE; ++i) {
//             auto instr = std::make_unique<Instruction>();
//             instr->opcode = Opcode::GEMM;
//             instr->tile_m = MATRIX_SIZE;
//             instr->tile_n = MATRIX_SIZE;
//             instr->tile_k = MATRIX_SIZE;

//             // 将指令加入 Tile
//             tile->instructions.push_back(std::move(instr));
//         }
//         // 将 Tile 加入 Core
//         target_core->PushTile(std::move(tile));
//     }
//     std::cout << "Loaded " << NUM_TILES << " tiles with " << INSTR_PER_TILE << " instructions each into Core 0."
//               << std::endl;
// }

void Simulator::loadInstructions() {
    // 假设我们只操作第一个核
    if (cores.empty()) return;
    auto& target_core = cores[0];

    // 参数设定
    const int NUM_TILES = 4;
    const int INSTR_PER_TILE = 128;
    const uint32_t MATRIX_SIZE = 256;  // 256*256

    // 计算大小
    const uint64_t MATRIX_BYTES = MATRIX_SIZE * MATRIX_SIZE * 4;

    // 起始地址
    addr_type global_addr_ptr = 0x100000;

    for (int t = 0; t < NUM_TILES; ++t) {
        auto tile = std::make_unique<Tile>();
        for (int i = 0; i < INSTR_PER_TILE; ++i) {
            auto instr = std::make_unique<Instruction>();
            instr->opcode = Opcode::GEMM;
            instr->tile_m = MATRIX_SIZE;
            instr->tile_n = MATRIX_SIZE;
            instr->tile_k = MATRIX_SIZE;

            // 设置大小
            instr->src_size = MATRIX_BYTES;
            instr->dest_size = MATRIX_BYTES;

            // --- 简单线性分配模式 ---
            // 每条指令都用自己独立的空间，互不干扰
            // Instr i: Src=[P, P+S], Dest=[P+S, P+2S]
            instr->src_addr = global_addr_ptr;
            global_addr_ptr += MATRIX_BYTES;

            instr->dest_addr = global_addr_ptr;
            global_addr_ptr += MATRIX_BYTES;

            // 将指令加入 Tile
            tile->instructions.push_back(std::move(instr));
        }
        // 将 Tile 加入 Core
        target_core->PushTile(std::move(tile));
    }
    std::cout << "Loaded " << NUM_TILES << " tiles with " << INSTR_PER_TILE << " instructions each into Core 0."
              << std::endl;
    std::cout << "Pattern: Independent Addressing (No Hazards)" << std::endl;
}