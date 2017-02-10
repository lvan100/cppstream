#pragma once

#include <time.h>

#include "typedefs.h"

#include <list>
#include <deque>
#include <vector>
#include <iostream>
using namespace std;

#include <chrono>
using namespace std::chrono;

#include "stream.h"
using namespace cpp::stream;

#include "new.h"

#ifdef _DEBUG
#define DEBUG_NEW new(__FILE__, __FUNCTION__ ,__LINE__)
#define new DEBUG_NEW
#endif

/**
 * 计算函数执行3次的平均时间
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
 * 执行性能测试
 */
void run_performance_test(int size, int skip_count, int limit_count) {

	srand((unsigned int)time(nullptr));

	ST3* arr = new ST3[size];
	for (int i = 0; i < size;i++) {
		arr[i].set(rand() % 5000);
	}

#if 0
	#define up_limit	4000
	#define down_limit  2600
#else
	const int up_limit = 4000;
	const int down_limit = 2600;
#endif
	
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

		auto stream_onestep_quick_count_test = [&]() {
			int count = make_stream(arr, size)
				->filter([&](const ST3& st)->bool {
					int i = st.st2.st1.st0.i;
					return i > down_limit && i < up_limit;
				})->quick_count();

			cout << "stream_onestep_quick_count found: " << count << endl;
		};

		time = time_it(stream_onestep_quick_count_test);
		cout << "stream_onestep_quick_count time: " << time.count() << " ms" << endl;
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

		auto stream_onestep_skip_quick_count_test = [&]() {
			int count = make_stream(arr, size)
				->filter([&](const ST3& st)->bool {
					int i = st.st2.st1.st0.i;
					return i > down_limit && i < up_limit;
				})->skip(skip_count)->quick_count();

			cout << "stream_onestep_skip_quick_count found: " << count << endl;
		};

		time = time_it(stream_onestep_skip_quick_count_test);
		cout << "stream_onestep_skip_quick_count time: " << time.count() << " ms" << endl;
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

		auto stream_onestep_limit_quick_count_test = [&]() {
			int count = make_stream(arr, size)
				->filter([&](const ST3& st)->bool {
					int i = st.st2.st1.st0.i;
					return i > down_limit && i < up_limit;
				})->limit(limit_count)->quick_count();

			cout << "stream_onestep_limit_quick_count found: " << count << endl;
		};

		time = time_it(stream_onestep_limit_quick_count_test);
		cout << "stream_onestep_limit_quick_count time: " << time.count() << " ms" << endl;
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

		auto stream_onestep_skip_limit_quick_count_test = [&]() {
			int count = make_stream(arr, size)
				->filter([&](const ST3& st)->bool {
					int i = st.st2.st1.st0.i;
					return i > down_limit && i < up_limit;
				})->skip(skip_count)->limit(limit_count)->quick_count();

			cout << "stream_onestep_skip_limit_quick_count found: " << count << endl;
		};

		time = time_it(stream_onestep_skip_limit_quick_count_test);
		cout << "stream_onestep_skip_limit_quick_count time: " << time.count() << " ms" << endl;
	}

	cout << endl;

	delete[] arr;
}

void run_performance_test(int size) {
	run_performance_test(size, 5000, 5000);
	run_performance_test(size, 50000, 50000);
	run_performance_test(size, 500000, 500000);
	run_performance_test(size, 5000000, 5000000);
}

void array_stream_test() {

	int arr[] = { 0,1,2,3,4,5,6,7,8,9 };

	int count = make_stream(arr)->quick_count();
	assert(count == 10);

	count = make_stream(arr)->reverse()->quick_count();
	assert(count == 10);

	count = make_stream(arr, 10)->quick_count();
	assert(count == 10);

	count = make_stream(arr, 10)->reverse()->quick_count();
	assert(count == 10);

	count = make_stream(arr, 3)->quick_count();
	assert(count == 3);

	count = make_stream(arr, 3)->reverse()->quick_count();
	assert(count == 3);

	count = make_stream(arr, 0, 5)->quick_count();
	assert(count == 5);

	count = make_stream(arr, 0, 5)->reverse()->quick_count();
	assert(count == 5);

	count = make_stream(arr, 3, 5)->quick_count();
	assert(count == 2);

	count = make_stream(arr, 3, 5)->reverse()->quick_count();
	assert(count == 2);
}

void iterator_stream_test() {

	{
		vector<int> vi = { 0,1,2,3,4,5,6,7,8,9 };

		int count = make_stream(vi)->quick_count();
		assert(count == 10);

		count = make_stream(vi, Reverse::True)->quick_count();
		assert(count == 10);

		count = make_stream(vi.begin(), vi.end())->quick_count();
		assert(count == 10);

		count = make_stream(vi.rbegin(), vi.rend())->quick_count();
		assert(count == 10);

		count = make_stream(vi.begin(), vi.begin() + 5)->quick_count();
		assert(count == 5);

		count = make_stream(vi.rbegin(), vi.rbegin() + 5)->quick_count();
		assert(count == 5);

		count = make_stream(vi.begin() + 3, vi.end())->quick_count();
		assert(count == 7);

		count = make_stream(vi.rbegin() + 3, vi.rend())->quick_count();
		assert(count == 7);
	}

	{
		list<int> li = { 0,1,2,3,4,5,6,7,8,9 };

		int count = make_stream(li)->quick_count();
		assert(count == 10);

		count = make_stream(li, Reverse::True)->quick_count();
		assert(count == 10);

		count = make_stream(li.begin(), li.end())->quick_count();
		assert(count == 10);

		count = make_stream(li.rbegin(), li.rend())->quick_count();
		assert(count == 10);

		auto li_iter_6 = li.begin();
		auto li_riter_6 = li.rbegin();
		for (int i = 0; i < 6;i++) {
			li_iter_6++;
			li_riter_6++;
		}

		count = make_stream(li.begin(), li_iter_6)->quick_count();
		assert(count == 6);

		count = make_stream(li.rbegin(), li_riter_6)->quick_count();
		assert(count == 6);

		count = make_stream(li_iter_6, li.end())->quick_count();
		assert(count == 4);

		count = make_stream(li_riter_6, li.rend())->quick_count();
		assert(count == 4);
	}

	{
		deque<int> di = { 0,1,2,3,4,5,6,7,8,9 };

		int count = make_stream(di)->quick_count();
		assert(count == 10);

		count = make_stream(di, Reverse::True)->quick_count();
		assert(count == 10);

		count = make_stream(di.begin(), di.end())->quick_count();
		assert(count == 10);

		count = make_stream(di.rbegin(), di.rend())->quick_count();
		assert(count == 10);

		count = make_stream(di.begin(), di.begin() + 4)->quick_count();
		assert(count == 4);

		count = make_stream(di.rbegin(), di.rbegin() + 4)->quick_count();
		assert(count == 4);

		count = make_stream(di.begin() + 4, di.end())->quick_count();
		assert(count == 6);

		count = make_stream(di.rbegin() + 4, di.rend())->quick_count();
		assert(count == 6);
	}

}

void repeat_stream_test() {

	int count = make_repeat_stream(1, 10)->quick_count();
	assert(count == 10);

	count = make_repeat_stream(2, 6)->quick_count();
	assert(count == 6);
}

void custom_stream_test() {

	struct Node {

		int value = 0;
		Node* next = nullptr;

	public:
		Node(int v) {
			value = v;
		}

	};

	Node* head = new Node(0);

	Node* curr = head;
	for (int i = 1; i < 10; i++) {
		curr->next = new Node(i);
		curr = curr->next;
	}

	curr = head;
	while (curr != nullptr) {
		cout << curr->value;
		curr = curr->next;
	}

	cout << endl;

	class NodePtrDataSource : public DataSource {

		Node* _start;

	public:
		NodePtrDataSource(Node* start)
			: _start(start)
		{}

		virtual void consum(ISinkChain* sink) override {
			ISink<int>* tsink = (ISink<int>*)sink;
			while (_start != nullptr) {
				cout << _start->value;
				_start = _start->next;
			}
		}

		Stream<int>* stream() {
			return new Stream<int>(nullptr, this);
		}

	};

	(new NodePtrDataSource(head))->stream()->count();
	cout << endl;

	(new NodePtrDataSource(head->next->next))->stream()->count();
	cout << endl;

	curr = head;
	Node* last = nullptr;
	while (curr != nullptr) {
		last = curr;
		curr = curr->next;
		delete last;
	}

	cout << endl;
}

void run_function_test() {

	using Tester = function<void()>;
	Tester testers[] = {
		array_stream_test,
		repeat_stream_test,
		custom_stream_test,
		iterator_stream_test,
	};

	for (Tester tester : testers) {
		tester();
	}
}

#ifdef _DEBUG
#undef new
#undef DEBUG_NEW
#endif