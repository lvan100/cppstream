#pragma once

struct ST0 {
	int i = 0;

public:
	ST0()
	{}

	ST0(int i) {
		set(i);
	}

	void set(int i) {
		this->i = i;
	}
};

struct ST1 {
	ST0 st0;

public:
	void set(int i) {
		st0.set(i);
	}
};

struct ST2 {
	ST1 st1;

public:
	void set(int i) {
		st1.set(i);
	}
};

struct ST3 {
	ST2 st2;

public:
	void set(int i) {
		st2.set(i);
	}
};