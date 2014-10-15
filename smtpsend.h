#ifndef SMTPSEND_H_INCLUDED
#define SMTPSEND_H_INCLUDED

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include <vmime/vmime.hpp>
#include <vmime/platforms/posix/posixHandler.hpp>

using namespace std;

void sendMessage(string fromstr, string tostr, string subjectstr, string contentstr, string urlString = "sendmail://localhost");


#endif // SMTPSEND_H_INCLUDED
