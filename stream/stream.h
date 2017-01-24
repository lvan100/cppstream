#pragma once

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
	 * �������ѽӿ�
	 */
	virtual void consum(const T& value) = 0;

};

/**
 * ����������������һ��ڵ����
 */
template<typename T, typename D> class SinkChain : public Sink<T> {

	/**
	 * ��������������ָ��
	 */
	Sink<D>* down = nullptr;

	/**
	 * ��ǰ�ڵ�����Ѻ���
	 */
	using Consum = function<D(const T&)>;
	Consum _f;

public:
	SinkChain(Consum f) : _f(f)
	{}

	virtual ISinkChain* link(ISinkChain* down) override {
		this->down = (Sink<D>*)down;
		return this;
	}

	virtual void consum(const T& v) override {
		D d = _f(v);
		if (down != nullptr) {
			down->consum(d);
		}
	}

};

/**
 * �����������ѽڵ�
 */
template<typename T> class FilterSink : public Sink<T> {

	/**
	 * ��������������ָ��
	 */
	Sink<T>* down = nullptr;

	/**
	 * ��ǰ�ڵ�����Ѻ���
	 */
	using Consum = function<bool(const T&)>;
	Consum _f;

public:
	FilterSink(Consum f) : _f(f)
	{}

	virtual ISinkChain* link(ISinkChain* down) override {
		this->down = (Sink<T>*)down;
		return this;
	}

	virtual void consum(const T& v) override {
		bool b = _f(v);
		if (b && down != nullptr) {
			down->consum(v);
		}
	}

};

/**
 * ��Լ�������ѽڵ�
 */
template<typename T> class ReduceSink : public Sink<T> {

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

	virtual void consum(const T& v) override {
		_value = _f(_value, v);
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
		this->_sink = new SinkChain<T, D>(f);
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
		for (int i = 0; i < _size; i++) {
			((Sink<T>*)sink)->consum(_array[i]);
		}
	}

	/**
	 * ����������
	 */
	Stream<T>* stream() {
		return new Stream<T>(nullptr, this);
	}

};