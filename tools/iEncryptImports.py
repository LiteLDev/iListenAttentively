#!/usr/bin/env python3
import sys
import itertools
import pefile

NON_ZERO_BYTE_MODULUS = 255


def encode_non_zero_byte(value, key):
    return ((value + key - 1) % NON_ZERO_BYTE_MODULUS) + 1


def encode_symbol_name(func_name, key):
    encrypted_name = bytes(encode_non_zero_byte(byte, key) for byte in func_name)
    if any(byte == 0 for byte in encrypted_name):
        raise ValueError("encrypted import name unexpectedly contains a NUL byte")
    return encrypted_name


def encrypt_imports_for_dll(pe, dll_name_target, xor_key, is_delay):
    """加密指定DLL的所有导入函数名字符串（支持标准导入和延迟导入），不加密DLL名称"""
    if is_delay:
        if not hasattr(pe, 'DIRECTORY_ENTRY_DELAY_IMPORT'):
            return
        entries = pe.DIRECTORY_ENTRY_DELAY_IMPORT
    else:
        if not hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
            return
        entries = pe.DIRECTORY_ENTRY_IMPORT

    for entry in entries:
        # 获取DLL名称
        dll_name_rva = entry.struct.szName if is_delay else entry.struct.Name
        dll_name = pe.get_string_at_rva(dll_name_rva)
        if dll_name != dll_name_target:
            continue

        print(f"Processing {dll_name.decode()} ({'delay' if is_delay else 'normal'})...")

        # 不再加密DLL名称，跳过

        # 遍历INT表
        int_rva = entry.struct.pINT if is_delay else entry.struct.OriginalFirstThunk
        thunk_size = 8 if pe.PE_TYPE == pefile.OPTIONAL_HEADER_MAGIC_PE_PLUS else 4
        ordinal_flag = 0x80000000 if thunk_size == 4 else 0x8000000000000000

        for idx in itertools.count():
            data = pe.get_data(int_rva + idx * thunk_size, thunk_size)
            addr_of_data = pefile.struct.unpack("<I" if thunk_size == 4 else "<Q", data)[0]
            if addr_of_data == 0:
                break

            # 跳过按序号导入的项
            if addr_of_data & ordinal_flag:
                continue

            # 获取函数名（位于Hint之后2字节）
            name_rva = addr_of_data & (0x7FFFFFFF if thunk_size == 4 else 0x7FFFFFFFFFFFFFFF)
            func_name = pe.get_string_at_rva(name_rva + 2)
            if func_name:
                name_offset = pe.get_offset_from_rva(name_rva + 2)
                encrypted_name = encode_symbol_name(func_name, xor_key)
                pe.set_bytes_at_offset(name_offset, encrypted_name)

def encrypt_imports(pe_path, xor_key):
    if xor_key <= 0 or xor_key >= NON_ZERO_BYTE_MODULUS:
        raise ValueError("xor_key must be in the range [1, 254]")
    pe = pefile.PE(pe_path)
    target_dll = b"bedrock_runtime.dll"
    encrypt_imports_for_dll(pe, target_dll, xor_key, is_delay=True)
    encrypt_imports_for_dll(pe, target_dll, xor_key, is_delay=False)
    pe.write(pe_path)
    print(f"Encryption completed for {pe_path} with key {xor_key:#04x}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: encrypt_imports.py <PE file> <xor_key>")
        print("Example: encrypt_imports.py plugin.dll 0xAB")
        sys.exit(1)
    try:
        xor_key = int(sys.argv[2], 0)
    except ValueError:
        print("Error: xor_key must be an integer")
        sys.exit(1)
    encrypt_imports(sys.argv[1], xor_key)
