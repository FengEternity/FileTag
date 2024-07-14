import os
import random

def generate_empty_files(num_files, save_path):
    # 定义文件后缀列表
    file_extensions = ['csv', 'doc', 'txt']

    # 检查保存路径是否存在，不存在则创建
    if not os.path.exists(save_path):
        os.makedirs(save_path)

    for i in range(num_files):
        # 随机选择一个文件后缀
        file_extension = random.choice(file_extensions)

        # 定义文件名
        file_name = f"empty_file_{i + 1}.{file_extension}"

        # 定义文件的完整路径
        file_path = os.path.join(save_path, file_name)

        # 创建空文件
        with open(file_path, 'w') as f:
            pass

        print(f"已创建文件: {file_path}")

if __name__ == "__main__":
    # 从用户输入获取生成文件的数量和保存路径
    num_files = int(input("请输入要生成的空文件数量: "))
    save_path = input("请输入文件保存路径: ")

    # 调用函数生成文件
    generate_empty_files(num_files, save_path)
