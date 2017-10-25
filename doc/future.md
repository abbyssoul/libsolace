Future
============

Future is a primitive that aliviate the callback hell from async programming.
The generel concept is well introduced in the article [here](https://philipnilsson.github.io/Badness10k/escaping-hell-with-monads/).
A very nice introduction to what kind of problems Promise/Future solve can be found in [this Facebook's post](https://code.facebook.com/posts/1661982097368498).
The implementation is very similar to [futures](https://github.com/facebook/folly/blob/master/folly/futures/README.md#compositional-building-blocks)
that are providided by Facebooks's [folly library](https://github.com/facebook/folly) with one huge difference.
libsolace Future are not associated with any thread. That is creating a Promise/Future does not guaranty that
it will be computed in any other thread. It is a responisibility of the owner of a Promise to perform a computition
using any async mechanism or thread and call promise.setValue() or promise.setError() to inform a holder of the associated Future
that the value is availiable. As a result there is no Future.wait() method as it is possible
(and often beneficial) to use Promise/Future in a single threaded async application.

Example of using Promise/Future with ASIO:
```

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

... // Client of the event that wishes to perform some action when event is ready:
event.asyncWait().then([]() {
    printf("Async event has arrived!\n");
});


```

