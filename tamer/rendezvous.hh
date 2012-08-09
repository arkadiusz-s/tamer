#ifndef TAMER_RENDEZVOUS_HH
#define TAMER_RENDEZVOUS_HH 1
/* Copyright (c) 2007-2012, Eddie Kohler
 * Copyright (c) 2007, Regents of the University of California
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Tamer LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Tamer LICENSE file; the license in that file is
 * legally binding.
 */
#include <tamer/util.hh>
#include <tamer/xbase.hh>
namespace tamer {

/** @file <tamer/rendezvous.hh>
 *  @brief  The rendezvous template classes.
 */

/** @class rendezvous tamer/event.hh <tamer/tamer.hh>
 *  @brief  A set of watched events.
 *
 *  Rendezvous may also be declared with one or zero template arguments, as in
 *  <tt>rendezvous<T0></tt> or <tt>rendezvous<></tt>.  Each specialized
 *  rendezvous class has functions similar to the full-featured rendezvous,
 *  but with parameters to @c join appropriate to the template arguments.
 *  Specialized rendezvous implementations are often more efficient than the
 *  full @c rendezvous.
 */
template <typename I0, typename I1>
class rendezvous : public tamerpriv::abstract_rendezvous { public:

    /** @brief  Construct rendezvous.
     *  @param  flags  tamer::rnormal (default) or tamer::rvolatile
     *
     *  Pass tamer::rvolatile as the @a flags parameter to create a volatile
     *  rendezvous.  Volatile rendezvous do not generate error messages when
     *  their events are dereferenced before trigger. */
    inline rendezvous(rendezvous_flags flags = rnormal);

    /** @brief  Test if any events are ready.
     *
     *  An event is ready if it has triggered, but no @a join or @c twait
     *  has reported it yet.
     */
    inline bool has_ready() const {
	return _bs.size();
    }

    /** @brief  Test if any events are waiting.
     *
     *  An event is waiting until it is either triggered or canceled.
     */
    inline bool has_waiting() const {
	return waiting_;
    }

    /** @brief  Test if any events are ready or waiting.
     */
    inline bool has_events() const {
	return has_ready() || has_waiting();
    }

    /** @brief  Report the next ready event.
     *  @param[out]  i0  Set to the first event ID of the ready event.
     *  @param[out]  i1  Set to the second event ID of the ready event.
     *  @return  True if there was a ready event, false otherwise.
     *
     *  @a i0 and @a i1 were modified if and only if @a join returns true.
     */
    inline bool join(I0 &i0, I1 &i1);

    /** @brief  Remove all events from this rendezvous.
     *
     *  Every event waiting on this rendezvous is made empty.  After clear(),
     *  has_events() will return 0.
     */
    inline void clear();

    /** @internal
     *  @brief  Add an occurrence to this rendezvous.
     *  @param  e   The occurrence.
     *  @param  i0  The occurrence's first event ID.
     *  @param  i1  The occurrence's first event ID.
     */
    inline void add(tamerpriv::simple_event *e, const I0 &i0, const I1 &i1);

    /** @internal
     *  @brief  Mark the triggering of an occurrence.
     *  @param  rid     The occurrence's ID within this rendezvous.
     *  @param  values  True if the occurrence was triggered with values.
     */
    inline void do_complete(tamerpriv::simple_event *e, bool values);

  private:

    struct evtrec {
	I0 i0;
	I1 i1;
	evtrec(const I0 &i0_, const I1 &i1_)
	    : i0(i0_), i1(i1_) {
	}
    };

    tamerutil::ready_set<evtrec> _bs;

};

template <typename I0, typename I1>
inline rendezvous<I0, I1>::rendezvous(rendezvous_flags flags)
    : abstract_rendezvous(flags, tamerpriv::rdefault)
{
}

template <typename I0, typename I1>
inline void rendezvous<I0, I1>::add(tamerpriv::simple_event *e, const I0 &i0, const I1 &i1)
{
    e->initialize(this, _bs.insert(i0, i1));
}

template <typename I0, typename I1>
inline void rendezvous<I0, I1>::do_complete(tamerpriv::simple_event *e, bool)
{
    _bs.push_back_element(e->rid());
    unblock();
}

template <typename I0, typename I1>
inline bool rendezvous<I0, I1>::join(I0 &i0, I1 &i1)
{
    if (evtrec *e = _bs.front_ptr()) {
	i0 = e->i0;
	i1 = e->i1;
	_bs.pop_front();
	return true;
    } else
	return false;
}

template <typename I0, typename I1>
inline void rendezvous<I0, I1>::clear()
{
    abstract_rendezvous::clear();
    _bs.clear();
}


/** @cond specialized_rendezvous */

template <typename I0>
class rendezvous<I0, void> : public tamerpriv::abstract_rendezvous { public:

    inline rendezvous(rendezvous_flags flags = rnormal);

    inline bool join(I0 &);
    inline void clear();

    inline bool has_ready() const {
	return _bs.size();
    }
    inline bool has_waiting() const {
	return waiting_;
    }
    inline bool has_events() const {
	return has_ready() || has_waiting();
    }

    inline void add(tamerpriv::simple_event *e, const I0 &i0);
    inline void do_complete(tamerpriv::simple_event *e, bool values);

  private:

    tamerutil::ready_set<I0> _bs;

};

template <typename I0>
inline rendezvous<I0, void>::rendezvous(rendezvous_flags flags)
    : abstract_rendezvous(flags, tamerpriv::rdefault)
{
}

template <typename I0>
inline void rendezvous<I0, void>::add(tamerpriv::simple_event *e, const I0 &i0)
{
    e->initialize(this, _bs.insert(i0));
}

template <typename I0>
inline void rendezvous<I0, void>::do_complete(tamerpriv::simple_event *e, bool)
{
    _bs.push_back_element(e->rid());
    unblock();
}

template <typename I0>
inline bool rendezvous<I0, void>::join(I0 &i0)
{
    if (I0 *e = _bs.front_ptr()) {
	i0 = *e;
	_bs.pop_front();
	return true;
    } else
	return false;
}

template <typename I0>
inline void rendezvous<I0, void>::clear()
{
    abstract_rendezvous::clear();
    _bs.clear();
}


template <>
class rendezvous<uintptr_t> : public tamerpriv::abstract_rendezvous { public:

    inline rendezvous(rendezvous_flags flags = rnormal);

    inline bool join(uintptr_t &);
    inline void clear();

    inline bool has_ready() const {
	return _buf.size();
    }
    inline bool has_waiting() const {
	return waiting_;
    }
    inline bool has_events() const {
	return has_ready() || has_waiting();
    }

    inline void add(tamerpriv::simple_event *e, uintptr_t i0) TAMER_NOEXCEPT;
    inline void do_complete(tamerpriv::simple_event *e, bool values);

  protected:

    tamerutil::debuffer<uintptr_t> _buf;

};

inline rendezvous<uintptr_t>::rendezvous(rendezvous_flags flags)
    : abstract_rendezvous(flags, tamerpriv::rdefault)
{
}

inline void rendezvous<uintptr_t>::add(tamerpriv::simple_event *e, uintptr_t i0) TAMER_NOEXCEPT
{
    e->initialize(this, i0);
}

inline void rendezvous<uintptr_t>::do_complete(tamerpriv::simple_event *e, bool)
{
    _buf.push_back(e->rid());
    unblock();
}

inline bool rendezvous<uintptr_t>::join(uintptr_t &i0)
{
    if (uintptr_t *x = _buf.front_ptr()) {
	i0 = *x;
	_buf.pop_front();
	return true;
    } else
	return false;
}

inline void rendezvous<uintptr_t>::clear()
{
    abstract_rendezvous::clear();
    _buf.clear();
}


template <typename T>
class rendezvous<T *> : public rendezvous<uintptr_t> { public:

    typedef rendezvous<uintptr_t> inherited;

    rendezvous(rendezvous_flags flags = rnormal)
	: inherited(flags) {
    }

    inline void add(tamerpriv::simple_event *e, T *i0) TAMER_NOEXCEPT {
	inherited::add(e, reinterpret_cast<uintptr_t>(i0));
    }

    inline bool join(T *&i0) {
	if (uintptr_t *x = _buf.front_ptr()) {
	    i0 = reinterpret_cast<T *>(*x);
	    _buf.pop_front();
	    return true;
	} else
	    return false;
    }

};


template <>
class rendezvous<int> : public rendezvous<uintptr_t> { public:

    typedef rendezvous<uintptr_t> inherited;

    rendezvous(rendezvous_flags flags = rnormal)
	: inherited(flags) {
    }

    inline void add(tamerpriv::simple_event *e, int i0) TAMER_NOEXCEPT {
	inherited::add(e, static_cast<uintptr_t>(i0));
    }

    inline bool join(int &i0) {
	if (uintptr_t *x = _buf.front_ptr()) {
	    i0 = static_cast<int>(*x);
	    _buf.pop_front();
	    return true;
	} else
	    return false;
    }

};


template <>
class rendezvous<bool> : public rendezvous<uintptr_t> { public:

    typedef rendezvous<uintptr_t> inherited;

    rendezvous(rendezvous_flags flags = rnormal)
	: inherited(flags) {
    }

    inline void add(tamerpriv::simple_event *e, bool i0) TAMER_NOEXCEPT {
	inherited::add(e, static_cast<uintptr_t>(i0));
    }

    inline bool join(bool &i0) {
	if (uintptr_t *x = _buf.front_ptr()) {
	    i0 = static_cast<bool>(*x);
	    _buf.pop_front();
	    return true;
	} else
	    return false;
    }

};


template <>
class rendezvous<void> : public tamerpriv::abstract_rendezvous { public:

    rendezvous(rendezvous_flags flags = rnormal)
	: abstract_rendezvous(flags, tamerpriv::rdefault), _nready(0) {
    }

    inline void add(tamerpriv::simple_event *e) TAMER_NOEXCEPT {
	e->initialize(this, 1);
    }

    inline void do_complete(tamerpriv::simple_event *, bool) {
	_nready++;
	unblock();
    }

    inline bool join() {
	if (_nready) {
	    _nready--;
	    return true;
	} else
	    return false;
    }

    inline void clear() {
	abstract_rendezvous::clear();
	_nready = 0;
    }

    inline bool has_ready() const {
	return _nready;
    }
    inline bool has_waiting() const {
	return waiting_;
    }
    inline bool has_events() const {
	return has_ready() || has_waiting();
    }

  protected:

    unsigned _nready;

};


class gather_rendezvous : public tamerpriv::abstract_rendezvous { public:

    inline gather_rendezvous(tamerpriv::tamer_closure *c)
	: abstract_rendezvous(rnormal, tamerpriv::rgather), linked_closure_(c) {
    }

    inline tamerpriv::tamer_closure *linked_closure() const {
	return linked_closure_;
    }

    inline bool has_waiting() const {
	return waiting_;
    }

    inline void add(tamerpriv::simple_event *e) TAMER_NOEXCEPT {
	e->initialize(this, 1);
    }

  private:

    tamerpriv::tamer_closure *linked_closure_;

};

/** @endcond */

}
#endif /* TAMER__RENDEZVOUS_HH */
