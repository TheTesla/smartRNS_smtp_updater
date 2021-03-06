
/*  Copyright (C) 2014 Stefan Helmert <stefan.helmert@gmx.net>
    smartRNS-Update-Client
*/


#include <iostream>

//#include <netinet/in.h>
//#include <arpa/nameser.h>
//#include <resolv.h>
//#include <string>
//#include <vector>
#include "configuration.h"
#include "data.h"
#include "parse.h"
#include "crypto.h"
#include "dnsquery.h"
#include "coreops.h"

#include "smtpsend.h"


#define MAXRECS 4

int main(int argc, char *argv[])
{
	vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();



    string domain, request, options, record;
    bool n, v, s, r, c, w, d;
    n = v = s = r = c = w = d = false; // allow unencrypted mode, verbose (show all recursion steps), show data structure, raw (show raw data), show configuration
    vector<string> decvec;
    vector<string> encvec;
    smartrns_conf_t conf, lastconf, resetconf;
    smartrns_data_t data;
    vector<keyval_t> keyvalvec;
    size_t pos = 0;


    resetconf.contenc = NO_CONTENC;
    resetconf.contprimenc = NO_PRIMENC;
    conf = resetconf;


    vector<string> txts;

    if(2==argc){
        request = argv[1]; // the domain to query
    }else if(3==argc | 4==argc){
        options = argv[1];
        if(std::string::npos != options.find_first_of('n')) n = true;
        if(std::string::npos != options.find_first_of('v')) v = true;
        if(std::string::npos != options.find_first_of('s')) s = true;
        if(std::string::npos != options.find_first_of('r')) r = true;
        if(std::string::npos != options.find_first_of('c')) c = true;
        if(std::string::npos != options.find_first_of('w')) w = true;
        if(std::string::npos != options.find_first_of('d')) d = true;
        request = argv[2];
        if(w){
            if(4==argc){
                record = argv[3];
            }else{
                cout << "Empty record written!" << endl;
            }
        }
    }else{
        cout << endl;
        cout << "Please specify Domain to lookup!" << endl;
        cout << endl;
        cout << "This program is designed to query and decode smartRNS data over standard DNS." << endl << endl;
        cout << "USAGE: smartRNSclient [options] YourName@smartrns.net" << endl << endl;
        cout << "options: n - allow unencrypted mode" << endl;
        cout << "         v - verbose: show all cycles" << endl;
        cout << "         s - show data-/config-structure" << endl;
        cout << "         r - raw (decrypted TXT records)" << endl;
        cout << "         c - show config" << endl;
        cout << "         w - write netries" << endl;
        cout << "         d - delete entries" << endl << endl;
        cout << "    Copyright (C) 2014 Stefan Helmert <stefan.helmert@gmx.net>" << endl;
        cout << endl;
        return 0;
    }


    // everything after the @
    domain = uritop(request, &pos);

    // now before the @
    while(1){ // recursion over all subdomains

        // get the DNS records
        txts = getTXTrecs(domain, MAXRECS);
        lastconf = conf;

        decvec.clear();
        try{
            // do decryption of requestet DNS content
            decvec = decrypt(txts, conf.salt+request, conf.contprimenc, conf.contenc);
        }
        catch(const primenc_et& f){
            conf = resetconf;
            cout << "No content primary encoding set!" << endl << endl;
            break;
        }
        catch(const contenc_et& e){
            conf = resetconf;
            keyvalvec.clear();
            cout << "No content secondary encoding set!" << endl << endl;
            break;
        }
        // interprete the content
        keyvalvec = txtrec2keyvalvec(decvec);
        conf = smartrnsvec2smartrnsconf(keyvalvec);
        data = smartrnsvec2smartrnsdata(keyvalvec);

        if(!n){
            if(NO_URIENC == conf.urienc){
                cout << "No domain encryption used by server - break! Please use parameter 'n' to allow sending an unencrypted domain - This is not secure!" << endl;
                break;
            }
            if(NO_CONTENC == conf.contenc){
                cout << "No content encryption used by server - break! Please use parameter 'n' to allow receiving unencrypted data - This is not secure!" << endl;
                break;
            }
        }

        if(0==pos) break; // no remaining subdomain


        if(v){
            if(r) {
                cout << "REQUEST" << endl << endl << "    " << domain << endl << endl;
                print_decvec(decvec);
            }
            if(s) print_key_val_vec(keyvalvec);   // output
            if(c) print_smartrns_config(conf);    // output
            print_smartrns_data(data);      // output
        }

        // next subdomain
        try{
            domain = getdomain(request, &pos, conf.subdomlen, conf.uriprimenc, conf.urienc, conf.salt)+'.'+domain;
        }
        catch(const primenc_et& f){
            if(PRIMENC_NOT_SPEC == f){
                cout << "URI primary encoding of subdomain not specified in configuration, aborting!" << endl;
            }
            break;
        }
        catch(const urienc_et& e){
            if(URIENC_NOT_SPEC == e){
                cout << "URI secondary encoding of subdomain not specified in configuration, aborting!" << endl;
            }
            break;
        }

    }

    if(r) {
        cout << "REQUEST" << endl << endl << "    " << domain << endl << endl;
        print_decvec(decvec);
    }

    //std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);

    if(d) sendMessage("testuser@smartrns.net", "test.access@smartrns.net", "testsubject", "del#\n"+domain+"#");
    if(w){
        encvec.clear();
        encvec.push_back(record);
        cout << request << endl;
        encvec = encrypt(encvec, lastconf.salt+request, lastconf.contprimenc, lastconf.contenc);
        sendMessage("testuser@smartrns.net", "test.access@smartrns.net", "testsubject", "add#\n"+domain+"#\n"+encvec[0]+"#");
    }
    if(s) print_key_val_vec(keyvalvec);   // output
    if(c) print_smartrns_config(conf);    // output

    print_smartrns_data(data);      // output



    //sendMessage("testuser@smartrns.net", "test.access@smartrns.net", "testsubject", "Das ist ein text.");

    return 0;
}
