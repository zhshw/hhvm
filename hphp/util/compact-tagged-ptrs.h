/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#ifndef incl_HPHP_UTIL_COMPACT_TAGGED_PTRS_H_
#define incl_HPHP_UTIL_COMPACT_TAGGED_PTRS_H_

#include <cstdint>

/*
 * A combined pointer + tag intended to save space.  The tag must fit in
 * a 16 bit integer anywhere you use this class.
 *
 * For all supported architectures, this class relies on the property that a
 * "canonical form" address must have all the upper bits set to the same value
 * as bit 47.  Further, it relies on the OS using only "lower half" canonical
 * form addresses for userland pointers.  For more, see:
 *
 *     http://en.wikipedia.org/wiki/X86-64#Canonical_form_addresses
 */

namespace HPHP {

//////////////////////////////////////////////////////////////////////

#if !(defined(__x86_64__) || defined(_M_X64) || defined(__powerpc64__) || \
      defined(__aarch64__))
#error CompactTaggedPtr is not supported on your architecture.
#endif

template<class T, class TagType = uint32_t>
struct CompactTaggedPtr {
  using Opaque = uintptr_t;
  CompactTaggedPtr() { set(TagType{}, 0); }

  // for save and restore
  explicit CompactTaggedPtr(Opaque v) : m_data(v) {}
  Opaque getOpaque() const { return m_data; }

  void set(TagType ttag, T* ptr) {
    auto const tag = static_cast<uint64_t>(ttag);
    assert(tag <= 0xffffu);
    assert(!(uintptr_t(ptr) >> 48));
    m_data = uintptr_t(ptr) | (size_t(tag) << 48);
  }

  TagType tag() const { return static_cast<TagType>(m_data >> 48); }

  T* ptr() const {
    return reinterpret_cast<T*>(m_data & (-1ull >> 16));
  }

private:
  uintptr_t m_data;
};

//////////////////////////////////////////////////////////////////////

/*
 * Same thing, named differently for self-documentation when the tag is
 * representing the size of something.
 */
template<class T>
struct CompactSizedPtr {
  void set(uint32_t size, T* ptr) { m_data.set(size, ptr); }

  uint32_t size() const { return m_data.tag(); }
  const T* ptr()  const { return m_data.ptr(); }
        T* ptr()        { return m_data.ptr(); }

private:
  CompactTaggedPtr<T> m_data;
};

//////////////////////////////////////////////////////////////////////

}

#endif
