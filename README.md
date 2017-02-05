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


 下面是性能测试结果(找出1千万个数据中符合条件的值):
	
	 ENABLE_SKIP  off
	 ENABLE_LIMIT off
	
	 basic found: 2612060
	 basic time: 109 ms
	 stream found: 2612060
	 stream time: 28623 ms
	
	 ENABLE_SKIP  off
	 ENABLE_LIMIT on
	
	 basic found: 5000
	 basic time: 0 ms
	 stream found: 5000
	 stream time: 56 ms
	
	 ENABLE_SKIP  on
	 ENABLE_LIMIT off
	
	 basic found: 2604543
	 basic time: 108 ms
	 stream found: 2604543
	 stream time: 28507 ms
	
	 ENABLE_SKIP  on
	 ENABLE_LIMIT on
	
	 basic found: 5000
	 basic time: 0 ms
	 stream found: 5000
	 stream time: 105 ms

 性能测试结果相当的不容乐观，二者竟然差了1、2百倍。