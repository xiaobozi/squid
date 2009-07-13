#include "squid.h"

#include "ConfigParser.h"
#include "Array.h"      // really Vector
#include "adaptation/Config.h"
#include "adaptation/AccessRule.h"
#include "adaptation/Service.h"
#include "adaptation/ServiceFilter.h"
#include "adaptation/ServiceGroups.h"

#define ServiceGroup ServiceGroup

Adaptation::ServiceGroup::ServiceGroup(const String &aKind, bool allSame):
    kind(aKind), method(methodNone), point(pointNone),
    allServicesSame(allSame)
{
}

Adaptation::ServiceGroup::~ServiceGroup()
{
}

void
Adaptation::ServiceGroup::parse()
{
    ConfigParser::ParseString(&id);

    wordlist *names = NULL;
    ConfigParser::ParseWordList(&names);
    for (wordlist *i = names; i; i = i->next)
        services.push_back(i->key);
    wordlistDestroy(&names);
}

// Note: configuration code aside, this method is called by DynamicServiceChain
void
Adaptation::ServiceGroup::finalize()
{
    // 1) warn if services have different methods or vectoring point
    // 2) warn if all-same services have different bypass status
    // 3) warn if there are seemingly identical services in the group
    // TODO: optimize by remembering ServicePointers rather than IDs

    String baselineKey;
    bool baselineBypass = false;
    for (Pos pos = 0; has(pos); ++pos) {
        // TODO: quit on all errors
        const String &sid = services[pos];
        ServicePointer service = at(pos);
        if (service != NULL) {
            if (method == methodNone) { 
                // optimization: cache values that should be the same
                method = service->cfg().method;
                point = service->cfg().point;
            } else {
                if (method != service->cfg().method)
                    finalizeMsg("Inconsistent service method for", sid, true);
                if (point != service->cfg().point)
                    finalizeMsg("Inconsistent vectoring point for", sid, true);
            }

            checkUniqueness(pos);

            if (allServicesSame) { 
                if (!baselineKey.size()) {
                    baselineKey = service->cfg().key;
                    baselineBypass = service->cfg().bypass;
                } else
                if (baselineBypass != service->cfg().bypass) {
                    debugs(93,0, "WARNING: Inconsistent bypass in " << kind <<
                        ' ' << id << " may produce surprising results: " <<
                        baselineKey << " vs. " << sid);
                }
            }
        } else { 
            finalizeMsg("ERROR: Unknown adaptation name", sid, true);
        }
    }
    debugs(93,7, HERE << "finalized " << kind << ": " << id);
}

/// checks that the service name or URI is not repeated later in the group
void
Adaptation::ServiceGroup::checkUniqueness(const Pos checkedPos) const
{
    ServicePointer checkedService = at(checkedPos);
    if (!checkedService) // should not happen but be robust
        return;

    for (Pos p = checkedPos + 1; has(p); ++p) {
        ServicePointer s = at(p);
        if (s != NULL && s->cfg().key == checkedService->cfg().key)
            finalizeMsg("duplicate service name", s->cfg().key, false);
        else
        if (s != NULL && s->cfg().uri == checkedService->cfg().uri)
            finalizeMsg("duplicate service URI", s->cfg().uri, false);
    }
}

/// emits a formatted warning or error message at the appropriate dbg level
void
Adaptation::ServiceGroup::finalizeMsg(const char *msg, const String &culprit,
    bool error) const
{
    const int level = error ? DBG_CRITICAL : DBG_IMPORTANT;
    const char *pfx = error ? "ERROR: " : "WARNING: ";
    debugs(93,level, pfx << msg << ' ' << culprit << " in " << kind << " '" <<
        id << "'");
}

Adaptation::ServicePointer Adaptation::ServiceGroup::at(const Pos pos) const {
    return FindService(services[pos]);
}

/// \todo: optimize to cut search short instead of looking for the best svc
bool
Adaptation::ServiceGroup::wants(const ServiceFilter &filter) const
{
    Pos pos = 0;
    return findService(filter, pos);
}

bool
Adaptation::ServiceGroup::findService(const ServiceFilter &filter, Pos &pos) const
{
    if (method != filter.method || point != filter.point) {
        debugs(93,5,HERE << id << " serves another location");
        return false; // assume other services have the same wrong location
    }

    // find the next interested service, skipping problematic ones if possible
    bool foundEssential = false;
    Pos essPos = 0;
    for (; has(pos); ++pos) {
        debugs(93,9,HERE << id << " checks service at " << pos);
        ServicePointer service = at(pos);

        if (!service)
            continue; // the service was lost due to reconfiguration

        if (!service->wants(filter))
            continue; // the service is not interested

        if (service->up() || !service->probed()) {
            debugs(93,9,HERE << id << " has matching service at " << pos);
            return true;
        }

        if (service->cfg().bypass) { // we can safely ignore bypassable downers
            debugs(93,9,HERE << id << " has bypassable service at " << pos);
            continue;
        }

        if (!allServicesSame) { // cannot skip (i.e., find best) service
            debugs(93,9,HERE << id << " has essential service at " << pos);
            return true;
        }

        if (!foundEssential) {
            debugs(93,9,HERE << id << " searches for best essential service from " << pos);
            foundEssential = true;
            essPos = pos;
        }
    }

    if (foundEssential) {
        debugs(93,9,HERE << id << " has best essential service at " << essPos);
        pos = essPos;
        return true;
    }

    debugs(93,5,HERE << id << " has no matching services");
    return false;
}

bool
Adaptation::ServiceGroup::findReplacement(const ServiceFilter &filter, Pos &pos) const
{
    return allServicesSame && findService(filter, pos);
}

bool
Adaptation::ServiceGroup::findLink(const ServiceFilter &filter, Pos &pos) const
{
    return !allServicesSame && findService(filter, pos);
}


/* ServiceSet */

Adaptation::ServiceSet::ServiceSet(): ServiceGroup("adaptation set", true)
{
}


/* SingleService */

Adaptation::SingleService::SingleService(const String &aServiceId):
        ServiceGroup("single-service group", false)
{
    id = aServiceId;
    services.push_back(aServiceId);
}


/* ServiceChain */

Adaptation::ServiceChain::ServiceChain(): ServiceGroup("adaptation chain", false)
{
}


/* ServiceChain */

Adaptation::DynamicServiceChain::DynamicServiceChain(const String &ids,
    const ServiceGroupPointer prev)
{
    kind = "dynamic adaptation chain"; // TODO: optimize by using String const
    id = ids; // use services ids as the dynamic group ID

    // initialize cache to improve consistency checks in finalize()
    if (prev != NULL) {
        method = prev->method;
        point = prev->point;
    }

    // populate services storage with supplied service ids
    const char *item = NULL;
    int ilen = 0;
    const char *pos = NULL;
    while (strListGetItem(&ids, ',', &item, &ilen, &pos))
        services.push_back(item);

    finalize(); // will report [dynamic] config errors
}

/* ServicePlan */

Adaptation::ServicePlan::ServicePlan(): pos(0), atEof(true)
{
}

Adaptation::ServicePlan::ServicePlan(const ServiceGroupPointer &g,
    const ServiceFilter &filter):
    group(g), pos(0), atEof(!g || !g->has(pos))
{
    // this will find the first service because starting pos is zero
    if (!atEof && !group->findService(filter, pos))
        atEof = true;
}

Adaptation::ServicePointer
Adaptation::ServicePlan::current() const
{
    // may return NULL even if not atEof
    return atEof ? Adaptation::ServicePointer() : group->at(pos);
}

Adaptation::ServicePointer
Adaptation::ServicePlan::replacement(const ServiceFilter &filter) {
    if (!atEof && !group->findReplacement(filter, ++pos))
        atEof = true;
    return current();
}

Adaptation::ServicePointer
Adaptation::ServicePlan::next(const ServiceFilter &filter) {
    if (!atEof && !group->findLink(filter, ++pos))
        atEof = true;
    return current();
}

std::ostream &
Adaptation::ServicePlan::print(std::ostream &os) const
{
    if (!group)
        return os << "[nil]";

    return os << group->id << '[' << pos << ".." << group->services.size() <<
        (atEof ? ".]" : "]");
}


/* globals */

Adaptation::Groups &
Adaptation::AllGroups()
{
    static Groups TheGroups;
    return TheGroups;
}

Adaptation::ServiceGroupPointer
Adaptation::FindGroup(const ServiceGroup::Id &id)
{
    typedef Groups::iterator GI;
    for (GI i = AllGroups().begin(); i != AllGroups().end(); ++i) {
        if ((*i)->id == id)
            return *i;
    }

    return NULL;
}
