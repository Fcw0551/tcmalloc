# 编译器与标准
CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -g -I./include  # C++17标准 + O2优化 + 所有警告 + 调试信息 + 头文件路径
LDFLAGS := -lpthread  # 多线程库链接

# 目录定义
SRC_DIR := src
BUILD_DIR := build
TARGET := mypool

# 自动扫描所有源文件
SRCS := $(wildcard $(SRC_DIR)/*.cc) main.cc
# 自动生成目标文件路径（所有.cc对应build/下的.o）
OBJS := $(patsubst %.cc, $(BUILD_DIR)/%.o, $(SRCS))

# 默认目标：编译生成可执行文件
all: $(TARGET)

# 链接所有目标文件生成最终可执行文件
$(TARGET): $(OBJS)
	@echo "Linking $@..."
	@$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@echo "✅ 编译完成！生成可执行文件：$@"

# 编译src目录下的源文件
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(@D)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译根目录下的main.cc
$(BUILD_DIR)/main.o: main.cc
	@mkdir -p $(@D)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理中间文件和可执行文件
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "🧹 清理完成！"

# 声明伪目标（避免和同名文件冲突）
.PHONY: all clean