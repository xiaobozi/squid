#ifndef _SQUID_SRC_COMM_ACCEPT_LIMITER_H
#define _SQUID_SRC_COMM_ACCEPT_LIMITER_H

#include "Array.h"

namespace Comm
{

class ListenStateData;

/**
 * FIFO Queue holding listener socket handlers which have been activated
 * ready to dupe their FD and accept() a new client connection.
 * But when doing so there were not enough FD available to handle the
 * new connection. These handlers are awaiting some FD to become free.
 *
 * defer - used only by Comm layer ListenStateData adding themselves when FD are limited.
 * kick - used by Comm layer when FD are closed.
 */
class AcceptLimiter
{

public:
    /** retrieve the global instance of the queue. */
    static AcceptLimiter &Instance();

    /** delay accepting a new client connection. */
    void defer(Comm::ListenStateData *afd);

    /** try to accept and begin processing any delayed client connections. */
    void kick();

private:
    static AcceptLimiter Instance_;

    /** FIFO queue */
    Vector<Comm::ListenStateData*> deferred;
};

}; // namepace Comm

#endif /* _SQUID_SRC_COMM_ACCEPT_LIMITER_H */