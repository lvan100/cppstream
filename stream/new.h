#pragma once

#include <stdio.h>
#include <malloc.h>
#include <string.h>

/**
 * �ڴ����ڵ�
 */
struct MemNode {

	/**
	 * �ڴ�������ڵĺ���
	 */
	char* func = nullptr;

	/**
	 * �ڴ�������ڵ��ļ�
	 */
	char* file = nullptr;

	/**
	 * �ڴ�����λ��
	 */
	void* ptr = nullptr;

	/**
	 * �ڴ����Ĵ�С
	 */
	size_t size = 0;

	/**
	 * �ڴ�������ڵ��к�
	 */
	int line = 0;

};

#define MAX_MEM_NODE 1024

/**
 * �ڴ�����б�
 */
class MemList {

	/**
	 * �ڴ����ڵ��б�
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
	// �������ֻ��Ϊ��������������new�ľ���
}

void operator delete(void* ptr) {
	if (ptr != nullptr) {
		theMemList.remove(ptr);
		free(ptr);
	}
}

#endif