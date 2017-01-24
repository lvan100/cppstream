#include "stream.h"

#include <string>
#include <iostream>
using namespace std;

#include "typedefs.h"

int main()
{
	ST3 it[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

	int count = ArraySplitter<ST3>(it, __crt_countof(it))
		.stream()->map<ST2>([](const ST3& st)->ST2 {
			return st.st2;
		})->map<ST1>([](const ST2& st)->ST1 {
			return st.st1;
		})->map<ST0>([](const ST1& st)->ST0 {
			return st.st0;
		})->map<int>([](const ST0& st)->int {
			return st.i;
		})->filter([](const int& i)->bool {
			return i > 6;
		})->filter([](const int& i)->bool {
			return i < 16;
		})->count();

	cout << count;

	return 0;
}