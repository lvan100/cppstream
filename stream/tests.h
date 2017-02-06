#pragma once

#include <time.h>

#include "typedefs.h"

#include <iostream>
using namespace std;

#include <chrono>
using namespace std::chrono;

#include "stream.h"
using namespace cpp::stream;

/**
 * ���㺯��ִ��3�ε�ƽ��ʱ��
 */
auto time_it(function<void()> test) {

	high_resolution_clock::duration all(0);

	for (int i = 0; i < 3; i++) {
		auto then = high_resolution_clock::now();
		test();
		all += high_resolution_clock::now() - then;
	}

	return duration_cast<milliseconds>(all) / 3;
}

/**
 * ִ�����ܲ���
 */
void run_performance_test(int size) {

	srand((unsigned int)time(nullptr));

	ST3* arr = new ST3[size];
	for (int i = 0; i < size;i++) {
		arr[i].set(rand() % 5000);
	}

	const int up_limit = 4000;
	const int down_limit = 2600;

	const int skip_count = 500000;
	const int limit_count = 500000;

	// B0
	{
		auto basic_test = [&]() {

			int count = 0;
			int skip = skip_count;
			int limit = limit_count;

			for (int i = 0; i < size; i++) {
				int v = arr[i].st2.st1.st0.i;
				if (v > down_limit && v < up_limit) {
					count++;
				}
			}

			cout << "basic found: " << count << endl;
		};

		auto time = time_it(basic_test);
		cout << "basic time: " << time.count() << " ms" << endl;

		auto basic_unfold_test = [&]() {

			int count = 0;
			int skip = skip_count;
			int limit = limit_count;

			for (int i = 0; i < size; i++) {

				ST2 st2 = arr[i].st2;
				ST1 st1 = st2.st1;
				ST0 st0 = st1.st0;
				int v = st0.i;

				if (v > down_limit && v < up_limit) {
					count++;
				}
			}

			cout << "basic_unfold found: " << count << endl;
		};

		time = time_it(basic_unfold_test);
		cout << "basic_unfold time: " << time.count() << " ms" << endl;
	}

	cout << endl;

	// S0
	{
		auto stream_test = [&]() {
			int count = make_stream(arr, size)
				->map([](const ST3& st)->int {
					return st.st2.st1.st0.i;
				})->filter([&](const int& i)->bool {
					return i > down_limit && i < up_limit;
				})->count();

			cout << "stream found: " << count << endl;
		};

		auto time = time_it(stream_test);
		cout << "stream time: " << time.count() << " ms" << endl;

		auto stream_unfold_test = [&]() {
			int count = make_stream(arr, size)
				->map([](const ST3& st)->ST2 {
					return st.st2;
				})->map([](const ST2& st)->ST1 {
					return st.st1;
				})->map([](const ST1& st)->ST0 {
					return st.st0;
				})->map([](const ST0& st)->int {
					return st.i;
				})->filter([&](const int& i)->bool {
					return i > down_limit && i < up_limit;
				})->count();

			cout << "stream_unfold found: " << count << endl;
		};

		time = time_it(stream_unfold_test);
		cout << "stream_unfold time: " << time.count() << " ms" << endl;

		auto stream_onestep_test = [&]() {
			int count = make_stream(arr, size)
				->filter([&](const ST3& st)->bool {
					int i = st.st2.st1.st0.i;
					return i > down_limit && i < up_limit;
				})->count();

			cout << "stream_onestep found: " << count << endl;
		};

		time = time_it(stream_onestep_test);
		cout << "stream_onestep time: " << time.count() << " ms" << endl;
	}

	cout << endl;

	// B1
	{
		auto basic_skip_test = [&]() {

			int count = 0;
			int skip = skip_count;
			int limit = limit_count;

			for (int i = 0; i < size; i++) {
				int v = arr[i].st2.st1.st0.i;
				if (v > down_limit && v < up_limit) {
					if (--skip < 0) {
						count++;
					}
				}
			}

			cout << "basic_skip found: " << count << endl;
		};

		auto time = time_it(basic_skip_test);
		cout << "basic_skip time: " << time.count() << " ms" << endl;

		auto basic_unfold_skip_test = [&]() {

			int count = 0;
			int skip = skip_count;
			int limit = limit_count;

			for (int i = 0; i < size; i++) {

				ST2 st2 = arr[i].st2;
				ST1 st1 = st2.st1;
				ST0 st0 = st1.st0;
				int v = st0.i;

				if (v > down_limit && v < up_limit) {
					if (--skip < 0) {
						count++;
					}
				}
			}

			cout << "basic_unfold_skip found: " << count << endl;
		};

		time = time_it(basic_unfold_skip_test);
		cout << "basic_unfold_skip time: " << time.count() << " ms" << endl;
	}

	cout << endl;

	// S1
	{
		auto stream_skip_test = [&]() {
			int count = make_stream(arr, size)
				->map([](const ST3& st)->int {
					return st.st2.st1.st0.i;
				})->filter([&](const int& i)->bool {
					return i > down_limit && i < up_limit;
				})->skip(skip_count)->count();

			cout << "stream_skip found: " << count << endl;
		};

		auto time = time_it(stream_skip_test);
		cout << "stream_skip time: " << time.count() << " ms" << endl;

		auto stream_unfold_skip_test = [&]() {
			int count = make_stream(arr, size)
				->map([](const ST3& st)->ST2 {
					return st.st2;
				})->map([](const ST2& st)->ST1 {
					return st.st1;
				})->map([](const ST1& st)->ST0 {
					return st.st0;
				})->map([](const ST0& st)->int {
					return st.i;
				})->filter([&](const int& i)->bool {
					return i > down_limit && i < up_limit;
				})->skip(skip_count)->count();

			cout << "stream_unfold_skip found: " << count << endl;
		};

		time = time_it(stream_unfold_skip_test);
		cout << "stream_unfold_skip time: " << time.count() << " ms" << endl;

		auto stream_onestep_skip_test = [&]() {
			int count = make_stream(arr, size)
				->filter([&](const ST3& st)->bool {
					int i = st.st2.st1.st0.i;
					return i > down_limit && i < up_limit;
				})->skip(skip_count)->count();

			cout << "stream_onestep_skip found: " << count << endl;
		};

		time = time_it(stream_onestep_skip_test);
		cout << "stream_onestep_skip time: " << time.count() << " ms" << endl;
	}

	cout << endl;

	// B2
	{
		auto basic_limit_test = [&]() {

			int count = 0;
			int skip = skip_count;
			int limit = limit_count;

			for (int i = 0; i < size; i++) {
				int v = arr[i].st2.st1.st0.i;
				if (v > down_limit && v < up_limit) {
					if (--limit < 0) {
						break;
					}
					count++;
				}
			}

			cout << "basic_limit found: " << count << endl;
		};

		auto time = time_it(basic_limit_test);
		cout << "basic_limit time: " << time.count() << " ms" << endl;

		auto basic_unfold_limit_test = [&]() {

			int count = 0;
			int skip = skip_count;
			int limit = limit_count;

			for (int i = 0; i < size; i++) {

				ST2 st2 = arr[i].st2;
				ST1 st1 = st2.st1;
				ST0 st0 = st1.st0;
				int v = st0.i;

				if (v > down_limit && v < up_limit) {
					if (--limit < 0) {
						break;
					}
					count++;
				}
			}

			cout << "basic_unfold_limit found: " << count << endl;
		};

		time = time_it(basic_unfold_limit_test);
		cout << "basic_unfold_limit time: " << time.count() << " ms" << endl;
	}

	cout << endl;
	
	// S2
	{
		auto stream_limit_test = [&]() {
			int count = make_stream(arr, size)
				->map([](const ST3& st)->int {
					return st.st2.st1.st0.i;
				})->filter([&](const int& i)->bool {
					return i > down_limit && i < up_limit;
				})->limit(limit_count)->count();

			cout << "stream_limit found: " << count << endl;
		};

		auto time = time_it(stream_limit_test);
		cout << "stream_limit time: " << time.count() << " ms" << endl;

		auto stream_unfold_limit_test = [&]() {
			int count = make_stream(arr, size)
				->map([](const ST3& st)->ST2 {
					return st.st2;
				})->map([](const ST2& st)->ST1 {
					return st.st1;
				})->map([](const ST1& st)->ST0 {
					return st.st0;
				})->map([](const ST0& st)->int {
					return st.i;
				})->filter([&](const int& i)->bool {
					return i > down_limit && i < up_limit;
				})->limit(limit_count)->count();

			cout << "stream_unfold_limit found: " << count << endl;
		};

		time = time_it(stream_unfold_limit_test);
		cout << "stream_unfold_limit time: " << time.count() << " ms" << endl;

		auto stream_onestep_limit_test = [&]() {
			int count = make_stream(arr, size)
				->filter([&](const ST3& st)->bool {
					int i = st.st2.st1.st0.i;
					return i > down_limit && i < up_limit;
				})->limit(limit_count)->count();

			cout << "stream_onestep_limit found: " << count << endl;
		};

		time = time_it(stream_onestep_limit_test);
		cout << "stream_onestep_limit time: " << time.count() << " ms" << endl;
	}

	cout << endl;

	// B3
	{
		auto basic_skip_limit_test = [&]() {

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

			cout << "basic_skip_limit found: " << count << endl;
		};

		auto time = time_it(basic_skip_limit_test);
		cout << "basic_skip_limit time: " << time.count() << " ms" << endl;

		auto basic_unfold_skip_limit_test = [&]() {

			int count = 0;
			int skip = skip_count;
			int limit = limit_count;

			for (int i = 0; i < size; i++) {

				ST2 st2 = arr[i].st2;
				ST1 st1 = st2.st1;
				ST0 st0 = st1.st0;
				int v = st0.i;

				if (v > down_limit && v < up_limit) {
					if (--skip < 0) {
						if (--limit < 0) {
							break;
						}
						count++;
					}
				}
			}

			cout << "basic_unfold_skip_limit found: " << count << endl;
		};

		time = time_it(basic_unfold_skip_limit_test);
		cout << "basic_unfold_skip_limit time: " << time.count() << " ms" << endl;
	}
	
	cout << endl;
	
	// S3
	{
		auto stream_skip_limit_test = [&]() {
			int count = make_stream(arr, size)
				->map([](const ST3& st)->int {
					return st.st2.st1.st0.i;
				})->filter([&](const int& i)->bool {
					return i > down_limit && i < up_limit;
				})->skip(skip_count)->limit(limit_count)->count();

			cout << "stream_skip_limit found: " << count << endl;
		};

		auto time = time_it(stream_skip_limit_test);
		cout << "stream_skip_limit time: " << time.count() << " ms" << endl;

		auto stream_unfold_skip_limit_test = [&]() {
			int count = make_stream(arr, size)
				->map([](const ST3& st)->ST2 {
					return st.st2;
				})->map([](const ST2& st)->ST1 {
					return st.st1;
				})->map([](const ST1& st)->ST0 {
					return st.st0;
				})->map([](const ST0& st)->int {
					return st.i;
				})->filter([&](const int& i)->bool {
					return i > down_limit && i < up_limit;
				})->skip(skip_count)->limit(limit_count)->count();

			cout << "stream_unfold_skip_limit found: " << count << endl;
		};

		time = time_it(stream_unfold_skip_limit_test);
		cout << "stream_unfold_skip_limit time: " << time.count() << " ms" << endl;

		auto stream_onestep_skip_limit_test = [&]() {
			int count = make_stream(arr, size)
				->filter([&](const ST3& st)->bool {
					int i = st.st2.st1.st0.i;
					return i > down_limit && i < up_limit;
				})->skip(skip_count)->limit(limit_count)->count();

			cout << "stream_onestep_skip_limit found: " << count << endl;
		};

		time = time_it(stream_onestep_skip_limit_test);
		cout << "stream_onestep_skip_limit time: " << time.count() << " ms" << endl;
	}

}