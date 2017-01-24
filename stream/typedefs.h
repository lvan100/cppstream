#pragma once

struct ST0 {
	int i = 0;
	ST0(int i) {
		this->i = i;
	}
};

struct ST1 {
	ST0 st0;
};

struct ST2 {
	ST1 st1;
};

struct ST3 {
	ST2 st2;
};