# 编译器和选项
CXX = g++
CXXFLAGS = -Wall -Wextra -g -I../src

# 源路径
SRC_DIR = ../src

# 所有 src 下的 .cpp 文件
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

# 对应的 .o 文件
SRC_OBJS = $(SRC_FILES:.cpp=.o)

# 当前目录下的源文件
MKFS_SRC = mkfs.cpp
MKFS_OBJ = $(MKFS_SRC:.cpp=.o)

# 最终目标可执行文件，输出到上级目录
TARGET = ../mmks.tst  # 注意：输出路径改到了上级目录

# 默认目标
all: $(TARGET)

# 生成最终可执行文件
$(TARGET): $(MKFS_OBJ) $(SRC_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# 编译当前目录下的 mkfs.cpp
$(MKFS_OBJ): $(MKFS_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 使用模式规则编译 src 下的所有 .cpp 文件
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理构建产物（注意只清理上级目录中的可执行文件）
clean:
	rm -f $(MKFS_OBJ) $(SRC_OBJS) ../mkfs

.PHONY: all clean