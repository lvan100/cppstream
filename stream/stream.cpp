#include "stream.h"
using namespace cpp::stream;

#ifdef _DEBUG
#define DEBUG_NEW new(__FILE__, __FUNCTION__ ,__LINE__)
#define new DEBUG_NEW
#endif

#include <string>
#include <iostream>
using namespace std;

#include "typedefs.h"

int main()
{
	ST3 arr[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

	int count = make_stream(arr, __crt_countof(arr))
		->map<ST2>([](const ST3& st)->ST2 {
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
		})->skip(4)->limit(4)->count();

	cout << count << endl;

	//这一行是为了检验内存泄漏检测真实有效
	new int(5);

	return 0;
}

#ifdef _DEBUG
#undef new
#undef DEBUG_NEW
#endif