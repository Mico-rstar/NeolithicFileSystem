# 编译器设置
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -I./src
LDFLAGS := -L./src
LDLIBS := 

# 源文件和测试文件路径
SRC_DIR := src
TEST_DIR := test

# 查找所有源文件和测试文件
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS := $(wildcard $(TEST_DIR)/test*.cpp)

# 生成对应的目标文件和可执行文件
OBJS := $(SRCS:.cpp=.o)
TEST_EXES := $(patsubst $(TEST_DIR)/%.cpp, %, $(TEST_SRCS))

# 默认目标
all: $(TEST_EXES)

# 编译测试可执行文件
%: $(TEST_DIR)/%.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@.o

# 编译源文件
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TEST_EXES)

.PHONY: all clean