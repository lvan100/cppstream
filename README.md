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
		
	性能测试结果(MAP_UNFOLD on):
	
	 ENABLE_SKIP  off
	 ENABLE_LIMIT off
	
	 basic found: 2611628
	 basic time: 6 ms
	 stream found: 2611628
	 stream time: 302 ms
	
	 ENABLE_SKIP  off
	 ENABLE_LIMIT on
	
	 basic found: 5000
	 basic time: 0 ms
	 stream found: 5000
	 stream time: 1 ms
	
	 ENABLE_SKIP  on
	 ENABLE_LIMIT off
	
	 basic found: 2605156
	 basic time: 32 ms
	 stream found: 2605156
	 stream time: 307 ms
	
	 ENABLE_SKIP  on
	 ENABLE_LIMIT on
	
	 basic found: 5000
	 basic time: 0 ms
	 stream found: 5000
	 stream time: 1 ms
	
	性能测试结果(MAP_UNFOLD off):
	
	 ENABLE_SKIP  off
	 ENABLE_LIMIT off
	
	 basic found: 2614501
	 basic time: 6 ms
	 stream found: 2614501
	 stream time: 172 ms

	性能测试结果(STREAM_ONESTEP on):
	
	 ENABLE_SKIP  off
	 ENABLE_LIMIT off
	
	 basic found: 2610194
	 basic time: 6 ms
	 stream found: 2610194
	 stream time: 96 ms
	
	 ENABLE_SKIP  on
	 ENABLE_LIMIT off
	
	 basic found: 2607504
	 basic time: 32 ms
	 stream found: 2607504
	 stream time: 97 ms

可以看到，经典写法和Stream写法二者的性能相差了30~50倍不止，而且Stream写法的条件组合对性能的影响也很大，Stream写法的条件组合越精简，性能相差越小。