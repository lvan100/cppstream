#pragma once

#include "assert.h"

#include <functional>
using namespace std;

/**
 * 数据消费器链条接口
 */
class ISinkChain {

public:
	/*
	 * 连接下游数据消费器，并返回当前数据消费器
	 */
	virtual ISinkChain* link(ISinkChain* down) = 0;

};

/**
 * 数据消费器接口
 */
template<typename T> class Sink : public ISinkChain {

public:
	/**
	 * 数据消费接口
	 */
	virtual void consum(const T& value) = 0;

};

/**
 * 数据消费器链条的一般节点对象
 */
template<typename T, typename D> class SinkChain : public Sink<T> {

	/**
	 * 下游数据消费器指针
	 */
	Sink<D>* down = nullptr;

	/**
	 * 当前节点的消费函数
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
 * 过滤数据消费节点
 */
template<typename T> class FilterSink : public Sink<T> {

	/**
	 * 下游数据消费器指针
	 */
	Sink<T>* down = nullptr;

	/**
	 * 当前节点的消费函数
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
 * 规约数据消费节点
 */
template<typename T> class ReduceSink : public Sink<T> {

	/**
	 * 当前节点的消费函数
	 */
	using Consum = function<T(const T&, const T&)>;
	Consum _f;

	/**
	 * 保存终端处理结果
	 */
	T _value;

public:
	ReduceSink(T init, Consum f) : _value(init), _f(f)
	{}

	virtual ISinkChain* link(ISinkChain* down) override {
		assert(false); // 理论上不应该调用此函数
		return this;
	}

	virtual void consum(const T& v) override {
		_value = _f(_value, v);
	}

	/**
	 * 获取终端处理结果
	 */
	T get() {
		return _value;
	}

};

/**
 * 数据分流器
 */
class Splitter {

public:
	/**
	 * 开始消费数据
	 */
	virtual void consum(ISinkChain* sink) = 0;

};

/**
 * 流基类
 */
class StreamBase {

public:
	/**
	 * 流的深度
	 */
	int _depth = 0;

	/**
	 * 流的数据消费器
	 */
	ISinkChain* _sink = nullptr;

	/**
	 * 数据分流器指针
	 */
	Splitter* _splitter = nullptr;

	/**
	 * 流的上游节点指针
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
	 * 对流内的对象进行计数
	 */
	virtual int count() = 0;

};

/**
 * 流
 */
template<typename T> class Stream : public StreamBase {

public:
	Stream(StreamBase* up, Splitter* sp)
		: StreamBase(up, sp)
	{}

	/**
	 * 将当前流转换为其他流
	 */
	template<typename D> Stream<D>* map(function<D(const T&)> f) {
		this->_sink = new SinkChain<T, D>(f);
		return new Stream<D>(this, _splitter);
	}

	/**
	 * 对当前流进行归约操作
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

		// 从流的最终节点开始释放
		delete this;

		return result;
	}

	/**
	 * 对流中的元素进行过滤
	 */
	Stream* filter(function<bool(const T&)> f) {
		this->_sink = new FilterSink<T>(f);
		return new Stream(this, _splitter);
	}

	/**
	 * 对流内的对象进行计数
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
 * 数组类型的数据分离器
 */
template<typename T> class ArraySplitter : public Splitter {

	/**
	 * 数组的元素数量
	 */
	int _size = 0;

	/**
	 * 数组元素的指针
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
	 * 构造流对象
	 */
	Stream<T>* stream() {
		return new Stream<T>(nullptr, this);
	}

};