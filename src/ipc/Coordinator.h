/*
 * $Id$
 *
 * DEBUG: section 54    Interprocess Communication
 *
 */

#ifndef SQUID_IPC_COORDINATOR_H
#define SQUID_IPC_COORDINATOR_H


#include "Array.h"
#include <map>
#include "ipc/Port.h"
#include "ipc/Messages.h"
#include "ipc/SharedListen.h"

namespace Ipc
{

///  Coordinates shared activities of Strands (Squid processes or threads)
class Coordinator: public Port
{
public:
    static Coordinator* Instance();

public:
    Coordinator();

    void broadcastSignal(int sig) const; ///< send sig to registered strands

protected:
    virtual void start(); // Port (AsyncJob) API
    virtual void receive(const TypedMsgHdr& message); // Port API

    StrandCoord* findStrand(int kidId); ///< registered strand or NULL
    void registerStrand(const StrandCoord &); ///< adds or updates existing
    void handleRegistrationRequest(const StrandCoord &); ///< register,ACK

    /// returns cached socket or calls openListenSocket()
    void handleSharedListenRequest(const SharedListenRequest& request);

    /// calls comm_open_listener()
    int openListenSocket(const SharedListenRequest& request, int &errNo);

private:
    typedef Vector<StrandCoord> Strands; ///< unsorted strands
    Strands strands; ///< registered processes and threads

    typedef std::map<OpenListenerParams, int> Listeners; ///< params:fd map
    Listeners listeners; ///< cached comm_open_listener() results

    static Coordinator* TheInstance; ///< the only class instance in existence

private:
    Coordinator(const Coordinator&); // not implemented
    Coordinator& operator =(const Coordinator&); // not implemented

    CBDATA_CLASS2(Coordinator);
};


} // namespace Ipc

#endif /* SQUID_IPC_COORDINATOR_H */