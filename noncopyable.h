#pragma once


/// @brief 禁止拷贝的类直接private继承该类，可在编译器检查错误
///        这个类应禁止生成对象但继承的子类需要能生成对象，故构造和析构设为protected
class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};