#pragma once

#include <time.h>

#include "typedefs.h"

#include <iostream>
using namespace std;

#include <chrono>
using namespace std::chrono;

#include "stream.h"
using namespace cpp::stream;

auto time_it(function<void()> test)
{
	auto then = high_resolution_clock::now();
	{
		test();
	}
	auto now = high_resolution_clock::now();

	return duration_cast<milliseconds>(now - then);
}

// #define ENABLE_SKIP
// #define ENABLE_LIMIT

void run_performance_test(int size) {

	srand((unsigned int)time(nullptr));

	ST3* arr = new ST3[size];
	for (int i = 0; i < size;i++) {
		arr[i].set(rand() % 5000);
	}

	auto basic_test = [&]() {

		int count = 0;
		int skip = 5000;
		int limit = 5000;

		for (int i = 0; i < size; i++) {

			int v = arr[i].st2.st1.st0.i;

#if 0
			ST2 st2 = arr[i].st2;
			ST1 st1 = st2.st1;
			ST0 st0 = st1.st0;
			int v = st0.i;
#endif

			if (v > 2600 && v < 4000) {

#ifdef ENABLE_SKIP
				if (--skip < 0) {
#endif

#ifdef ENABLE_LIMIT
					if (--limit < 0) {
						break;
					}
#endif

					count++;
#ifdef ENABLE_SKIP
				}
#endif
			}
		}

		cout << "basic found: " << count << endl;
	};

	auto stream_test = [&]() {
		int count = make_stream(arr, size)
			->map([](const ST3& st)->ST2 {
				return st.st2;
			})->map([](const ST2& st)->ST1 {
				return st.st1;
			})->map([](const ST1& st)->ST0 {
				return st.st0;
			})->map([](const ST0& st)->int {
				return st.i;
			})->filter([](const int& i)->bool {
				return i > 2600;
			})->filter([](const int& i)->bool {
				return i < 4000;
			})

#ifdef ENABLE_SKIP
			->skip(5000)
#endif

#ifdef ENABLE_LIMIT
			->limit(5000)
#endif

			->count();

		cout << "stream found: " << count << endl;
	};

	auto time = time_it(basic_test);
	cout << "basic time: " << time.count() << " ms" << endl;

	time = time_it(stream_test);
	cout << "stream time: " << time.count() << " ms" << endl;
}

/***********************************************************

 性能测试结果:

 ENABLE_SKIP  off
 ENABLE_LIMIT off

 basic found: 2612031
 basic time: 6 ms
 stream found: 2612031
 stream time: 324 ms

 ENABLE_SKIP  off
 ENABLE_LIMIT on

 basic found: 5000
 basic time: 0 ms
 stream found: 5000
 stream time: 1 ms

 ENABLE_SKIP  on
 ENABLE_LIMIT off

 basic found: 2608040
 basic time: 31 ms
 stream found: 2608040
 stream time: 325 ms

 ENABLE_SKIP  on
 ENABLE_LIMIT on

 basic found: 5000
 basic time: 0 ms
 stream found: 5000
 stream time: 1 ms

************************************************************/