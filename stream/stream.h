#pragma once

#include "new.h"

#ifdef _DEBUG
#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "assert.h"

#include <functional>
using namespace std;

/**
 * ���������������ӿ�
 */
class ISinkChain {

public:
	/*
	 * �������������������������ص�ǰ����������
	 */
	virtual ISinkChain* link(ISinkChain* down) = 0;

};

/**
 * �����������ӿ�
 */
template<typename T> class Sink : public ISinkChain {

public:
	/**
	 * �������ѽӿڣ������Ƿ�Ӧ�ü�������
	 */
	virtual bool consum(const T& value) = 0;

};

/**
 * ������������������ʵ��
 */
template<typename T, typename D> class SinkChain : public Sink<T> {

protected:
	/**
	 * ��������������ָ��
	 */
	Sink<D>* down = nullptr;

public:
	virtual ISinkChain* link(ISinkChain* down) override {
		this->down = (Sink<D>*)down;
		return this;
	}

};

/**
 * ת�����͵�����������
 */
template<typename T, typename D> class ConvertSink : public SinkChain<T, D> {

	/**
	 * ��ǰ�ڵ�����Ѻ���
	 */
	using Consum = function<D(const T&)>;
	Consum _f;

public:
	ConvertSink(Consum f) : _f(f)
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
 * ��Ծ�������ѽڵ�
 */
template<typename T> class SkipSink : public SinkChain<T, T> {

	/**
	 * ���ж���������Ҫ������
	 */
	int _skip = 0;

public:
	SkipSink(int skip) : _skip(skip)
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
 * �����������ѽڵ�
 */
template<typename T> class LimitSink : public SinkChain<T, T> {

	/**
	 * ���ж��������ܹ�����ȡ
	 */
	int _limit = 0;

public:
	LimitSink(int limit) : _limit(limit)
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
 * �����������ѽڵ�
 */
template<typename T> class FilterSink : public SinkChain<T, T> {
	
	/**
	 * ��ǰ�ڵ�����Ѻ���
	 */
	using Consum = function<bool(const T&)>;
	Consum _f;

public:
	FilterSink(Consum f) : _f(f)
	{}
	
	virtual bool consum(const T& v) override {
		bool b = _f(v);
		if (b && down != nullptr) {
			return down->consum(v);
		}
		return true;
	}

};

/**
 * ��Լ�������ѽڵ�
 */
template<typename T> class ReduceSink : public SinkChain<T, T> {

	/**
	 * ��ǰ�ڵ�����Ѻ���
	 */
	using Consum = function<T(const T&, const T&)>;
	Consum _f;

	/**
	 * �����ն˴�����
	 */
	T _value;

public:
	ReduceSink(T init, Consum f) : _value(init), _f(f)
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
	 * ��ȡ�ն˴�����
	 */
	T get() {
		return _value;
	}

};

/**
 * ���ݷ�����
 */
class Splitter {

public:
	/**
	 * ��ʼ��������
	 */
	virtual void consum(ISinkChain* sink) = 0;

};

/**
 * ������
 */
class StreamBase {

public:
	/**
	 * �������
	 */
	int _depth = 0;

	/**
	 * ��������������
	 */
	ISinkChain* _sink = nullptr;

	/**
	 * ���ݷ�����ָ��
	 */
	Splitter* _splitter = nullptr;

	/**
	 * �������νڵ�ָ��
	 */
	StreamBase* _upstream = nullptr;

public:
	StreamBase(StreamBase* up, Splitter* sp)
		: _upstream(up), _splitter(sp) {
		if (up != nullptr) {
			_depth = up->_depth + 1;
		}
	}

	/* virtual */ ~StreamBase() {

		if (_sink != nullptr) {
			delete _sink;
			_sink = nullptr;
		}

		if (_upstream != nullptr) {
			delete _upstream;
			_upstream = nullptr;
		}
	}

	/**
	 * �����ڵĶ�����м���
	 */
	virtual int count() = 0;

};

/**
 * ��
 */
template<typename T> class Stream : public StreamBase {

public:
	Stream(StreamBase* up, Splitter* sp)
		: StreamBase(up, sp)
	{}

	/**
	 * ����ǰ��ת��Ϊ������
	 */
	template<typename D> Stream<D>* map(function<D(const T&)> f) {
		this->_sink = new ConvertSink<T, D>(f);
		return new Stream<D>(this, _splitter);
	}

	/**
	 * �Ե�ǰ�����й�Լ����
	 */
	T reduce(T init, function<T(const T&, const T&)> f) {

		ReduceSink<T>* sink = new ReduceSink<T>(init, f);
		this->_sink = sink;

		ISinkChain* ps = this->_sink;
		StreamBase* pu = this->_upstream;

		for (int i = this->_depth; i > 0; i--) {
			ps = pu->_sink->link(ps);
			pu = pu->_upstream;
		}

		assert(_splitter != nullptr);

		if (_splitter != nullptr) {
			_splitter->consum(ps);
		}

		T result = sink->get();

		// ���������սڵ㿪ʼ�ͷ�
		delete this;

		return result;
	}

	/**
	 * �����е�Ԫ�ؽ��й���
	 */
	Stream* filter(function<bool(const T&)> f) {
		this->_sink = new FilterSink<T>(f);
		return new Stream(this, _splitter);
	}

	/**
	 * ����ĳЩ��ȷ���
	 */
	Stream* skip(int nSkip) {
		this->_sink = new SkipSink<T>(nSkip);
		return new Stream(this, _splitter);
	}

	/**
	 * ������ȷ���������
	 */
	Stream* limit(int nLimit) {
		this->_sink = new LimitSink<T>(nLimit);
		return new Stream(this, _splitter);
	}

	/**
	 * �����ڵĶ�����м���
	 */
	virtual int count() override {
		return map<int>([](const T& t)->int {
			return 1;
		})->reduce(0, [](const int& i, const int& v)->int {
			return i + v;
		});
	}

};

/**
 * �������͵����ݷ�����
 */
template<typename T> class ArraySplitter : public Splitter {

	/**
	 * �����Ԫ������
	 */
	int _size = 0;

	/**
	 * ����Ԫ�ص�ָ��
	 */
	T* _array = nullptr;
	
public:
	ArraySplitter(T* arr, int size)
		: _array(arr), _size(size)
	{}

	virtual void consum(ISinkChain* sink) override {
		Sink<T>* tsink = (Sink<T>*)sink;
		for (int i = 0; i < _size; i++) {
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