
#include "smtpsend.h"

int main()
{
	vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();

    sendMessage("testuser@smartrns.net", "test.access@smartrns.net", "testsubject", "Das ist ein text.");

    return 0;
}
