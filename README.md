# cppstream

仿照 Java8 Stream 实现的C++版本，使用时只需要包含一个头文件。

	#include "stream.h"
	using namespace cpp::stream;

同时提供了 -> 和 >> 两种类型的操作符调用:

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

	int count2 = make_stream(arr, __crt_countof(arr))
		>> map([](const ST3& st)->ST2 {
			return st.st2;
		}) >> map([](const ST2& st)->ST1 {
			return st.st1;
		}) >> map([](const ST1& st)->ST0 {
			return st.st0;
		}) >> map([](const ST0& st)->int {
			return st.i;
		}) >> filter([](const int& i)->bool {
			return i > 6;
		}) >> filter([](const int& i)->bool {
			return i < 16;
		}) >> skip(4) >> limit(3) >> count();

	cout << count2 << endl;


经过几次优化后，经典写法和Stream写法之间的效率差距在逐渐减少，目前已经可以控制在3倍以内。
		

测试数组：

	ST3* arr = new ST3[size];
	for (int i = 0; i < size;i++) {
		arr[i].set(rand() % 5000);
	}

经典写法：

	int count = 0;
	int skip = skip_count;
	int limit = limit_count;
	
	for (int i = 0; i < size; i++) {
		int v = arr[i].st2.st1.st0.i;
		if (v > down_limit && v < up_limit) {
			count++;
		}
	}

Stream 写法：

	int count = make_stream(arr, size)
		->filter([&](const ST3& st)->bool {
			int i = st.st2.st1.st0.i;
			return i > down_limit && i < up_limit;
		})->quick_count();


当size=10000000时，查找2600至4000之间的数据，测试结果如下：
	
	basic time: 59 ms
	stream time: 120 ms

如果在此基础上，查找第500000到1000000个符合条件的数据，测试结果如下：

	basic time: 24 ms
	stream time: 48 ms

经典写法：

	int count = 0;
	int skip = skip_count;
	int limit = limit_count;
	
	for (int i = 0; i < size; i++) {
		int v = arr[i].st2.st1.st0.i;
		if (v > down_limit && v < up_limit) {
			if (--skip < 0) {
				if (--limit < 0) {
					break;
				}
				count++;
			}
		}
	}

Stream 写法：

	int count = make_stream(arr, size)
			->filter([&](const ST3& st)->bool {
				int i = st.st2.st1.st0.i;
				return i > down_limit && i < up_limit;
			})->skip(skip_count)->limit(limit_count)->quick_count();

虽然二者在执行效率上差距还是很大，但是在写法上明显后者更胜一筹。