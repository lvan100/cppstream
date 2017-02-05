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
		template<typename T, typename D> class MapperSink : public Sink<T, D> {

			/**
			 * ӳ�亯��
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
		 * ��Ծ���͵��������ѽڵ�
		 */
		template<typename T> class SkipperSink : public Sink<T, T> {

			/**
			 * ���ж���������Ҫ������
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
		 * �������͵��������ѽڵ�
		 */
		template<typename T> class LimiterSink : public Sink<T, T> {

			/**
			 * ���ж��������ܹ�����ȡ
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
		 * �������͵��������ѽڵ�
		 */
		template<typename T> class FilterSink : public Sink<T, T> {

			/**
			 * ���˺���
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
		 * ��Լ���͵��������ѽڵ�
		 */
		template<typename T> class ReducerSink : public Sink<T, T> {

			/**
			 * ��Լ����
			 */
			using Reducer = function<T(const T&, const T&)>;
			Reducer _f;

			/**
			 * �����Լ�����Ľ��
			 */
			T _value;

		public:
			ReducerSink(T init, Reducer f) : _value(init), _f(f)
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
		 * ����Դ
		 */
		class DataSource {

		public:
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
				function<typename ft_1<F, T>::ret(const T&)> fmap = f;
				return map(fmap); // Stream<typename ft_1<F, T>::ret>*
			}

			/**
			 * ����ǰ��ת��Ϊ������
			 */
			template<typename D> Stream<D>* map(function<D(const T&)> f) {
				ISinkChain* sink = new MapperSink<T, D>(f);
				DataSource* ds = storeSink(sink);
				return new Stream<D>(this, ds);
			}

			/**
			 * �Ե�ǰ�����й�Լ����
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

				// �ͷż��������ʹ�õ�ָ����Դ
				{
					if (dataSource != nullptr) {
						delete dataSource;
					}

					delete this;
				}
				
				return result;
			}

			/**
			 * �����е�Ԫ�ؽ��й���
			 */
			Stream* filter(function<bool(const T&)> f) {
				ISinkChain* sink = new FilterSink<T>(f);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * ����һЩ��ȷ���
			 */
			Stream* skip(int nSkip) {
				ISinkChain* sink = new SkipperSink<T>(nSkip);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * ������ȷ���������
			 */
			Stream* limit(int nLimit) {
				ISinkChain* sink = new LimiterSink<T>(nLimit);
				DataSource* ds = storeSink(sink);
				return new Stream<T>(this, ds);
			}

			/**
			 * �����ڵĶ�����м���
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
		// ����Դ��ʵ��

		/**
		 * �������͵�����Դ
		 */
		template<typename T> class ArrayDataSource : public DataSource {

			/**
			 * ����������յ�
			 */
			int _end = 0;

			/**
			 * ������������
			 */
			int _start = 0;

			/**
			 * ����Ԫ�ص�ָ��
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
			 * ����������
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
		auto skip(int nSkip) {
			return [nSkip](auto* s) {
				return s->skip(nSkip);
			};
		}

		/**
		 * ��������Ԫ�ص�����
		 */
		auto limit(int nLimit) {
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

	}
}

#ifdef _MEMDUMP
#undef new
#undef DUMP_NEW
#endif

#endif /* CPP_STREAM_H */