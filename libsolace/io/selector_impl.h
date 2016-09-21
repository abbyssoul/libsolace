#ifndef SELECTOR_IMPL_H
#define SELECTOR_IMPL_H


#include <tuple>


class Solace::IO::Selector::IPollerImpl {
public:

    virtual ~IPollerImpl() = default;

    virtual void add(ISelectable* selectable, int events) = 0;

    virtual void remove(const ISelectable* selectable) = 0;

    virtual std::tuple<uint, uint> poll(uint32 msec) = 0;

    virtual Selector::Event getEvent(uint i) = 0;

    virtual int advance(uint i) = 0;
};



#endif // SELECTOR_IMPL_H
