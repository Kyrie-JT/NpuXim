#pragma once
#include <deque>
#include <memory>
// 如果你需要 Instruction 的定义，可能需要包含 Core.h 或者单独把 Instruction 提出来
// 但因为 Core.h 已经包含了 Tile 的定义，这里可能是多余的文件或者应该反过来引用
// 这里的 Tile.h 当前只有一行 include< 且未完成
// 建议把 Tile 定义放回单独的文件，或者如果它已经很紧密地和 Core 绑定，就保持在 Core.h 里。
// 鉴于 Core.h 目前包含了 Tile 的完整定义，我会忽略这个未完成的文件，直接操作 Core.h
