#include "Core.h"

#include <iostream>
Core::Core(uint32_t id, uint64_t bandWidth) : id(id), bandWidth(bandWidth), _core_cycles(0) {}

void Core::Cycle() {
    // if(tiles.empty())

    ++_core_cycles;
    Compute();  // 芯片后端进行计算,逆序调用 "Compute、取指" 以模拟流水
    // 优先执行当前 Tile (tile_rr)，只有当它为空时，才寻找下一个有指令的 Tile
    if (tiles[tile_rr]->instructions.empty()) {
        for (int offset = 1; offset < tiles.size(); offset++) {
            int candidate = (tile_rr + offset) % tiles.size();
            if (!tiles[candidate]->instructions.empty()) {
                tile_rr = candidate;
                break;
            }
        }
    }

    // 执行当前选定的 Tile（如果所有 Tile 都空了，这里自然会跳过）
    if (!tiles[tile_rr]->instructions.empty()) {
        // // --   LOG1
        // std::cout << "[Cycle " << _core_cycles << "] Fetching from Tile " << tile_rr << std::endl;
        auto& inst = tiles[tile_rr]->instructions.front();
        
        if (inst->opcode == Opcode::GEMM) {
            CalculateLatency(*inst);
            // //   ----------LOG2--------
            // std::cout << "  -> Moving inst to ex_queue" << std::endl;
            _ex_inst_queue.push(std::move(inst));
        }
        tiles[tile_rr]->instructions.pop_front();
        
        // 【关键逻辑】不再每执行一条就切换 Tile 
        // tile_rr = (i + 1) % tiles.size(); 
    }
}

void Core::Compute() {       // 这里把指令发入不同的pipeline计算。现在只有cube_pipeline
                             // offset的计算相当于给双缓冲留了接口
    finish_cube_pipeline();  // 让已完成的指令退休
    if (!_ex_inst_queue.empty()) {
        // // --- [Log 3] 从发射队列取出 ---
        // std::cout << "[Cycle " << _core_cycles << "] Pop from ex_queue" << std::endl;

        auto front = std::move(_ex_inst_queue.front());
        _ex_inst_queue.pop();  // 必须移除队首元素
        CalculateLatency(*front);
        // 初始化就按照cube_pipeline为空来，从当前周期开始计算
        cycle_type last_start_cycle = _core_cycles;
        cycle_type offset = 0;
        if (!cube_pipeline.empty()) {
            last_start_cycle = cube_pipeline.back()->start_cycle;
            offset = cube_pipeline.back()->transfer_cycle;
        }
        cycle_type start_cycle = last_start_cycle + offset;

        if (start_cycle < _core_cycles) {
            front->start_cycle = _core_cycles;
        } else {
            front->start_cycle = start_cycle;
        }
        front->finish_cycle = front->start_cycle + front->compute_size;
        cube_pipeline.push(std::move(front));
    }
}

// 计算指令的传输周期和计算周期
void Core::CalculateLatency(Instruction& instr) {
    cycle_type total_ops = (cycle_type)instr.tile_k * instr.tile_m * instr.tile_n;
    cycle_type ops_per_cycle = core_Height * core_Width;

    // 修复：传输数据量应该是 M * K * 4字节(float)，而不是 total_ops
    cycle_type data_size_bytes = (cycle_type)instr.tile_m * instr.tile_k * 4;
    instr.transfer_cycle = (data_size_bytes + bandWidth - 1) / bandWidth;

    instr.compute_size = (total_ops + ops_per_cycle - 1) / ops_per_cycle;
}

void Core::PushTile(std::unique_ptr<Tile> tile) {
    tiles.push_back(std::move(tile));
}

bool Core::isRunning() const {
    // 只要有任何一个队列不为空，就认为 Core 还在忙
    if (!_ex_inst_queue.empty() || !cube_pipeline.empty()) {
        return true;
    }
    for (const auto& tile : tiles) {
        if (!tile->instructions.empty()) {
            return true;
        }
    }
    return false;
}

void Core::finish_cube_pipeline() {
    if (!cube_pipeline.empty()) {
        while (cube_pipeline.front()->finish_cycle <= _core_cycles) {
            // // --- [Log 5] 指令退休 ---
            // std::cout << "[Cycle " << _core_cycles << "] Retire instruction" << std::endl;
            cube_pipeline.pop();
            if (cube_pipeline.empty()) break;
        }
    }
}
