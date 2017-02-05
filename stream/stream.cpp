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

template <typename F>
typename ft_0<F>::ret foo(F f) {
	return f();
}

template <typename F, typename T>
typename ft_1<F, T>::ret foo(F f, T t) {
	return f(t);
}

int main()
{
	foo([]() {
		return 3;
	});

	foo([](int i) {
		return i + 3;
	}, 4);

	ST3 arr[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

	int count = make_stream(arr, __crt_countof(arr))
		->map([](const ST3& st)->ST2 {
			return st.st2;
		})->map([](const ST2& st)->ST1 {
			return st.st1;
		})->map([](const ST1& st)->ST0 {
			return st.st0;
		})->map([](const ST0& st)->int {
			return st.i;
		})->filter([](const int& i)->bool {
			return i > 6;
		})->filter([](const int& i)->bool {
			return i < 16;
		})->skip(4)->limit(4)->count();

	cout << count << endl;

	//这一行是为了检验内存泄漏检测真实有效
	new int(5);

	int count2 = make_stream(arr, __crt_countof(arr))
		>> map<ST3>([](const ST3& st)->ST2 {
			return st.st2;
		}) >> map<ST2>([](const ST2& st)->ST1 {
			return st.st1;
		}) >> map<ST1>([](const ST1& st)->ST0 {
			return st.st0;
		}) >> map<ST0>([](const ST0& st)->int {
			return st.i;
		}) >> filter<int>([](const int& i)->bool {
			return i > 6;
		}) >> filter<int>([](const int& i)->bool {
			return i < 16;
		}) >> skip<int>(4) >> limit<int>(3) >> _count<int>();

	cout << count2 << endl;

	return 0;
}

#ifdef _DEBUG
#undef new
#undef DEBUG_NEW
#endif