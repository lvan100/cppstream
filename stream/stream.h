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
 * ����: lvan100@yeah.net        ���֤: Apache-2.0
 *
 * C++��streamʵ�ֲο���Java8��ʵ�֣���������������
 * �Ĳ��죨��Ҫ��ģ�������Ĩ���ϣ��������ڼ̳нṹ
 * ���кܴ���죬����Ҳ�ǻ��˺ܳ�ʱ����������йؼ���
 */

namespace cpp {
	namespace stream {

		//////////////////////////////////////////////////////////
		// Lambda���ʽ�ķ���ֵ�����ƶ�

		/**
		 * ��ȡ0��������Lambda���ʽ�ķ���ֵ����
		 */
		template<typename F>
		struct ft_0 {
			typedef decltype(((F*)0)->operator()()) ret;
		};

		/**
		 * ��ȡ1��������Lambda���ʽ�ķ���ֵ����
		 */
		template<typename F, typename T>
		struct ft_1 {
			typedef decltype(((F*)0)->operator()(*((T*)0))) ret;
		};

		//////////////////////////////////////////////////////////
		// ����ʵ��

		/**
		 * ���������������ӿ�
		 */
		class ISinkChain {

		public:
			/**
			 * �������������������������ص�ǰ����������ָ�롣
			 */
			virtual ISinkChain* link(ISinkChain* down) = 0;

		};

		/**
		 * �����������ӿ�
		 */
		template<typename T> class ISink : public ISinkChain {

		public:
			/**
			 * �������ѽӿڣ�����ֵ��ʾ�Ƿ�������ѡ�
			 */
			virtual bool consum(const T& value) = 0;

		};

		/**
		 * ����������ʵ��
		 */
		template<typename T, typename D> class Sink : public ISink<T> {

		protected:
			/**
			 * ��������������ָ��
			 */
			ISink<D>* down = nullptr;

		public:
			virtual ISinkChain* link(ISinkChain* down) override {
				this->down = (ISink<D>*) down;
				return this;
			}

		};

		/**
		 * ӳ�����͵�����������
		 */
		template<typename T, typename D, typename F> class MapperSink : public Sink<T, D> {

			/**
			 * ӳ�亯����ԭ��һ��Ϊ (const T&)->D ���� (const T&)->D&&��
			 */
			F _f;

		public:
			MapperSink(F f) : _f(f)
			{}

			virtual bool consum(const T& v) override {
				D d = _f(v);
				// �����ָ����ȥ�����������ˣ���֣�
				if (down != nullptr) {
					return down->consum(d);
				}
				return true;
			}

		};

		/**
		 * ��Ծ���͵��������ѽڵ�
		 */
		template<typename T> class SkipperSink : public Sink<T, T> {

			/**
			 * ���ж���������Ҫ������
			 */
			size_t _skip = 0;

		public:
			SkipperSink(size_t skip) : _skip(skip)
			{}

			virtual bool consum(const T& v) override {
				if (_skip > 0) {
					_skip--;

				} else {
					// �����ָ����ȥ�����������ˣ���֣�
					if (down != nullptr) {
						return down->consum(v);
					}
				}
				return true;
			}

		};

		/**
		 * �������͵��������ѽڵ�
		 */
		template<typename T> class LimiterSink : public Sink<T, T> {

			/**
			 * ���ж��������ܹ�����ȡ
			 */
			size_t _limit = 0;

		public:
			LimiterSink(size_t limit) : _limit(limit)
			{}

			virtual bool consum(const T& v) override {
				if (_limit > 0) {
					_limit--;

					// �����ָ����ȥ�����������ˣ���֣�
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
		 * �������͵��������ѽڵ�
		 */
		template<typename T, typename F> class FilterSink : public Sink<T, T> {

			/**
			 * ���˺�����ԭ��һ��Ϊ (const T&)->bool��
			 */
			F _f;

		public:
			FilterSink(F f) : _f(f)
			{}

			virtual bool consum(const T& v) override {
				// �����ָ����ȥ�����������ˣ���֣�
				if (_f(v) && down != nullptr) {
					return down->consum(v);
				}
				return true;
			}

		};

		/**
		 * �������͵��������ѽڵ�
		 */
		template<typename T> class CounterSink : public Sink<T, T> {

			/**
			 *�������
			 */
			size_t count = 0;

		public:
			virtual bool consum(const T& v) override {
				count++;
				return true;
			}

			/**
			 * ��ȡ�������
			 */
			size_t get() { return count; }

		};

		/**
		 * ��Լ���͵��������ѽڵ�
		 */
		template<typename T, typename F> class ReducerSink : public Sink<T, T> {

			/**
			 * ��Լ������ԭ��һ��Ϊ (const T&, const T&)->T��
			 */
			F _f;

			/**
			 * �����Լ�����Ľ��
			 */
			T _value;

		public:
			ReducerSink(T init, F f) : _value(init), _f(f)
			{}

			virtual ISinkChain* link(ISinkChain* down) override {
				assert(false); // �����ϲ�Ӧ�õ��ô˺���
				return this;
			}

			virtual bool consum(const T& v) override {
				_value = _f(_value, v);
				return true;
			}

			/**
			 * ��ȡ��Լ�����Ľ��
			 */
			T get() { return _value; }

		};

		/**
		 * ���ҵ�һ����������������
		 */
		template<typename T> class FirstFinderSink : public Sink<T, T> {

			/**
			 * �Ƿ��Ѿ��ҵ����
			 */
			bool _found = false;

			/**
			 * ���ҵ��Ľ��
			 */
			T _value;

		public:
			FirstFinderSink(T def) : _value(def)
			{}

			virtual bool consum(const T& v) override {
				_value = v;
				_found = true;
				return false;
			}

			/**
			 * ��ȡ���ҽ��
			 */
			T& get() { return _value; }

			/**
			 * �Ƿ��Ѿ��ҵ����
			 */
			bool found() { return _found; }

		};

		/**
		 * ����Դ
		 */
		class DataSource {

		public:
			virtual ~DataSource() {}
			
			/**
			 * ��������
			 */
			virtual void consum(ISinkChain* sink) = 0;

		};

		/**
		 * ���ܵ�
		 */
		class Pipeline {

		public:
			union {

				/**
				 * ��ΪDataSourceָ�����Stream�ڵ�֮�䴫��
				 * ֱ�����սڵ㣬Ȼ���м�ڵ�ȴ������Ҫʹ��
				 * ������ǡ��Sinkָ��Ϊ�ӳٳ�ʼ���������ʼ
				 * ��֮ǰ������ʱ����DataSourceָ�룬�����
				 * ʼ��ʱ�ֿ��Դ��ݸ�����Stream������ʱ���棬
				 * ����ʹ���������ܹ�����Ч��ʹ�ô洢�ռ䡣
				 */

				 /**
				  * ����Դָ��
				  */
				DataSource* _dataSource;

				/**
				 * ����������ָ��
				 */
				ISinkChain* _sink;

			};

			/**
			 * ���ܵ������νڵ�ָ��
			 */
			Pipeline* _upstream;

			/**
			 * ���ܵ������
			 */
			size_t _depth = 0;

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
		 * �Զ��ͷ����ܵ���Դ
		 */
		class AutoReleasePipeline {

			Pipeline* _pipeline = nullptr;

		public:
			AutoReleasePipeline(Pipeline* pipeline)
				: _pipeline(pipeline)
			{}

			~AutoReleasePipeline() {
				if (_pipeline != nullptr) {
					delete _pipeline;
				}
			}
		};

		/**
		 * ��
		 */
		template<typename T> class Stream : public Pipeline {

		public:
			Stream(Pipeline* up, DataSource* ds)
				: Pipeline(up, ds)
			{}

			/**
			 * �洢Sinkָ�룬������ԭ����DataSourceָ�롣
			 */
			DataSource* storeSink(ISinkChain* sink) {
				DataSource* ds = _dataSource;
				this->_sink = sink;
				return ds;
			}

			/**
			 * ����ǰ��ת��Ϊ������
			 */
			template<typename F> auto map(F f) {
				using D = typename ft_1<F, T>::ret;
				ISinkChain* sink = new MapperSink<T, D, F>(f);
				DataSource* ds = storeSink(sink);
				return new Stream<D>(this, ds);
			}

			/**
			 * �Ե�ǰ�����й�Լ����
			 */
			template<typename F> T reduce(T init, F f) {

				ReducerSink<T, F>* sink = new ReducerSink<T, F>(init, f);
				ConsumeData(sink);

				AutoReleasePipeline arp(this);
				return sink->get();
			}

			/**
			 * �����е�Ԫ�ؽ��й���
			 */
			template<typename F> Stream* filter(F f) {
				ISinkChain* sink = new FilterSink<T, F>(f);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * ����һЩ��ȷ���
			 */
			Stream* skip(size_t nSkip) {
				ISinkChain* sink = new SkipperSink<T>(nSkip);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * ������ȷ���������
			 */
			Stream* limit(size_t nLimit) {
				ISinkChain* sink = new LimiterSink<T>(nLimit);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * �������
			 */
			Stream* reverse() {				
				return this;
			}

			/**
			 * �����ڵĶ�����м���
			 */
			size_t count() {
				return map([](const T& t)->size_t {
					return 1;
				})->reduce(0, [](const size_t& i, const size_t& v)->size_t {
					return i + v;
				});
			}

			/**
			 * �����ڵĶ�����м���(���ٰ汾)
			 */
			size_t quick_count() {

				CounterSink<T>* sink = new CounterSink<T>();
				ConsumeData(sink);

				AutoReleasePipeline arp(this);
				return sink->get();
			}

			/**
			 * �ҵ���һ����������������
			 */
			T findFirst(T def) {

				FirstFinderSink<T>* sink = new FirstFinderSink<T>(def);
				ConsumeData(sink);

				AutoReleasePipeline arp(this);
				return sink->get();
			}

		private:
			/**
			 * ������������������
			 */
			ISinkChain* BuildSinkChain() {

				ISinkChain* ps = this->_sink;
				Pipeline* pu = this->_upstream;

				for (size_t i = this->_depth; i > 0; i--) {
					ps = pu->_sink->link(ps);
					pu = pu->_upstream;
				}

				return ps;
			}

			/**
			 * ��ʼ��������
			 */
			void ConsumeData(ISinkChain* sink) {

				DataSource* dataSource = storeSink(sink);
				assert(dataSource != nullptr);

				ISinkChain* ps = BuildSinkChain();

				if (dataSource != nullptr) {
					dataSource->consum(ps);
				}

				if (dataSource != nullptr) {
					delete dataSource;
				}
			}

		};

		//////////////////////////////////////////////////////////
		// ����Դ��ʵ��

		/**
		 * ����Դ�Ƿ������������ʹ��bool��Ϊ������ģ�壬��������
		 */
		enum Reverse {
			False /* ���� */,
			True  /* ���� */
		};

		/**
		 * �������͵�����Դ
		 */
		template<typename T> class ArrayDataSource : public DataSource {

			/**
			 * �Ƿ��������
			 */
			Reverse _reverse = Reverse::False;

			/**
			 * ����Ԫ�ص�ָ��
			 */
			T* _array = nullptr;

			/**
			 * ����������յ�
			 */
			ptrdiff_t _end = 0;

			/**
			 * ������������
			 */
			ptrdiff_t _start = 0;

		public:
			ArrayDataSource(T* arr, ptrdiff_t end, Reverse reverse)
				: _array(arr), _start(0), _end(end), _reverse(reverse)
			{}

			ArrayDataSource(T* arr, ptrdiff_t start, ptrdiff_t end, Reverse reverse)
				: _array(arr), _start(start), _end(end), _reverse(reverse)
			{}

			virtual void consum(ISinkChain* sink) override {
				ISink<T>* tsink = (ISink<T>*) sink;
				if (_reverse == Reverse::True) {
					for (ptrdiff_t i = _end - 1; i >= _start; --i) {
						if (!tsink->consum(_array[i])) {
							break;
						}
					}
				} else {
					for (ptrdiff_t i = _start; i < _end; ++i) {
						if (!tsink->consum(_array[i])) {
							break;
						}
					}
				}
			}

			/**
			 * ����������
			 */
			Stream<T>* stream() {
				return new Stream<T>(nullptr, this);
			}

		};

		/**
		 * ��C�������͵������캯��
		 */
		template<typename T, size_t size>
		auto make_stream(T(&arr)[size], Reverse reverse = Reverse::False) {
			return (new ArrayDataSource<T>(arr, size, reverse))->stream();
		}

		/**
		 * ָ���������͵������캯��
		 */
		template<typename T>
		auto make_stream(T* arr, ptrdiff_t end, Reverse reverse = Reverse::False) {
			return (new ArrayDataSource<T>(arr, end, reverse))->stream();
		}

		/**
		 * ָ���������͵������캯��
		 */
		template<typename T>
		auto make_stream(T* arr, ptrdiff_t start, ptrdiff_t end, Reverse reverse = Reverse::False) {
			return (new ArrayDataSource<T>(arr, start, end, reverse))->stream();
		}

		/**
		 * ���������͵�����Դ
		 */
		template<typename T, typename Iterator>
		class IteratorDataSource : public DataSource {

			/**
			 * �����������
			 */
			Iterator _begin;

			/**
			 * ���������յ�
			 */
			Iterator _end;

		public:
			IteratorDataSource(Iterator begin, Iterator end)
				: _begin(begin), _end(end)
			{}

			virtual void consum(ISinkChain* sink) override {
				ISink<T>* tsink = (ISink<T>*)sink;
				for (Iterator iter = _begin; iter != _end; iter++) {
					if (!tsink->consum(*iter)) {
						break;
					}
				}
			}

			/**
			* ����������
			*/
			Stream<T>* stream() {
				return new Stream<T>(nullptr, this);
			}

		};

		/**
		 * ���������͵������캯��
		 */
		template<typename Iterator>
		auto make_stream(Iterator start, Iterator end) {
			return (new IteratorDataSource<Iterator::value_type, Iterator>(start, end))->stream();
		}

		/**
		 * STL��׼�����������캯��
		 */
		template<typename Container>
		auto make_stream(Container& container, Reverse reverse = Reverse::False) {
			if (reverse == Reverse::False) {
				return (new IteratorDataSource<Container::value_type, Container::iterator>(container.begin(), container.end()))->stream();
			}
			return (new IteratorDataSource<Container::value_type, Container::reverse_iterator>(container.rbegin(), container.rend()))->stream();
		}

		/**
		 * �ظ�ֵ���͵�����Դ
		 */
		template<typename T>
		class RepeatDataSource : public DataSource {

			/**
			 * �ظ���ֵ
			 */
			T _value;

			/**
			 * �ظ�����
			 */
			size_t _count;

		public:
			RepeatDataSource(T value, size_t count)
				: _value(value), _count(count)
			{}

			virtual void consum(ISinkChain* sink) override {
				ISink<T>* tsink = (ISink<T>*)sink;
				for (size_t i = 0U; i < _count; i++) {
					if (!tsink->consum(_value)) {
						break;
					}
				}
			}

			/**
			* ����������
			*/
			Stream<T>* stream() {
				return new Stream<T>(nullptr, this);
			}

		};

		/**
		 * �ظ�ֵ���͵������캯��
		 */
		template<typename T>
		auto make_repeat_stream(T value, size_t count) {
			return (new RepeatDataSource<T>(value, count))->stream();
		}

		//////////////////////////////////////////////////////////
		// >>��������ʵ��

		/**
		 * ʵ������>>������
		 */
		template<typename T, typename Sink>
		auto operator >> (Stream<T>* s, Sink sink) {
			return sink(s);
		}

		/**
		 * ����ӳ����������͵���
		 */
		template<typename F>
		auto map(F f) {
			return [f](auto* s) {
				return s->map(f);
			};
		};

		/**
		 * �������е�ĳЩԪ��
		 */
		template<typename F>
		auto filter(F f) {
			return [f](auto* s) {
				return s->filter(f);
			};
		}

		/**
		 * �������е�ĳЩ���
		 */
		auto skip(size_t nSkip) {
			return [nSkip](auto* s) {
				return s->skip(nSkip);
			};
		}

		/**
		 * ��������Ԫ�ص�����
		 */
		auto limit(size_t nLimit) {
			return [nLimit](auto* s) {
				return s->limit(nLimit);
			};
		}

		/**
		 * ��������Ԫ�ص�����
		 */
		auto count() {
			return [](auto* s) {
				return s->count();
			};
		}

		/**
		 * ��������Ԫ�ص�����(���ٰ汾)
		 */
		auto quick_count() {
			return [](auto* s) {
				return s->quick_count();
			};
		}

		/**
		 * �ҵ���һ����������������
		 */
		template<typename F, typename T>
		auto findFirst(F f, T def) {
			return [&f, &def](auto* s) {
				return s->findFirst(f, def);
			};
		}

	}
}

#ifdef _MEMDUMP
#undef new
#undef DUMP_NEW
#endif

#endif /* CPP_STREAM_H */