#ifndef CPP_STREAM_H
#define CPP_STREAM_H

#ifdef _MEMDUMP
#include "new.h"
#define DUMP_NEW new(__FILE__, __FUNCTION__ ,__LINE__)
#define new DUMP_NEW
#endif

#include "assert.h"

#include <functional>
using namespace std;

/**
 * 作者: lvan100@yeah.net        许可证: Apache-2.0
 *
 * C++的stream实现参考了Java8的实现，但是由于语言上
 * 的差异（主要在模板的类型抹除上），两者在继承结构
 * 上有很大差异，作者也是花了很长时间才明白其中关键。
 */

namespace cpp {
	namespace stream {

		//////////////////////////////////////////////////////////
		// Lambda表达式的返回值类型推断

		/**
		 * 获取0个参数的Lambda表达式的返回值类型
		 */
		template<typename F>
		struct ft_0 {
			typedef decltype(((F*)0)->operator()()) ret;
		};

		/**
		 * 获取1个参数的Lambda表达式的返回值类型
		 */
		template<typename F, typename T>
		struct ft_1 {
			typedef decltype(((F*)0)->operator()(*((T*)0))) ret;
		};

		//////////////////////////////////////////////////////////
		// 流的实现

		/**
		 * 数据消费器链条接口
		 */
		class ISinkChain {

		public:
			/**
			 * 连接下游数据消费器，并返回当前数据消费器指针。
			 */
			virtual ISinkChain* link(ISinkChain* down) = 0;

		};

		/**
		 * 数据消费器接口
		 */
		template<typename T> class ISink : public ISinkChain {

		public:
			/**
			 * 数据消费接口，返回值表示是否继续消费。
			 */
			virtual bool consum(const T& value) = 0;

		};

		/**
		 * 数据消费器实现
		 */
		template<typename T, typename D> class Sink : public ISink<T> {

		protected:
			/**
			 * 下游数据消费器指针
			 */
			ISink<D>* down = nullptr;

		public:
			virtual ISinkChain* link(ISinkChain* down) override {
				this->down = (ISink<D>*) down;
				return this;
			}

		};

		/**
		 * 映射类型的数据消费器
		 */
		template<typename T, typename D> class MapperSink : public Sink<T, D> {

			/**
			 * 映射函数
			 */
			using Mapper = function<D(const T&)>;
			Mapper _f;

		public:
			MapperSink(Mapper f) : _f(f)
			{}

			virtual bool consum(const T& v) override {
				D d = _f(v);
				if (down != nullptr) {
					return down->consum(d);
				}
				return true;
			}

		};

		/**
		 * 跳跃类型的数据消费节点
		 */
		template<typename T> class SkipperSink : public Sink<T, T> {

			/**
			 * 还有多少数据需要被跳过
			 */
			int _skip = 0;

		public:
			SkipperSink(int skip) : _skip(skip)
			{}

			virtual bool consum(const T& v) override {
				if (--_skip < 0) {
					if (down != nullptr) {
						return down->consum(v);
					}
				}
				return true;
			}

		};

		/**
		 * 限制类型的数据消费节点
		 */
		template<typename T> class LimiterSink : public Sink<T, T> {

			/**
			 * 还有多少数据能够被获取
			 */
			int _limit = 0;

		public:
			LimiterSink(int limit) : _limit(limit)
			{}

			virtual bool consum(const T& v) override {
				if (_limit-- > 0) {
					if (down != nullptr) {
						return down->consum(v);
					}
					return true;
				} else {
					return false;
				}
			}

		};

		/**
		 * 过滤类型的数据消费节点
		 */
		template<typename T> class FilterSink : public Sink<T, T> {

			/**
			 * 过滤函数
			 */
			using Filter = function<bool(const T&)>;
			Filter _f;

		public:
			FilterSink(Filter f) : _f(f)
			{}

			virtual bool consum(const T& v) override {
				if (_f(v) && down != nullptr) {
					return down->consum(v);
				}
				return true;
			}

		};

		/**
		 * 规约类型的数据消费节点
		 */
		template<typename T> class ReducerSink : public Sink<T, T> {

			/**
			 * 规约函数
			 */
			using Reducer = function<T(const T&, const T&)>;
			Reducer _f;

			/**
			 * 保存规约处理后的结果
			 */
			T _value;

		public:
			ReducerSink(T init, Reducer f) : _value(init), _f(f)
			{}

			virtual ISinkChain* link(ISinkChain* down) override {
				assert(false); // 理论上不应该调用此函数
				return this;
			}

			virtual bool consum(const T& v) override {
				_value = _f(_value, v);
				return true;
			}

			/**
			 * 获取规约处理后的结果
			 */
			T get() { return _value; }

		};

		/**
		 * 数据源
		 */
		class DataSource {

		public:
			/**
			 * 消费数据
			 */
			virtual void consum(ISinkChain* sink) = 0;

		};

		/**
		 * 流管道
		 */
		class Pipeline {

		public:
			union {

				/**
				 * 因为DataSource指针会在Stream节点之间传递
				 * 直到最终节点，然而中间节点却并不需要使用
				 * 它，又恰好Sink指针为延迟初始化，在其初始
				 * 化之前可以暂时保存DataSource指针，在其初
				 * 始化时又可以传递给下游Stream进行暂时保存，
				 * 所以使用联合体能够更有效地使用存储空间。
				 */

				 /**
				  * 数据源指针
				  */
				DataSource* _dataSource;

				/**
				 * 数据消费器指针
				 */
				ISinkChain* _sink;

			};

			/**
			 * 流管道的上游节点指针
			 */
			Pipeline* _upstream;

			/**
			 * 流管道的深度
			 */
			int _depth = 0;

		public:
			Pipeline(Pipeline* up, DataSource* ds)
				: _upstream(up), _dataSource(ds) {
				if (up != nullptr) {
					_depth = up->_depth + 1;
				}
			}

			/* virtual */ ~Pipeline() {

				if (_sink != nullptr) {
					delete _sink;
					_sink = nullptr;
				}

				if (_upstream != nullptr) {
					delete _upstream;
					_upstream = nullptr;
				}
			}

		};

		/**
		 * 流
		 */
		template<typename T> class Stream : public Pipeline {

		public:
			Stream(Pipeline* up, DataSource* ds)
				: Pipeline(up, ds)
			{}

			/**
			 * 存储Sink指针，并返回原来的DataSource指针。
			 */
			DataSource* storeSink(ISinkChain* sink) {
				DataSource* ds = _dataSource;
				this->_sink = sink;
				return ds;
			}

			/**
			 * 将当前流转换为其他流
			 */
			template<typename F> auto map(F f) {
				function<typename ft_1<F, T>::ret(const T&)> fmap = f;
				return map(fmap); // Stream<typename ft_1<F, T>::ret>*
			}

			/**
			 * 将当前流转换为其他流
			 */
			template<typename D> Stream<D>* map(function<D(const T&)> f) {
				ISinkChain* sink = new MapperSink<T, D>(f);
				DataSource* ds = storeSink(sink);
				return new Stream<D>(this, ds);
			}

			/**
			 * 对当前流进行归约操作
			 */
			T reduce(T init, function<T(const T&, const T&)> f) {

				ReducerSink<T>* sink = new ReducerSink<T>(init, f);
				DataSource* dataSource = storeSink(sink);
				assert(dataSource != nullptr);

				ISinkChain* ps = this->_sink;
				Pipeline* pu = this->_upstream;

				for (int i = this->_depth; i > 0; i--) {
					ps = pu->_sink->link(ps);
					pu = pu->_upstream;
				}

				if (dataSource != nullptr) {
					dataSource->consum(ps);
				}

				T result = sink->get();

				// 释放计算过程中使用的指针资源
				{
					if (dataSource != nullptr) {
						delete dataSource;
					}

					delete this;
				}
				
				return result;
			}

			/**
			 * 对流中的元素进行过滤
			 */
			Stream* filter(function<bool(const T&)> f) {
				ISinkChain* sink = new FilterSink<T>(f);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * 跳过一些正确结果
			 */
			Stream* skip(int nSkip) {
				ISinkChain* sink = new SkipperSink<T>(nSkip);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * 限制正确结果的数量
			 */
			Stream* limit(int nLimit) {
				ISinkChain* sink = new LimiterSink<T>(nLimit);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * 对流内的对象进行计数
			 */
			int count() {
				return map([](const T& t)->int {
					return 1;
				})->reduce(0, [](const int& i, const int& v)->int {
					return i + v;
				});
			}

		};

		//////////////////////////////////////////////////////////
		// 数据源的实现

		/**
		 * 数组类型的数据源
		 */
		template<typename T> class ArrayDataSource : public DataSource {

			/**
			 * 数组遍历的终点
			 */
			int _end = 0;

			/**
			 * 数组遍历的起点
			 */
			int _start = 0;

			/**
			 * 数组元素的指针
			 */
			T* _array = nullptr;

		public:
			ArrayDataSource(T* arr, int size)
				: _array(arr), _end(size)
			{}

			virtual void consum(ISinkChain* sink) override {
				ISink<T>* tsink = (ISink<T>*)sink;
				for (int i = _start; i < _end; i++) {
					if (!tsink->consum(_array[i])) {
						break;
					}
				}
			}

			/**
			 * 构造流对象
			 */
			Stream<T>* stream() {
				return new Stream<T>(nullptr, this);
			}

		};

		template<typename T>
		Stream<T>* make_stream(T* arr, int size) {
			return (new ArrayDataSource<T>(arr, size))->stream();
		}

		//////////////////////////////////////////////////////////
		// >>操作符的实现

		/**
		 * 实现流的>>操作符
		 */
		template<typename T, typename Sink>
		auto operator >> (Stream<T>* s, Sink sink) {
			return sink(s);
		}

		/**
		 * 将流映射成其他类型的流
		 */
		template<typename F>
		auto map(F f) {
			return [f](auto* s) {
				return s->map(f);
			};
		};

		/**
		 * 过滤流中的某些元素
		 */
		template<typename F>
		auto filter(F f) {
			return [f](auto* s) {
				return s->filter(f);
			};
		}

		/**
		 * 跳过流中的某些结果
		 */
		auto skip(int nSkip) {
			return [nSkip](auto* s) {
				return s->skip(nSkip);
			};
		}

		/**
		 * 限制流中元素的数量
		 */
		auto limit(int nLimit) {
			return [nLimit](auto* s) {
				return s->limit(nLimit);
			};
		}

		/**
		 * 计算流中元素的数量
		 */
		auto count() {
			return [](auto* s) {
				return s->count();
			};
		}

	}
}

#ifdef _MEMDUMP
#undef new
#undef DUMP_NEW
#endif

#endif /* CPP_STREAM_H */