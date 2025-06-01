#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <system_error>
#include <cerrno>

#include "../src/file.h"

File f;

std::string readFile(const std::string &filePath)
{
    // 创建输入文件流
    std::ifstream file(filePath, std::ios::binary);

    // 检查文件是否成功打开
    if (!file.is_open())
    {
        throw std::system_error(errno, std::generic_category(), "无法打开文件: " + filePath);
    }

    // 使用字符串流作为缓冲区
    std::stringstream buffer;

    try
    {
        // 将整个文件内容读入缓冲区
        buffer << file.rdbuf();
    }
    catch (const std::exception &e)
    {
        // 关闭文件后重新抛出异常
        file.close();
        throw std::runtime_error("读取文件错误: " + std::string(e.what()));
    }

    // 确保文件被正确关闭
    file.close();

    // 将内容转换为字符串返回
    return buffer.str();
}

void FreeSpace()
{
    // TODO: 显示当前实际可用空间（以KB为单位）
    std::cout << "FreeSpace command executed" << std::endl;
    printf("FreeSpace(blocks): %d\n", f.fstat().nfb);
}

void CreateDir(const std::string &path)
{
    // TODO: 创建指定路径的目录
    std::cout << "CreateDir: " << path << std::endl;
    char pa[MAXPATH];
    std::memset(pa, 0, MAXPATH);
    std::memmove(pa, path.c_str(), path.size());
    f.mkdir(pa);
}

void CopyFile(const std::string &src, const std::string &dest)
{
    // TODO: 复制文件到目标路径
    std::cout << "CopyFile: " << src << " -> " << dest << std::endl;

    std::string content = readFile(src);
    char *pa = new char[content.size() + 1];
    std::memset(pa, 0, content.size() + 1);
    std::memmove(pa, content.c_str(), content.size());

    char dpa[MAXPATH];
    std::memset(dpa, 0, MAXPATH);
    std::memmove(dpa, dest.c_str(), dest.size());
    file *op = f.open(dpa, true, true);
    f.write(op, pa, content.size() + 1);
    f.close(op);
}

void Tree(const std::string &path)
{
    // TODO: 显示目录树结构
    std::cout << "Tree: " << path << std::endl;
    char pa[MAXPATH];
    std::memset(pa, 0, MAXPATH);
    std::memmove(pa, path.c_str(), path.size());
    f.tree(pa);
}
// --- 函数接口声明结束 ---

// 辅助函数：转换字符串为小写
std::string toLower(const std::string &str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });
    return result;
}

// 辅助函数：去除首尾空格
std::string trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// 命令解析器
void parseCommand(const std::string &input)
{
    if (input.empty())
        return;

    // 分割命令和参数
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;

    while (std::getline(iss, token, ','))
    {
        tokens.push_back(trim(token));
    }

    if (tokens.empty())
        return;

    // 转换为小写统一处理
    std::string command = toLower(tokens[0]);

    // 根据命令调用相应函数
    if (command == "freespace")
    {
        if (tokens.size() != 1)
        {
            std::cerr << "Invalid parameters for FreeSpace" << std::endl;
            return;
        }
        FreeSpace();
    }
    else if (command == "createdir")
    {
        if (tokens.size() != 2)
        {
            std::cerr << "Usage: CreateDir, <path>" << std::endl;
            return;
        }
        CreateDir(tokens[1]);
    }
    else if (command == "copyfile")
    {
        if (tokens.size() != 3)
        {
            std::cerr << "Usage: CopyFile, <src>, <dest>" << std::endl;
            return;
        }
        CopyFile(tokens[1], tokens[2]);
    }
    else if (command == "tree")
    {
        if (tokens.size() != 2)
        {
            std::cerr << "Usage: Tree, <path>" << std::endl;
            return;
        }
        Tree(tokens[1]);
    }
    else
    {
        std::cerr << "Unknown command: " << tokens[0] << std::endl;
    }
}

int main()
{
    std::string input;

    std::cout << "Command Shell (type 'exit' to quit)" << std::endl;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, input);

        // 检查退出命令
        if (toLower(trim(input)) == "exit")
            break;

        // 执行命令解析
        parseCommand(input);
    }

    return 0;
}