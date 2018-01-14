Promise/Future
============
Future is a primitive that alleviate the call-back hell from of programming.
The general concept is well introduced in the article [here](https://philipnilsson.github.io/Badness10k/escaping-hell-with-monads/).
Another nice introduction to what kind of problems Promise/Future solve can be found in this [Facebook engineering post](https://code.facebook.com/posts/1661982097368498).

The implementation is similar in spirit to [futures](https://github.com/facebook/folly/blob/master/folly/futures/README.md)
provided by Facebooks's [folly library](https://github.com/facebook/folly) with one huge difference:
libsolace Future are not associated with any thread. That is creation of a Promise/Future does not guaranty that it will be computed in any other thread or indeed at all. It is responsibility of the Promise owner to perform a computation using any async mechanism or thread and call promise.setValue() or promise.setError() to inform a holder of the associated Future that the value is available. As a result there is no Future.wait() method. It is possible
(and often beneficial) to use Promise/Future in a single threaded async application.
Here is an typical example of using Promise/Future with [ASIO](http://www.boost.org/doc/libs/master/doc/html/boost_asio.html):

```C++
Future<void> Event::asyncWait() {
    Promise<void> promise;
    auto f = promise.getFuture();
    _eventFd.async_read_some(asio::buffer(&_readBuffer, sizeof(_readBuffer)),
        [pm = std::move(promise)] (const asio::error_code& error, std::size_t) mutable {
        if (error) {
            pm.setError(fromAsioError(error));
        } else {
            pm.setValue();
        }
    });

    return f;
}

...
// Client of the event that wishes to perform some action when event is ready:
event.asyncWait().then([]() {
    printf("Async event has arrived!\n");
});
```
