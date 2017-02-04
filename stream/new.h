#pragma once

#include <stdio.h>
#include <malloc.h>
#include <string.h>

/**
 * 内存分配节点
 */
struct MemNode {

	/**
	 * 内存分配所在的函数
	 */
	char* func = nullptr;

	/**
	 * 内存分配所在的文件
	 */
	char* file = nullptr;

	/**
	 * 内存分配的位置
	 */
	void* ptr = nullptr;

	/**
	 * 内存分配的大小
	 */
	size_t size = 0;

	/**
	 * 内存分配所在的行号
	 */
	int line = 0;

};

#define MAX_MEM_NODE 1024

/**
 * 内存分配列表
 */
class MemList {

	/**
	 * 内存分配节点列表
	 */
	MemNode _nodes[MAX_MEM_NODE];

public:
	~MemList() {
		dump();
	}

	void add(void* ptr, size_t size, char* file, char* func, int line) {
		for (int i = 0; i < __crt_countof(_nodes); i++) {
			if (_nodes[i].ptr == nullptr) {
				_nodes[i].file = file;
				_nodes[i].func = func;
				_nodes[i].line = line;
				_nodes[i].size = size;
				_nodes[i].ptr = ptr;
				break;
			}
		}
	}

	void remove(void* ptr) {
		for (int i = 0; i < __crt_countof(_nodes); i++) {
			if (_nodes[i].ptr == ptr) {
				_nodes[i].file = nullptr;
				_nodes[i].func = nullptr;
				_nodes[i].ptr = nullptr;
				_nodes[i].line = 0;
				_nodes[i].size = 0;
				break;
			}
		}
	}

	void dump() {
		for (int i = 0; i < __crt_countof(_nodes); i++) {
			if (_nodes[i].ptr != nullptr) {
				printf("%s:%d %s\n", _nodes[i].file, _nodes[i].line, _nodes[i].func);
			}
		}
	}

};

#ifdef _DEBUG

MemList theMemList;

void* operator new(size_t size, char* file, char* func, int line){
	void* ptr = malloc(size);
	if (ptr != nullptr) {
		theMemList.add(ptr, size, file, func, line);
	}
	return ptr;
}

void operator delete(void* ptr, char* file, char* func, int line) {
	// 这个函数只是为了消除编译器对new的警告
}

void operator delete(void* ptr) {
	if (ptr != nullptr) {
		theMemList.remove(ptr);
		free(ptr);
	}
}

#endif