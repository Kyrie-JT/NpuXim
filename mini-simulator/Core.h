#include <deque>
#include <memory>
#include <queue>
#include <string>

typedef uint64_t cycle_type;
typedef uint64_t addr_type;
enum class Opcode { GEMM };  // 现在只仿真矩阵乘法

struct Instruction {
    Opcode opcode;  // 指令类型
    cycle_type offset = 0;
    cycle_type start_cycle;
    cycle_type finish_cycle;  // 记录开始周期和结束周期
    cycle_type compute_size;
    cycle_type transfer_cycle;
    addr_type src_addr;
    addr_type dest_addr;
    bool finished = false;
    uint32_t tile_k, tile_m, tile_n;
};

struct Tile {
public:
    std::deque<std::unique_ptr<Instruction>> instructions;

    // Tile 可以有一个构造函数或者初始化方法，这里保持简单
};

class Core {
public:
    Core(uint32_t id, uint64_t bandWidth);
    virtual ~Core() = default;
    void Cycle();
    void Compute();
    void finish_cube_pipeline();
    void CalculateLatency(Instruction& instr);
    void PushTile(std::unique_ptr<Tile> tile);  // 把Tile装入Core中
    cycle_type DataTransfer(const std::unique_ptr<Instruction>& instr);
    cycle_type Compute_GEMM(const Instruction& instr);
    bool isRunning() const;

private:
    uint32_t id;
    uint32_t core_Height = 16;
    uint32_t core_Width = 16;
    uint64_t bandWidth;                                      // Core中的带宽，一个时钟周期可以传递多少数据
    std::queue<std::unique_ptr<Instruction>> cube_pipeline;  // 模拟计算中的流水线

    std::deque<std::unique_ptr<Tile>> tiles;
    std::queue<std::unique_ptr<Instruction>> _ex_inst_queue;  // 用于判断指令该发入vector单元还是Cube单元
    cycle_type _core_cycles;
    uint32_t tile_rr = 0;  // 用于轮询搜索Core中的Tile，防止一直重复
};
