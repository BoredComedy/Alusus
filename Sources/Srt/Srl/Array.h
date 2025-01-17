/**
 * @file Srl/Array.h
 * Contains the header of class Srl::Array.
 *
 * @copyright Copyright (C) 2021 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <https://alusus.org/license.html>.
 */
//==============================================================================

#ifndef SRL_ARRAY_H
#define SRL_ARRAY_H

namespace Srl
{

template <class T> class ArrayData
{
  //=================
  // Member Variables

  public: ArchInt refCount;
  public: ArchInt length;
  public: ArchInt bufSize;
  public: T buf[1];

  //=================
  // Member Functions

  public: static ArrayData<T>* alloc(ArchInt size) {
    if (size < 2) size = 2;
    ArchInt byteCount = sizeof(ArrayData<T>) + sizeof(T) * (size - 1);
    ArrayData<T> *data = (ArrayData<T>*)malloc(byteCount);
    data->bufSize = size;
    data->length = 0;
    data->refCount = 1;
    return data;
  }

  public: static ArrayData<T>* realloc(ArrayData<T> *data, ArchInt newSize) {
    if (newSize < 2) newSize = 2;
    ArchInt byteCount = sizeof(ArrayData<T>) + sizeof(T) * (newSize - 1);
    data = (ArrayData<T>*)::realloc(data, byteCount);
    data->bufSize = newSize;
    return data;
  }

  public: static void release(ArrayData<T> *data) {
    ArchInt i;
    for (i = 0; i < data->length; ++i) data->buf[i].~T();
    free(data);
  }
};


//============================================================================
// Array Type

template <class T> class Array
{
  //=================
  // Member Variables

  private: ArrayData<T> *data;

  //==========================
  // Cosntructors & Destructor

  public: Array() {
    this->_init();
  }

  public: Array(Array const &src) {
    this->_init();
    this->assign(src);
  }

  public: Array(ArchInt size, T val) {
    this->_init();
    this->data = ArrayData<T>::alloc(size);
    for (; size > 0; --size) this->add(val);
  }

  public: ~Array() {
    this->_release();
  }

  //=================
  // Member Functions

  private: void _init() {
    this->data = 0;
  }

  private: void _release() {
    if (this->data != 0) {
      if (--this->data->refCount == 0) ArrayData<T>::release(this->data);
      this->_init();
    }
  }

  public: void reserve(ArchInt size) {
    if (this->data == 0) this->data = ArrayData<T>::alloc(size);
    else if (size > this->data->bufSize) this->data = ArrayData<T>::realloc(this->data, size);
  }

  public: ArchInt getLength() const {
    if (this->data == 0) return 0;
    else return this->data->length;
  }

  public: ArchInt getBufSize() const {
    if (this->data == 0) return 0;
    else return this->data->bufSize;
  }

  public: void assign (Array<T> const &ary) {
    this->_release();
    this->data = ary.data;
    if (this->data != 0) {
      ++this->data->refCount;
    }
  }

  private: void _prepareToModify (Bool enlarge) {
    if (this->data == 0) {
      this->data = ArrayData<T>::alloc(2);
    } else if (this->data->refCount == 1) {
      if (enlarge && this->data->length >= this->data->bufSize) {
        this->data = ArrayData<T>::realloc(this->data, this->data->bufSize + (this->data->bufSize >> 1));
      }
    } else {
      ArrayData<T> *curData = this->data;
      --this->data->refCount;
      this->data = ArrayData<T>::alloc(curData->length + (curData->length >> 1));
      ArchInt i;
      for (i = 0; i < curData->length; ++i) new(this->data->buf + i) T(curData->buf[i]);
      this->data->length = curData->length;
    }
  }

  public: void add (T item) {
    this->_prepareToModify(true);
    new (this->data->buf + this->data->length) T(item);
    ++this->data->length;
  }

  public: void insert (ArchInt index, T item) {
    if (index < 0 || index >= this->getLength()) {
      this->add(item);
    } else {
      this->_prepareToModify(true);
      memcpy(this->data->buf + index + 1, this->data->buf + index, sizeof(T) * (this->data->length - index));
      new(this->data->buf + index) T(item);
      ++this->data->length;
    }
  }

  public: void remove (ArchInt index) {
    if (index >= 0 && index < this->getLength()) {
      this->_prepareToModify(false);
      this->data->buf[index].~T();
      if (index < this->getLength() - 1) {
        memcpy(this->data->buf + index, this->data->buf + index + 1, sizeof(T) * (this->data->length - (index + 1)));
      };
      --this->data->length;
    }
  }

  public: void clear() {
    this->_release();
  };

  public: T& at (ArchInt i) {
    static T dummy;
    if (i >= 0 && i < this->getLength()) return this->data->buf[i]; else return dummy;
  }

  public: T const& at (ArchInt i) const {
    static T dummy;
    if (i >= 0 && i < this->getLength()) return this->data->buf[i]; else return dummy;
  }

  public: ArchInt findPos (T const &val) const {
    for (ArchInt i = 0; i < this->getLength(); ++i) {
      if (this->at(i) == val) return i;
    }
    return -1;
  }

  //==========
  // Operators

  public: void operator=(Array<T> const &value) {
    this->assign(value);
  }

  public: T& operator()(ArchInt i) {
    return this->at(i);
  }

  public: T const& operator()(ArchInt i) const {
    return this->at(i);
  }
}; // class

} // namespace

#endif
