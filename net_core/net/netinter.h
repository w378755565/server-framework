#include "src\_external\sources.h"
#include "src\_external\netctrl.h"

class Export NetInterface
{
public:
    NetInterface(NetMode netMode = Reactor);
    virtual ~NetInterface();
};