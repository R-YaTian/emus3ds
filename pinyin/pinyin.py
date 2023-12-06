import re


def is_hex_within_range(hex_value):
    # 判断十六进制值是否在指定范围内
    high_byte = hex_value[:2]
    low_byte = hex_value[2:]
    if (0xD7 < int(high_byte, 16) <= 0xF7) and (0xA1 <= int(low_byte, 16) <= 0xFE):
        return True
    else:
        return False


def process_line(line):
    # 使用正则表达式提取汉字和=号后的第一个英文字母
    match = re.match(r"([\u4e00-\u9fa5]+)=(\w)", line)

    if match:
        chinese_chars = match.group(1)

        # 将汉字转为GBK编码的十六进制格式
        chinese_hex = ''.join([char.encode("gbk").hex().upper() for char in chinese_chars])

        # 判断是否在指定范围内
        if is_hex_within_range(chinese_hex):
            english_upper = match.group(2).upper()  # 提取=号后的第一个英文字母并转为大写
            return chinese_hex, english_upper

    return None


def process_file(input_file, output_file):
    data_list = []  # 保存提取的16进制和英文字母的列表

    with open(input_file, 'r', encoding='gbk') as infile:
        for line in infile:
            processed_line = process_line(line.strip())
            if processed_line:
                data_list.append(processed_line)

    # 按照提取的16进制大小进行排序
    sorted_data_list = sorted(data_list, key=lambda x: int(x[0], 16))

    # 写入到输出文件中
    with open(output_file, 'w') as outfile:
        counter = 0  # 初始化计数器
        for data in sorted_data_list:
            outfile.write(f"0x{data[0]}, '{data[1]}',")
            counter += 1
            if counter % 5 == 0:
                outfile.write('\n')  # 每5组数据换行
            else:
                outfile.write(' ')  # 每组数据之间用空格分隔


if __name__ == "__main__":
    input_file_path = "pinyin.txt"  # 替换为你的输入文件路径
    output_file_path = "output"  # 替换为你的输出文件路径

    process_file(input_file_path, output_file_path)
    print("处理完成。")
