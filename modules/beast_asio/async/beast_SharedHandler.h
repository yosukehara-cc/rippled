//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#ifndef BEAST_SHAREDHANDLER_H_INCLUDED
#define BEAST_SHAREDHANDLER_H_INCLUDED

template <typename T>
struct SharedHandlerAllocator;

/** Reference counted wrapper that can hold any other boost::asio handler.

    This object will match these signatures:

    @code

    void (void)
    void (system::error_code)
    void (system::error_code, std::size_t)

    @endcode

    If the underlying implementation does not support the signature,
    undefined behavior will result.

    Supports these concepts:
        Destructible
*/
class SharedHandler : public SharedObject
{
protected:
    typedef boost::system::error_code error_code;
    typedef boost::function <void(void)> invoked_type;

    SharedHandler () noexcept { }

public:
    // For asio::async_result<>
    typedef void result_type;

    typedef SharedObjectPtr <SharedHandler> Ptr;

    virtual void operator() ();
    virtual void operator() (error_code const&);
    virtual void operator() (error_code const&, std::size_t);

    /** Dispatch the Function on our context. */
    /*
    template <typename Dispatcher, typename Function>
    void dispatch (Dispatcher& dispatcher, BOOST_ASIO_MOVE_ARG(Function) function)
    {
        dispatcher.dispatch (boost::bind (
            &SharedHandler::invoke <Function>, this,
                BOOST_ASIO_MOVE_CAST(Function)(function)));
    }
    */

    template <typename Function>
    void invoke (BOOST_ASIO_MOVE_ARG(Function) f);

    virtual void  invoke (invoked_type& invoked) = 0;
    virtual void* allocate (std::size_t size) = 0;
    virtual void  deallocate (void* p, std::size_t size) = 0;
    virtual bool  is_continuation () = 0;
    virtual void  destroy () = 0;

    static void pure_virtual_called (char const* fileName, int lineNumber);

private:
    template <typename Function>
    friend void  asio_handler_invoke (BOOST_ASIO_MOVE_ARG(Function) f, SharedHandler*);
    friend void* asio_handler_allocate (std::size_t, SharedHandler*);
    friend void  asio_handler_deallocate (void*, std::size_t, SharedHandler*);
    friend bool  asio_handler_is_continuation (SharedHandler*);
    friend struct ContainerDeletePolicy <SharedHandler>;
};

//--------------------------------------------------------------------------

// For SharedObjectPtr <SharedHandler>
template <>
struct ContainerDeletePolicy <SharedHandler>
{
    // SharedObjectPtr will use this when
    // the reference count drops to zero.
    //
    inline static void destroy (SharedHandler* handler)
    {
        handler->destroy ();
    }
};

//--------------------------------------------------------------------------
//
// Context execution guarantees
//

template <class Function>
void asio_handler_invoke (BOOST_ASIO_MOVE_ARG(Function) f, SharedHandler* handler)
{
    handler->invoke (BOOST_ASIO_MOVE_CAST(Function)(f));
}

inline void* asio_handler_allocate (std::size_t size, SharedHandler* handler)
{
    return handler->allocate (size);
}

inline void asio_handler_deallocate (void* p, std::size_t size, SharedHandler* handler)
{
    handler->deallocate (p, size);
}

inline bool asio_handler_is_continuation (SharedHandler* handler)
{
    return handler->is_continuation ();
}

#endif
