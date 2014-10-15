
#include "smtpsend.h"

static vmime::ref <vmime::net::session> g_session = vmime::create <vmime::net::session>();

// Certificate verifier (TLS/SSL)
class NonInteractiveCertificateVerifier : public vmime::security::cert::defaultCertificateVerifier
{
public:

	void verify(vmime::ref <vmime::security::cert::certificateChain> chain)
	{
		try
		{
			setX509TrustedCerts(m_trustedCerts);

			defaultCertificateVerifier::verify(chain);
		}
		catch (vmime::exceptions::certificate_verification_exception&)
		{
			// Obtain subject's certificate
			vmime::ref <vmime::security::cert::certificate> cert = chain->getAt(0);


            // Accept it, and remember user's choice for later
            if (cert->getType() == "X.509")
            {
                m_trustedCerts.push_back(cert.dynamicCast
                    <vmime::security::cert::X509Certificate>());
            }

		}
	}

private:

	static std::vector <vmime::ref <vmime::security::cert::X509Certificate> > m_trustedCerts;
};


std::vector <vmime::ref <vmime::security::cert::X509Certificate> >NonInteractiveCertificateVerifier::m_trustedCerts;



void sendMessage(string fromstr, string tostr, string subjectstr, string contentstr, string urlString)
{
	try
	{
		vmime::utility::url url(urlString);
		vmime::ref <vmime::net::transport> tr = g_session->getTransport(url);
		// Enable TLS support if available
		tr->setProperty("connection.tls", true);
		tr->setCertificateVerifier(vmime::create <NonInteractiveCertificateVerifier>());
		vmime::mailbox from(fromstr);
		vmime::mailboxList to;
        to.appendMailbox(vmime::create <vmime::mailbox>(tostr));
		std::ostringstream data;
        data << "From: " << fromstr << "\r\nTo: " << tostr << "\r\nSubject: " << subjectstr << "\r\n" << contentstr << "\r\n";
		// Connect to server
		tr->connect();
        vmime::string msgData = data.str();
        vmime::utility::inputStreamStringAdapter vis(msgData);
        tr->send(from, to, vis, msgData.length());
		tr->disconnect();
	}
	catch (vmime::exception& e)
	{
	}
	catch (std::exception& e)
	{
	}
}

