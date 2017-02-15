#include "tests.h"

#include "new.h"

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

template<typename T, int size>
void test(T(&t)[size]) {
	for (int i = 0; i < size;i++) {
		cout << t[i];
	}
	cout << endl;
}

int main()
{
	int ia_3[3] = { 0,1,2 };
	test(ia_3);

	int ia_4[4] = { 0,1,2,3 };
	test(ia_4);

	foo([]() {
		return 3;
	});

	foo([](int i) {
		return i + 3;
	}, 4);

	cout << endl;

	ST3 arr[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

	int count1 = make_stream(arr, __crt_countof(arr))
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

	cout << count1 << endl;

	//这一行是为了检验内存泄漏检测真实有效
	new int(5);

	int count2 = make_stream(arr, __crt_countof(arr))
		>> cpp::stream::map([](const ST3& st)->ST2 {
			return st.st2;
		}) >> cpp::stream::map([](const ST2& st)->ST1 {
			return st.st1;
		}) >> cpp::stream::map([](const ST1& st)->ST0 {
			return st.st0;
		}) >> cpp::stream::map([](const ST0& st)->int {
			return st.i;
		}) >> filter([](const int& i)->bool {
			return i > 6;
		}) >> filter([](const int& i)->bool {
			return i < 16;
		}) >> skip(4) >> limit(3) >> count();

	cout << count2 << endl;

	cout << endl;

#ifdef _DEBUG
	run_function_test();
#endif

#ifdef _DEBUG
	run_performance_test(1000);  // 测试内存泄露
#else
	run_performance_test(10000000); // 1千万数据
#endif

	cout << endl;

	return 0;
}

#ifdef _DEBUG
#undef new
#undef DEBUG_NEW
#endif