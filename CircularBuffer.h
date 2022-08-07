/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Created by Maurik Holtrop on 4/29/22.
////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef RASTERMON_CIRCULARBUFFER_H
#define RASTERMON_CIRCULARBUFFER_H

#include <algorithm>
#include <iterator>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <utility>


template<typename T>
class CircularBuffer {
// public:
private:
   template <bool isConst> struct BufferIterator;
   
public:
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   
public:
   explicit CircularBuffer(size_t size)
         :_buff{std::unique_ptr<T[]>(new T[size])}, _max_size{size}{}

   CircularBuffer(const CircularBuffer& other)
         :_buff{std::unique_ptr<T[]>(new T[other._max_size])},
          _max_size{other._max_size},
          _size{other._size},
          _head{other._head},
          _tail{other._tail}{
      std::copy(other.data(), other.data() + _max_size, _buff.get());
   }


   CircularBuffer& operator=(const CircularBuffer& other){
      if ( this != &other){
         _buff.reset(new T[other._max_size]);
         _max_size = other._max_size;
         _size = other._size;
         _head = other._head;
         _tail = other._tail;
         std::copy(other.data(), other.data() + _max_size, _buff.get());
      }
      return *this;
   }

   CircularBuffer(CircularBuffer&& other)
         :_buff{std::move(other._buff)},
          _max_size{other._max_size},
          _size{other._size},
          _head{other._head},
          _tail{other._tail}{

      other._buff = nullptr;
      other._max_size = 0;
      other._size = 0;
      other._head = 0;
      other._tail = 0;
   }


   CircularBuffer& operator=(CircularBuffer&& other) {
      if ( this != &other){
         _buff = std::move(other._buff);
         _max_size = other._max_size;
         _size = other._size;
         _head = other._head;
         _tail = other._tail;

         other._buff = nullptr;
         other._max_size = 0;
         other._size = 0;
         other._head = 0;
         other._tail = 0;
      }
      return *this;
   }

   void push_back(const T& data);
   void push_back(T&& data);
   void pop_front();
   const T& front();
   const T& back();
   const T& front() const;
   const T& back() const;
   void clear() {   std::lock_guard<std::mutex> _lck(_mtx);
      _head = _tail = _size = 0;};
   bool empty() const {return _size == 0;};
   bool full() const {return _size == _max_size;};
   size_type capacity() const {return _max_size;};
   size_type size() const {   std::lock_guard<std::mutex> _lck(_mtx);
      return _size;};
   size_type size_low() const {   std::lock_guard<std::mutex> _lck(_mtx);
      if( _tail + 1 < _head ) return( _size );
      else return _max_size - _tail;};
   size_type size_high() const {   std::lock_guard<std::mutex> _lck(_mtx);
      if( _tail < _head ) return( 0 );
      else return _head;};
   size_type buffer_size() const {return sizeof(T)*_max_size;};
   const T* data() const { return _buff.get(); }
   const T* data_low() const {
      return _buff.get() + _tail; };
   const T* data_high() const {
      return _buff.get(); };

   const T& operator[](size_type index) const;
   const T& operator[](size_type index);
   const T& at(size_type index) const;
   const T& at(size_type index);
   const T& at_absolute(size_type index) const;
   const T& at_absolute(size_type index);

   void print_state() const{
      std::cout << "s: " << size() << " sl: " << size_low() << " sh: " << size_high() << " tail: " << _tail << " head:" << _head << std::endl;
      for(int i=0; i< std::min(size(), (size_t) 100); ++i) std::cout << at_absolute(i) << " ";
      std::cout << std::endl;
   }

   typedef BufferIterator<false> iterator;
   typedef BufferIterator<true> const_iterator;

   iterator begin();
   const_iterator begin() const;
   iterator end();
   const_iterator end() const;
   const_iterator cbegin() const;
   const_iterator cend() const;
   iterator rbegin();
   const_iterator rbegin() const;
   iterator rend();
   const_iterator rend() const;

// public:
private:
   void _increment_bufferstate();
   void _decrement_bufferstate();
   mutable std::mutex _mtx;
   std::unique_ptr<T[]> _buff;
   size_type _head = 0;
   size_type _tail = 0;
   size_type _size = 0;
   size_type _max_size = 0;

   template<bool isConst = false>
   struct  BufferIterator{
   public:
      friend class CircularBuffer<T>;
      typedef std::random_access_iterator_tag iterator_category;
      typedef ptrdiff_t difference_type;
      typedef typename std::conditional<isConst, const T&, T&>::type const reference;
      typedef typename std::conditional<isConst, const T*, T*>::type pointer;
      typedef typename std::conditional<isConst, const CircularBuffer<T>*,
            CircularBuffer<T>*>::type cbuf_pointer;
   // public:
   private:
      cbuf_pointer _ptrToBuffer;
      size_type _offset;
      size_type _index;
      bool _reverse;

      bool _comparable(const BufferIterator<isConst>& other) const{
         return (_ptrToBuffer == other._ptrToBuffer)&&(_reverse == other._reverse);
      }

   public:
      BufferIterator()
            :_ptrToBuffer{nullptr}, _offset{0}, _index{0}, _reverse{false}{}

      BufferIterator(const BufferIterator<false>& it)
            :_ptrToBuffer{it._ptrToBuffer},
             _offset{it._offset},
             _index{it._index},
             _reverse{it._reverse}{}

      reference operator*(){
         if(_reverse)
            return (*_ptrToBuffer)[(_ptrToBuffer->size() - _index - 1)];
         return (*_ptrToBuffer)[_index];
      }

      pointer operator->() { return &(operator*()); }

      reference operator[](size_type index){
         BufferIterator iter = *this;
         iter._index += index;
         return *iter;
      }

      BufferIterator& operator++(){
         ++_index;
         return *this;
      }

      BufferIterator operator++(int){
         BufferIterator iter = *this;
         ++_index;
         return iter;
      }

      BufferIterator& operator--(){
         --_index;
         return *this;
      }

      BufferIterator operator--(int){
         BufferIterator iter = *this;
         --_index;
         return iter;
      }

      friend BufferIterator operator+(BufferIterator lhsiter, difference_type n){
         lhsiter._index += n;
         return lhsiter;
      }

      friend BufferIterator operator+(difference_type n, BufferIterator rhsiter){
         rhsiter._index += n;
         return rhsiter;
      }


      BufferIterator& operator+=(difference_type n){
         _index += n;
         return *this;
      }

      friend BufferIterator operator-(BufferIterator lhsiter, difference_type n){
         lhsiter._index -= n;
         return lhsiter;
      }

      friend difference_type operator-(const BufferIterator& lhsiter, const BufferIterator& rhsiter){

         return lhsiter._index - rhsiter._index;
      }

      BufferIterator& operator-=(difference_type n){
         _index -= n;
         return *this;
      }

      bool operator==(const BufferIterator& other) const{
         if (!_comparable(other))
            return false;
         return ((_index == other._index)&&(_offset == other._offset));
      }

      bool operator!=(const BufferIterator& other) const{
         if (!_comparable(other))
            return true;
         return ((_index != other._index)||(_offset != other._offset));
      }

      bool operator<(const BufferIterator& other) const {
         if (!_comparable(other))
            return false;
         return ((_index + _offset)<(other._index+other._offset));
      }

      bool operator>(const BufferIterator& other) const{
         if (!_comparable(other))
            return false;
         return ((_index + _offset)>(other._index+other._offset));
      }

      bool operator<=(const BufferIterator& other) const {
         if (!_comparable(other))
            return false;
         return ((_index + _offset)<=(other._index+other._offset));
      }

      bool operator>=(const BufferIterator& other) const {
         if (!_comparable(other))
            return false;
         return ((_index + _offset)>=(other._index+other._offset));
      }
   };
};

template<typename T>
inline
const T& CircularBuffer<T>::front() {
   std::lock_guard<std::mutex> _lck(_mtx);
   if(empty())
      throw std::length_error("front function called on empty buffer");
   return _buff[_tail];
}

template<typename T>
inline
const T& CircularBuffer<T>::back() {
   std::lock_guard<std::mutex> _lck(_mtx);
   if(empty())
      throw std::length_error("back function called on empty buffer");
   return _head == 0 ? _buff[_max_size - 1] : _buff[_head - 1];
}

template<typename T>
inline
const T& CircularBuffer<T>::front() const{
   std::lock_guard<std::mutex> _lck(_mtx);
   if(empty())
      throw std::length_error("front function called on empty buffer");
   return _buff[_tail];
}

template<typename T>
inline
const T& CircularBuffer<T>::back() const{
   std::lock_guard<std::mutex> _lck(_mtx);
   if(empty())
      throw std::length_error("back function called on empty buffer");
   return _head == 0 ? _buff[_max_size - 1] : _buff[_head - 1];
}

template<typename T>
inline
void CircularBuffer<T>::push_back(const T& data){
   std::lock_guard<std::mutex> _lck(_mtx);
   _buff[_head] = data;
   _increment_bufferstate();
}

template<typename T>
inline
void CircularBuffer<T>::push_back(T&& data) {
   std::lock_guard<std::mutex> _lck(_mtx);
   _buff[_head] = std::move(data);
   _increment_bufferstate();
}


template<typename T>
inline
void CircularBuffer<T>::_increment_bufferstate(){
   if(full())
      _tail = (_tail + 1)%_max_size;
   else
      ++_size;
   _head = (_head + 1)%_max_size;
}

template<typename T>
inline
void CircularBuffer<T>::pop_front(){
   std::lock_guard<std::mutex> _lck(_mtx);
   if(empty())
      throw std::length_error("pop_front called on empty buffer");
   _decrement_bufferstate();
}

template<typename T>
inline
void CircularBuffer<T>::_decrement_bufferstate(){
   --_size;
   _tail = (_tail + 1)%_max_size;
}

template<typename T>
inline
const T& CircularBuffer<T>::operator[](size_t index) {
   std::lock_guard<std::mutex> _lck(_mtx);
   if((index<0)||(index>=_size))
      throw std::out_of_range("Index is out of Range of buffer size");
   index += _tail;
   index %= _max_size;
   return _buff[index];
}

template<typename T>
inline
const T& CircularBuffer<T>::operator[](size_t index) const {
   std::lock_guard<std::mutex> _lck(_mtx);
   if((index<0)||(index>=_size))
      throw std::out_of_range("Index is out of Range of buffer size");
   index += _tail;
   index %= _max_size;
   return _buff[index];
}

template<typename T>
inline
const T& CircularBuffer<T>::at(size_t index) {
   std::lock_guard<std::mutex> _lck(_mtx);
   if((index<0)||(index>=_size))
      throw std::out_of_range("Index is out of Range of buffer size");
   index += _tail;
   index %= _max_size;
   return _buff[index];
}

template<typename T>
inline
const T& CircularBuffer<T>::at(size_t index) const {
   std::lock_guard<std::mutex> _lck(_mtx);
   if((index<0)||(index>=_size))
      throw std::out_of_range("Index is out of Range of buffer size");
   index += _tail;
   index %= _max_size;
   return _buff[index];
}

template<typename T>
inline
const T& CircularBuffer<T>::at_absolute(size_type index) {
   std::lock_guard<std::mutex> _lck(_mtx);
   if((index<0)||(index>=_size))
      throw std::out_of_range("Index is out of Range of buffer size");
   return _buff[index];
}

template<typename T>
inline
const T& CircularBuffer<T>::at_absolute(size_t index) const {
   std::lock_guard<std::mutex> _lck(_mtx);
   if((index<0)||(index>=_size))
      throw std::out_of_range("Index is out of Range of buffer size");
   return _buff[index];
}

template<typename T>
inline
typename CircularBuffer<T>::iterator CircularBuffer<T>::begin() {
   std::lock_guard<std::mutex> _lck(_mtx);
   iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = 0;
   iter._reverse = false;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::begin() const{
   std::lock_guard<std::mutex> _lck(_mtx);
   const_iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = 0;
   iter._reverse = false;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::iterator CircularBuffer<T>::end() {
   std::lock_guard<std::mutex> _lck(_mtx);
   iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = _size;
   iter._reverse = false;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::end() const{
   std::lock_guard<std::mutex> _lck(_mtx);
   const_iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = _size;
   iter._reverse = false;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::cbegin() const{
   std::lock_guard<std::mutex> _lck(_mtx);
   const_iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = 0;
   iter._reverse = false;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::cend() const{
   std::lock_guard<std::mutex> _lck(_mtx);
   const_iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = _size;
   iter._reverse = false;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::iterator CircularBuffer<T>::rbegin(){
   std::lock_guard<std::mutex> _lck(_mtx);
   iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = 0;
   iter._reverse = true;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::rbegin() const{
   std::lock_guard<std::mutex> _lck(_mtx);
   const_iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = 0;
   iter._reverse = true;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::iterator CircularBuffer<T>::rend(){
   std::lock_guard<std::mutex> _lck(_mtx);
   iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = _size;
   iter._reverse = true;
   return iter;
}

template<typename T>
inline
typename CircularBuffer<T>::const_iterator CircularBuffer<T>::rend() const{
   std::lock_guard<std::mutex> _lck(_mtx);
   const_iterator iter;
   iter._ptrToBuffer = this;
   iter._offset = _tail;
   iter._index = _size;
   iter._reverse = true;
   return iter;
}

#endif /* RASTERMON_CIRCULARBUFFER_H */

