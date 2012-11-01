// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/base/x509_certificate.h"

#include "base/basictypes.h"
#include "base/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/pickle.h"
#include "base/sha1.h"
#include "base/string_number_conversions.h"
#include "base/string_split.h"
#include "crypto/rsa_private_key.h"
#include "net/base/asn1_util.h"
#include "net/base/cert_test_util.h"
#include "net/base/net_errors.h"
#include "net/base/test_certificate_data.h"
#include "net/base/test_data_directory.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(USE_NSS)
#include <cert.h>
#endif

using base::HexEncode;
using base::Time;

namespace net {

// Certificates for test data. They're obtained with:
//
// $ openssl s_client -connect [host]:443 -showcerts > /tmp/host.pem < /dev/null
// $ openssl x509 -inform PEM -outform DER < /tmp/host.pem > /tmp/host.der
//
// For fingerprint
// $ openssl x509 -inform DER -fingerprint -noout < /tmp/host.der

// For valid_start, valid_expiry
// $ openssl x509 -inform DER -text -noout < /tmp/host.der |
//    grep -A 2 Validity
// $ date +%s -d '<date str>'

// Google's cert.
uint8 google_fingerprint[] = {
  0xab, 0xbe, 0x5e, 0xb4, 0x93, 0x88, 0x4e, 0xe4, 0x60, 0xc6, 0xef, 0xf8,
  0xea, 0xd4, 0xb1, 0x55, 0x4b, 0xc9, 0x59, 0x3c
};

// webkit.org's cert.
uint8 webkit_fingerprint[] = {
  0xa1, 0x4a, 0x94, 0x46, 0x22, 0x8e, 0x70, 0x66, 0x2b, 0x94, 0xf9, 0xf8,
  0x57, 0x83, 0x2d, 0xa2, 0xff, 0xbc, 0x84, 0xc2
};

// thawte.com's cert (it's EV-licious!).
uint8 thawte_fingerprint[] = {
  0x85, 0x04, 0x2d, 0xfd, 0x2b, 0x0e, 0xc6, 0xc8, 0xaf, 0x2d, 0x77, 0xd6,
  0xa1, 0x3a, 0x64, 0x04, 0x27, 0x90, 0x97, 0x37
};

// A certificate for https://www.unosoft.hu/, whose AIA extension contains
// an LDAP URL without a host name.
uint8 unosoft_hu_fingerprint[] = {
  0x32, 0xff, 0xe3, 0xbe, 0x2c, 0x3b, 0xc7, 0xca, 0xbf, 0x2d, 0x64, 0xbd,
  0x25, 0x66, 0xf2, 0xec, 0x8b, 0x0f, 0xbf, 0xd8
};

// The fingerprint of the Google certificate used in the parsing tests,
// which is newer than the one included in the x509_certificate_data.h
uint8 google_parse_fingerprint[] = {
  0x40, 0x50, 0x62, 0xe5, 0xbe, 0xfd, 0xe4, 0xaf, 0x97, 0xe9, 0x38, 0x2a,
  0xf1, 0x6c, 0xc8, 0x7c, 0x8f, 0xb7, 0xc4, 0xe2
};

// The fingerprint for the Thawte SGC certificate
uint8 thawte_parse_fingerprint[] = {
  0xec, 0x07, 0x10, 0x03, 0xd8, 0xf5, 0xa3, 0x7f, 0x42, 0xc4, 0x55, 0x7f,
  0x65, 0x6a, 0xae, 0x86, 0x65, 0xfa, 0x4b, 0x02
};

// Dec 18 00:00:00 2009 GMT
const double kGoogleParseValidFrom = 1261094400;
// Dec 18 23:59:59 2011 GMT
const double kGoogleParseValidTo = 1324252799;

struct CertificateFormatTestData {
  const char* file_name;
  X509Certificate::Format format;
  uint8* chain_fingerprints[3];
};

const CertificateFormatTestData FormatTestData[] = {
  // DER Parsing - single certificate, DER encoded
  { "google.single.der", X509Certificate::FORMAT_SINGLE_CERTIFICATE,
    { google_parse_fingerprint,
      NULL, } },
  // DER parsing - single certificate, PEM encoded
  { "google.single.pem", X509Certificate::FORMAT_SINGLE_CERTIFICATE,
    { google_parse_fingerprint,
      NULL, } },
  // PEM parsing - single certificate, PEM encoded with a PEB of
  // "CERTIFICATE"
  { "google.single.pem", X509Certificate::FORMAT_PEM_CERT_SEQUENCE,
    { google_parse_fingerprint,
      NULL, } },
  // PEM parsing - sequence of certificates, PEM encoded with a PEB of
  // "CERTIFICATE"
  { "google.chain.pem", X509Certificate::FORMAT_PEM_CERT_SEQUENCE,
    { google_parse_fingerprint,
      thawte_parse_fingerprint,
      NULL, } },
  // PKCS#7 parsing - "degenerate" SignedData collection of certificates, DER
  // encoding
  { "google.binary.p7b", X509Certificate::FORMAT_PKCS7,
    { google_parse_fingerprint,
      thawte_parse_fingerprint,
      NULL, } },
  // PKCS#7 parsing - "degenerate" SignedData collection of certificates, PEM
  // encoded with a PEM PEB of "CERTIFICATE"
  { "google.pem_cert.p7b", X509Certificate::FORMAT_PKCS7,
    { google_parse_fingerprint,
      thawte_parse_fingerprint,
      NULL, } },
  // PKCS#7 parsing - "degenerate" SignedData collection of certificates, PEM
  // encoded with a PEM PEB of "PKCS7"
  { "google.pem_pkcs7.p7b", X509Certificate::FORMAT_PKCS7,
    { google_parse_fingerprint,
      thawte_parse_fingerprint,
      NULL, } },
  // All of the above, this time using auto-detection
  { "google.single.der", X509Certificate::FORMAT_AUTO,
    { google_parse_fingerprint,
      NULL, } },
  { "google.single.pem", X509Certificate::FORMAT_AUTO,
    { google_parse_fingerprint,
      NULL, } },
  { "google.chain.pem", X509Certificate::FORMAT_AUTO,
    { google_parse_fingerprint,
      thawte_parse_fingerprint,
      NULL, } },
  { "google.binary.p7b", X509Certificate::FORMAT_AUTO,
    { google_parse_fingerprint,
      thawte_parse_fingerprint,
      NULL, } },
  { "google.pem_cert.p7b", X509Certificate::FORMAT_AUTO,
    { google_parse_fingerprint,
      thawte_parse_fingerprint,
      NULL, } },
  { "google.pem_pkcs7.p7b", X509Certificate::FORMAT_AUTO,
    { google_parse_fingerprint,
      thawte_parse_fingerprint,
      NULL, } },
};

void CheckGoogleCert(const scoped_refptr<X509Certificate>& google_cert,
                     uint8* expected_fingerprint,
                     double valid_from, double valid_to) {
  ASSERT_NE(static_cast<X509Certificate*>(NULL), google_cert);

  const CertPrincipal& subject = google_cert->subject();
  EXPECT_EQ("www.google.com", subject.common_name);
  EXPECT_EQ("Mountain View", subject.locality_name);
  EXPECT_EQ("California", subject.state_or_province_name);
  EXPECT_EQ("US", subject.country_name);
  EXPECT_EQ(0U, subject.street_addresses.size());
  ASSERT_EQ(1U, subject.organization_names.size());
  EXPECT_EQ("Google Inc", subject.organization_names[0]);
  EXPECT_EQ(0U, subject.organization_unit_names.size());
  EXPECT_EQ(0U, subject.domain_components.size());

  const CertPrincipal& issuer = google_cert->issuer();
  EXPECT_EQ("Thawte SGC CA", issuer.common_name);
  EXPECT_EQ("", issuer.locality_name);
  EXPECT_EQ("", issuer.state_or_province_name);
  EXPECT_EQ("ZA", issuer.country_name);
  EXPECT_EQ(0U, issuer.street_addresses.size());
  ASSERT_EQ(1U, issuer.organization_names.size());
  EXPECT_EQ("Thawte Consulting (Pty) Ltd.", issuer.organization_names[0]);
  EXPECT_EQ(0U, issuer.organization_unit_names.size());
  EXPECT_EQ(0U, issuer.domain_components.size());

  // Use DoubleT because its epoch is the same on all platforms
  const Time& valid_start = google_cert->valid_start();
  EXPECT_EQ(valid_from, valid_start.ToDoubleT());

  const Time& valid_expiry = google_cert->valid_expiry();
  EXPECT_EQ(valid_to, valid_expiry.ToDoubleT());

  const SHA1HashValue& fingerprint = google_cert->fingerprint();
  for (size_t i = 0; i < 20; ++i)
    EXPECT_EQ(expected_fingerprint[i], fingerprint.data[i]);

  std::vector<std::string> dns_names;
  google_cert->GetDNSNames(&dns_names);
  ASSERT_EQ(1U, dns_names.size());
  EXPECT_EQ("www.google.com", dns_names[0]);
}

TEST(X509CertificateTest, GoogleCertParsing) {
  scoped_refptr<X509Certificate> google_cert(
      X509Certificate::CreateFromBytes(
          reinterpret_cast<const char*>(google_der), sizeof(google_der)));

  CheckGoogleCert(google_cert, google_fingerprint,
                  1238192407,   // Mar 27 22:20:07 2009 GMT
                  1269728407);  // Mar 27 22:20:07 2010 GMT
}

TEST(X509CertificateTest, WebkitCertParsing) {
  scoped_refptr<X509Certificate> webkit_cert(X509Certificate::CreateFromBytes(
      reinterpret_cast<const char*>(webkit_der), sizeof(webkit_der)));

  ASSERT_NE(static_cast<X509Certificate*>(NULL), webkit_cert);

  const CertPrincipal& subject = webkit_cert->subject();
  EXPECT_EQ("Cupertino", subject.locality_name);
  EXPECT_EQ("California", subject.state_or_province_name);
  EXPECT_EQ("US", subject.country_name);
  EXPECT_EQ(0U, subject.street_addresses.size());
  ASSERT_EQ(1U, subject.organization_names.size());
  EXPECT_EQ("Apple Inc.", subject.organization_names[0]);
  ASSERT_EQ(1U, subject.organization_unit_names.size());
  EXPECT_EQ("Mac OS Forge", subject.organization_unit_names[0]);
  EXPECT_EQ(0U, subject.domain_components.size());

  const CertPrincipal& issuer = webkit_cert->issuer();
  EXPECT_EQ("Go Daddy Secure Certification Authority", issuer.common_name);
  EXPECT_EQ("Scottsdale", issuer.locality_name);
  EXPECT_EQ("Arizona", issuer.state_or_province_name);
  EXPECT_EQ("US", issuer.country_name);
  EXPECT_EQ(0U, issuer.street_addresses.size());
  ASSERT_EQ(1U, issuer.organization_names.size());
  EXPECT_EQ("GoDaddy.com, Inc.", issuer.organization_names[0]);
  ASSERT_EQ(1U, issuer.organization_unit_names.size());
  EXPECT_EQ("http://certificates.godaddy.com/repository",
      issuer.organization_unit_names[0]);
  EXPECT_EQ(0U, issuer.domain_components.size());

  // Use DoubleT because its epoch is the same on all platforms
  const Time& valid_start = webkit_cert->valid_start();
  EXPECT_EQ(1205883319, valid_start.ToDoubleT());  // Mar 18 23:35:19 2008 GMT

  const Time& valid_expiry = webkit_cert->valid_expiry();
  EXPECT_EQ(1300491319, valid_expiry.ToDoubleT());  // Mar 18 23:35:19 2011 GMT

  const SHA1HashValue& fingerprint = webkit_cert->fingerprint();
  for (size_t i = 0; i < 20; ++i)
    EXPECT_EQ(webkit_fingerprint[i], fingerprint.data[i]);

  std::vector<std::string> dns_names;
  webkit_cert->GetDNSNames(&dns_names);
  ASSERT_EQ(2U, dns_names.size());
  EXPECT_EQ("*.webkit.org", dns_names[0]);
  EXPECT_EQ("webkit.org", dns_names[1]);

  // Test that the wildcard cert matches properly.
  EXPECT_TRUE(webkit_cert->VerifyNameMatch("www.webkit.org"));
  EXPECT_TRUE(webkit_cert->VerifyNameMatch("foo.webkit.org"));
  EXPECT_TRUE(webkit_cert->VerifyNameMatch("webkit.org"));
  EXPECT_FALSE(webkit_cert->VerifyNameMatch("www.webkit.com"));
  EXPECT_FALSE(webkit_cert->VerifyNameMatch("www.foo.webkit.com"));
}

TEST(X509CertificateTest, ThawteCertParsing) {
  scoped_refptr<X509Certificate> thawte_cert(X509Certificate::CreateFromBytes(
      reinterpret_cast<const char*>(thawte_der), sizeof(thawte_der)));

  ASSERT_NE(static_cast<X509Certificate*>(NULL), thawte_cert);

  const CertPrincipal& subject = thawte_cert->subject();
  EXPECT_EQ("www.thawte.com", subject.common_name);
  EXPECT_EQ("Mountain View", subject.locality_name);
  EXPECT_EQ("California", subject.state_or_province_name);
  EXPECT_EQ("US", subject.country_name);
  EXPECT_EQ(0U, subject.street_addresses.size());
  ASSERT_EQ(1U, subject.organization_names.size());
  EXPECT_EQ("Thawte Inc", subject.organization_names[0]);
  EXPECT_EQ(0U, subject.organization_unit_names.size());
  EXPECT_EQ(0U, subject.domain_components.size());

  const CertPrincipal& issuer = thawte_cert->issuer();
  EXPECT_EQ("thawte Extended Validation SSL CA", issuer.common_name);
  EXPECT_EQ("", issuer.locality_name);
  EXPECT_EQ("", issuer.state_or_province_name);
  EXPECT_EQ("US", issuer.country_name);
  EXPECT_EQ(0U, issuer.street_addresses.size());
  ASSERT_EQ(1U, issuer.organization_names.size());
  EXPECT_EQ("thawte, Inc.", issuer.organization_names[0]);
  ASSERT_EQ(1U, issuer.organization_unit_names.size());
  EXPECT_EQ("Terms of use at https://www.thawte.com/cps (c)06",
            issuer.organization_unit_names[0]);
  EXPECT_EQ(0U, issuer.domain_components.size());

  // Use DoubleT because its epoch is the same on all platforms
  const Time& valid_start = thawte_cert->valid_start();
  EXPECT_EQ(1227052800, valid_start.ToDoubleT());  // Nov 19 00:00:00 2008 GMT

  const Time& valid_expiry = thawte_cert->valid_expiry();
  EXPECT_EQ(1263772799, valid_expiry.ToDoubleT());  // Jan 17 23:59:59 2010 GMT

  const SHA1HashValue& fingerprint = thawte_cert->fingerprint();
  for (size_t i = 0; i < 20; ++i)
    EXPECT_EQ(thawte_fingerprint[i], fingerprint.data[i]);

  std::vector<std::string> dns_names;
  thawte_cert->GetDNSNames(&dns_names);
  ASSERT_EQ(1U, dns_names.size());
  EXPECT_EQ("www.thawte.com", dns_names[0]);
}

// Test that all desired AttributeAndValue pairs can be extracted when only
// a single RelativeDistinguishedName is present. "Normally" there is only
// one AVA per RDN, but some CAs place all AVAs within a single RDN.
// This is a regression test for http://crbug.com/101009
TEST(X509CertificateTest, MultivalueRDN) {
  FilePath certs_dir = GetTestCertsDirectory();

  scoped_refptr<X509Certificate> multivalue_rdn_cert =
      ImportCertFromFile(certs_dir, "multivalue_rdn.pem");
  ASSERT_NE(static_cast<X509Certificate*>(NULL), multivalue_rdn_cert);

  const CertPrincipal& subject = multivalue_rdn_cert->subject();
  EXPECT_EQ("Multivalue RDN Test", subject.common_name);
  EXPECT_EQ("", subject.locality_name);
  EXPECT_EQ("", subject.state_or_province_name);
  EXPECT_EQ("US", subject.country_name);
  EXPECT_EQ(0U, subject.street_addresses.size());
  ASSERT_EQ(1U, subject.organization_names.size());
  EXPECT_EQ("Chromium", subject.organization_names[0]);
  ASSERT_EQ(1U, subject.organization_unit_names.size());
  EXPECT_EQ("Chromium net_unittests", subject.organization_unit_names[0]);
  ASSERT_EQ(1U, subject.domain_components.size());
  EXPECT_EQ("Chromium", subject.domain_components[0]);
}

// Test that characters which would normally be escaped in the string form,
// such as '=' or '"', are not escaped when parsed as individual components.
// This is a regression test for http://crbug.com/102839
TEST(X509CertificateTest, UnescapedSpecialCharacters) {
  FilePath certs_dir = GetTestCertsDirectory();

  scoped_refptr<X509Certificate> unescaped_cert =
      ImportCertFromFile(certs_dir, "unescaped.pem");
  ASSERT_NE(static_cast<X509Certificate*>(NULL), unescaped_cert);

  const CertPrincipal& subject = unescaped_cert->subject();
  EXPECT_EQ("127.0.0.1", subject.common_name);
  EXPECT_EQ("Mountain View", subject.locality_name);
  EXPECT_EQ("California", subject.state_or_province_name);
  EXPECT_EQ("US", subject.country_name);
  ASSERT_EQ(1U, subject.street_addresses.size());
  EXPECT_EQ("1600 Amphitheatre Parkway", subject.street_addresses[0]);
  ASSERT_EQ(1U, subject.organization_names.size());
  EXPECT_EQ("Chromium = \"net_unittests\"", subject.organization_names[0]);
  ASSERT_EQ(2U, subject.organization_unit_names.size());
  EXPECT_EQ("net_unittests", subject.organization_unit_names[0]);
  EXPECT_EQ("Chromium", subject.organization_unit_names[1]);
  EXPECT_EQ(0U, subject.domain_components.size());
}

TEST(X509CertificateTest, SerialNumbers) {
  scoped_refptr<X509Certificate> google_cert(
      X509Certificate::CreateFromBytes(
          reinterpret_cast<const char*>(google_der), sizeof(google_der)));

  static const uint8 google_serial[16] = {
    0x01,0x2a,0x39,0x76,0x0d,0x3f,0x4f,0xc9,
    0x0b,0xe7,0xbd,0x2b,0xcf,0x95,0x2e,0x7a,
  };

  ASSERT_EQ(sizeof(google_serial), google_cert->serial_number().size());
  EXPECT_TRUE(memcmp(google_cert->serial_number().data(), google_serial,
                     sizeof(google_serial)) == 0);

  // We also want to check a serial number where the first byte is >= 0x80 in
  // case the underlying library tries to pad it.
  scoped_refptr<X509Certificate> paypal_null_cert(
      X509Certificate::CreateFromBytes(
          reinterpret_cast<const char*>(paypal_null_der),
          sizeof(paypal_null_der)));

  static const uint8 paypal_null_serial[3] = {0x00, 0xf0, 0x9b};
  ASSERT_EQ(sizeof(paypal_null_serial),
            paypal_null_cert->serial_number().size());
  EXPECT_TRUE(memcmp(paypal_null_cert->serial_number().data(),
                     paypal_null_serial, sizeof(paypal_null_serial)) == 0);
}

TEST(X509CertificateTest, CAFingerprints) {
  FilePath certs_dir = GetTestCertsDirectory();

  scoped_refptr<X509Certificate> server_cert =
      ImportCertFromFile(certs_dir, "salesforce_com_test.pem");
  ASSERT_NE(static_cast<X509Certificate*>(NULL), server_cert);

  scoped_refptr<X509Certificate> intermediate_cert1 =
      ImportCertFromFile(certs_dir, "verisign_intermediate_ca_2011.pem");
  ASSERT_NE(static_cast<X509Certificate*>(NULL), intermediate_cert1);

  scoped_refptr<X509Certificate> intermediate_cert2 =
      ImportCertFromFile(certs_dir, "verisign_intermediate_ca_2016.pem");
  ASSERT_NE(static_cast<X509Certificate*>(NULL), intermediate_cert2);

  X509Certificate::OSCertHandles intermediates;
  intermediates.push_back(intermediate_cert1->os_cert_handle());
  scoped_refptr<X509Certificate> cert_chain1 =
      X509Certificate::CreateFromHandle(server_cert->os_cert_handle(),
                                        intermediates);

  intermediates.clear();
  intermediates.push_back(intermediate_cert2->os_cert_handle());
  scoped_refptr<X509Certificate> cert_chain2 =
      X509Certificate::CreateFromHandle(server_cert->os_cert_handle(),
                                        intermediates);

  // No intermediate CA certicates.
  intermediates.clear();
  scoped_refptr<X509Certificate> cert_chain3 =
      X509Certificate::CreateFromHandle(server_cert->os_cert_handle(),
                                        intermediates);

  static const uint8 cert_chain1_ca_fingerprint[20] = {
    0xc2, 0xf0, 0x08, 0x7d, 0x01, 0xe6, 0x86, 0x05, 0x3a, 0x4d,
    0x63, 0x3e, 0x7e, 0x70, 0xd4, 0xef, 0x65, 0xc2, 0xcc, 0x4f
  };
  static const uint8 cert_chain2_ca_fingerprint[20] = {
    0xd5, 0x59, 0xa5, 0x86, 0x66, 0x9b, 0x08, 0xf4, 0x6a, 0x30,
    0xa1, 0x33, 0xf8, 0xa9, 0xed, 0x3d, 0x03, 0x8e, 0x2e, 0xa8
  };
  // The SHA-1 hash of nothing.
  static const uint8 cert_chain3_ca_fingerprint[20] = {
    0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55,
    0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09
  };
  EXPECT_TRUE(memcmp(cert_chain1->ca_fingerprint().data,
                     cert_chain1_ca_fingerprint, 20) == 0);
  EXPECT_TRUE(memcmp(cert_chain2->ca_fingerprint().data,
                     cert_chain2_ca_fingerprint, 20) == 0);
  EXPECT_TRUE(memcmp(cert_chain3->ca_fingerprint().data,
                     cert_chain3_ca_fingerprint, 20) == 0);
}

TEST(X509CertificateTest, ParseSubjectAltNames) {
  FilePath certs_dir = GetTestCertsDirectory();

  scoped_refptr<X509Certificate> san_cert =
      ImportCertFromFile(certs_dir, "subjectAltName_sanity_check.pem");
  ASSERT_NE(static_cast<X509Certificate*>(NULL), san_cert);

  std::vector<std::string> dns_names;
  std::vector<std::string> ip_addresses;
  san_cert->GetSubjectAltName(&dns_names, &ip_addresses);

  // Ensure that DNS names are correctly parsed.
  ASSERT_EQ(1U, dns_names.size());
  EXPECT_EQ("test.example", dns_names[0]);

  // Ensure that both IPv4 and IPv6 addresses are correctly parsed.
  ASSERT_EQ(2U, ip_addresses.size());

  static const uint8 kIPv4Address[] = {
      0x7F, 0x00, 0x00, 0x02
  };
  ASSERT_EQ(arraysize(kIPv4Address), ip_addresses[0].size());
  EXPECT_EQ(0, memcmp(ip_addresses[0].data(), kIPv4Address,
                      arraysize(kIPv4Address)));

  static const uint8 kIPv6Address[] = {
      0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
  };
  ASSERT_EQ(arraysize(kIPv6Address), ip_addresses[1].size());
  EXPECT_EQ(0, memcmp(ip_addresses[1].data(), kIPv6Address,
                      arraysize(kIPv6Address)));

  // Ensure the subjectAltName dirName has not influenced the handling of
  // the subject commonName.
  EXPECT_EQ("127.0.0.1", san_cert->subject().common_name);
}

TEST(X509CertificateTest, ExtractSPKIFromDERCert) {
  FilePath certs_dir = GetTestCertsDirectory();
  scoped_refptr<X509Certificate> cert =
      ImportCertFromFile(certs_dir, "nist.der");
  ASSERT_NE(static_cast<X509Certificate*>(NULL), cert);

  std::string derBytes;
  EXPECT_TRUE(X509Certificate::GetDEREncoded(cert->os_cert_handle(),
                                             &derBytes));

  base::StringPiece spkiBytes;
  EXPECT_TRUE(asn1::ExtractSPKIFromDERCert(derBytes, &spkiBytes));

  uint8 hash[base::kSHA1Length];
  base::SHA1HashBytes(reinterpret_cast<const uint8*>(spkiBytes.data()),
                      spkiBytes.size(), hash);

  EXPECT_EQ(0, memcmp(hash, kNistSPKIHash, sizeof(hash)));
}

TEST(X509CertificateTest, ExtractCRLURLsFromDERCert) {
  FilePath certs_dir = GetTestCertsDirectory();
  scoped_refptr<X509Certificate> cert =
      ImportCertFromFile(certs_dir, "nist.der");
  ASSERT_NE(static_cast<X509Certificate*>(NULL), cert);

  std::string derBytes;
  EXPECT_TRUE(X509Certificate::GetDEREncoded(cert->os_cert_handle(),
                                             &derBytes));

  std::vector<base::StringPiece> crl_urls;
  EXPECT_TRUE(asn1::ExtractCRLURLsFromDERCert(derBytes, &crl_urls));

  EXPECT_EQ(1u, crl_urls.size());
  if (crl_urls.size() > 0) {
    EXPECT_EQ("http://SVRSecure-G3-crl.verisign.com/SVRSecureG3.crl",
              crl_urls[0].as_string());
  }
}

// Tests X509CertificateCache via X509Certificate::CreateFromHandle.  We
// call X509Certificate::CreateFromHandle several times and observe whether
// it returns a cached or new OSCertHandle.
TEST(X509CertificateTest, Cache) {
  X509Certificate::OSCertHandle google_cert_handle;
  X509Certificate::OSCertHandle thawte_cert_handle;

  // Add a single certificate to the certificate cache.
  google_cert_handle = X509Certificate::CreateOSCertHandleFromBytes(
      reinterpret_cast<const char*>(google_der), sizeof(google_der));
  scoped_refptr<X509Certificate> cert1(X509Certificate::CreateFromHandle(
      google_cert_handle, X509Certificate::OSCertHandles()));
  X509Certificate::FreeOSCertHandle(google_cert_handle);

  // Add the same certificate, but as a new handle.
  google_cert_handle = X509Certificate::CreateOSCertHandleFromBytes(
      reinterpret_cast<const char*>(google_der), sizeof(google_der));
  scoped_refptr<X509Certificate> cert2(X509Certificate::CreateFromHandle(
      google_cert_handle, X509Certificate::OSCertHandles()));
  X509Certificate::FreeOSCertHandle(google_cert_handle);

  // A new X509Certificate should be returned.
  EXPECT_NE(cert1.get(), cert2.get());
  // But both instances should share the underlying OS certificate handle.
  EXPECT_EQ(cert1->os_cert_handle(), cert2->os_cert_handle());
  EXPECT_EQ(0u, cert1->GetIntermediateCertificates().size());
  EXPECT_EQ(0u, cert2->GetIntermediateCertificates().size());

  // Add the same certificate, but this time with an intermediate. This
  // should result in the intermediate being cached. Note that this is not
  // a legitimate chain, but is suitable for testing.
  google_cert_handle = X509Certificate::CreateOSCertHandleFromBytes(
      reinterpret_cast<const char*>(google_der), sizeof(google_der));
  thawte_cert_handle = X509Certificate::CreateOSCertHandleFromBytes(
      reinterpret_cast<const char*>(thawte_der), sizeof(thawte_der));
  X509Certificate::OSCertHandles intermediates;
  intermediates.push_back(thawte_cert_handle);
  scoped_refptr<X509Certificate> cert3(X509Certificate::CreateFromHandle(
      google_cert_handle, intermediates));
  X509Certificate::FreeOSCertHandle(google_cert_handle);
  X509Certificate::FreeOSCertHandle(thawte_cert_handle);

  // Test that the new certificate, even with intermediates, results in the
  // same underlying handle being used.
  EXPECT_EQ(cert1->os_cert_handle(), cert3->os_cert_handle());
  // Though they use the same OS handle, the intermediates should be different.
  EXPECT_NE(cert1->GetIntermediateCertificates().size(),
      cert3->GetIntermediateCertificates().size());
}

TEST(X509CertificateTest, Pickle) {
  X509Certificate::OSCertHandle google_cert_handle =
      X509Certificate::CreateOSCertHandleFromBytes(
          reinterpret_cast<const char*>(google_der), sizeof(google_der));
  X509Certificate::OSCertHandle thawte_cert_handle =
      X509Certificate::CreateOSCertHandleFromBytes(
          reinterpret_cast<const char*>(thawte_der), sizeof(thawte_der));

  X509Certificate::OSCertHandles intermediates;
  intermediates.push_back(thawte_cert_handle);
  scoped_refptr<X509Certificate> cert = X509Certificate::CreateFromHandle(
      google_cert_handle, intermediates);
  ASSERT_NE(static_cast<X509Certificate*>(NULL), cert.get());

  X509Certificate::FreeOSCertHandle(google_cert_handle);
  X509Certificate::FreeOSCertHandle(thawte_cert_handle);

  Pickle pickle;
  cert->Persist(&pickle);

  PickleIterator iter(pickle);
  scoped_refptr<X509Certificate> cert_from_pickle =
      X509Certificate::CreateFromPickle(
          pickle, &iter, X509Certificate::PICKLETYPE_CERTIFICATE_CHAIN_V3);
  ASSERT_NE(static_cast<X509Certificate*>(NULL), cert_from_pickle);
  EXPECT_TRUE(X509Certificate::IsSameOSCert(
      cert->os_cert_handle(), cert_from_pickle->os_cert_handle()));
  const X509Certificate::OSCertHandles& cert_intermediates =
      cert->GetIntermediateCertificates();
  const X509Certificate::OSCertHandles& pickle_intermediates =
      cert_from_pickle->GetIntermediateCertificates();
  ASSERT_EQ(cert_intermediates.size(), pickle_intermediates.size());
  for (size_t i = 0; i < cert_intermediates.size(); ++i) {
    EXPECT_TRUE(X509Certificate::IsSameOSCert(cert_intermediates[i],
                                              pickle_intermediates[i]));
  }
}

TEST(X509CertificateTest, Policy) {
  scoped_refptr<X509Certificate> google_cert(X509Certificate::CreateFromBytes(
      reinterpret_cast<const char*>(google_der), sizeof(google_der)));

  scoped_refptr<X509Certificate> webkit_cert(X509Certificate::CreateFromBytes(
      reinterpret_cast<const char*>(webkit_der), sizeof(webkit_der)));

  CertPolicy policy;

  EXPECT_EQ(policy.Check(google_cert.get()), CertPolicy::UNKNOWN);
  EXPECT_EQ(policy.Check(webkit_cert.get()), CertPolicy::UNKNOWN);
  EXPECT_FALSE(policy.HasAllowedCert());
  EXPECT_FALSE(policy.HasDeniedCert());

  policy.Allow(google_cert.get());

  EXPECT_EQ(policy.Check(google_cert.get()), CertPolicy::ALLOWED);
  EXPECT_EQ(policy.Check(webkit_cert.get()), CertPolicy::UNKNOWN);
  EXPECT_TRUE(policy.HasAllowedCert());
  EXPECT_FALSE(policy.HasDeniedCert());

  policy.Deny(google_cert.get());

  EXPECT_EQ(policy.Check(google_cert.get()), CertPolicy::DENIED);
  EXPECT_EQ(policy.Check(webkit_cert.get()), CertPolicy::UNKNOWN);
  EXPECT_FALSE(policy.HasAllowedCert());
  EXPECT_TRUE(policy.HasDeniedCert());

  policy.Allow(webkit_cert.get());

  EXPECT_EQ(policy.Check(google_cert.get()), CertPolicy::DENIED);
  EXPECT_EQ(policy.Check(webkit_cert.get()), CertPolicy::ALLOWED);
  EXPECT_TRUE(policy.HasAllowedCert());
  EXPECT_TRUE(policy.HasDeniedCert());
}

TEST(X509CertificateTest, IntermediateCertificates) {
  scoped_refptr<X509Certificate> webkit_cert(
      X509Certificate::CreateFromBytes(
          reinterpret_cast<const char*>(webkit_der), sizeof(webkit_der)));

  scoped_refptr<X509Certificate> thawte_cert(
      X509Certificate::CreateFromBytes(
          reinterpret_cast<const char*>(thawte_der), sizeof(thawte_der)));

  X509Certificate::OSCertHandle google_handle;
  // Create object with no intermediates:
  google_handle = X509Certificate::CreateOSCertHandleFromBytes(
      reinterpret_cast<const char*>(google_der), sizeof(google_der));
  X509Certificate::OSCertHandles intermediates1;
  scoped_refptr<X509Certificate> cert1;
  cert1 = X509Certificate::CreateFromHandle(google_handle, intermediates1);
  EXPECT_EQ(0u, cert1->GetIntermediateCertificates().size());

  // Create object with 2 intermediates:
  X509Certificate::OSCertHandles intermediates2;
  intermediates2.push_back(webkit_cert->os_cert_handle());
  intermediates2.push_back(thawte_cert->os_cert_handle());
  scoped_refptr<X509Certificate> cert2;
  cert2 = X509Certificate::CreateFromHandle(google_handle, intermediates2);

  // Verify it has all the intermediates:
  const X509Certificate::OSCertHandles& cert2_intermediates =
      cert2->GetIntermediateCertificates();
  ASSERT_EQ(2u, cert2_intermediates.size());
  EXPECT_TRUE(X509Certificate::IsSameOSCert(cert2_intermediates[0],
                                            webkit_cert->os_cert_handle()));
  EXPECT_TRUE(X509Certificate::IsSameOSCert(cert2_intermediates[1],
                                            thawte_cert->os_cert_handle()));

  // Cleanup
  X509Certificate::FreeOSCertHandle(google_handle);
}

#if !defined(OS_IOS)
// TODO(ios): Not yet implemented on iOS.
#if defined(OS_MACOSX)
TEST(X509CertificateTest, IsIssuedBy) {
  FilePath certs_dir = GetTestCertsDirectory();

  // Test a client certificate from MIT.
  scoped_refptr<X509Certificate> mit_davidben_cert(
      ImportCertFromFile(certs_dir, "mit.davidben.der"));
  ASSERT_NE(static_cast<X509Certificate*>(NULL), mit_davidben_cert);

  CertPrincipal mit_issuer;
  mit_issuer.country_name = "US";
  mit_issuer.state_or_province_name = "Massachusetts";
  mit_issuer.organization_names.push_back(
      "Massachusetts Institute of Technology");
  mit_issuer.organization_unit_names.push_back("Client CA v1");

  // IsIssuedBy should return true even if it cannot build a chain
  // with that principal.
  std::vector<CertPrincipal> mit_issuers(1, mit_issuer);
  EXPECT_TRUE(mit_davidben_cert->IsIssuedBy(mit_issuers));

  // Test a client certificate from FOAF.ME.
  scoped_refptr<X509Certificate> foaf_me_chromium_test_cert(
      ImportCertFromFile(certs_dir, "foaf.me.chromium-test-cert.der"));
  ASSERT_NE(static_cast<X509Certificate*>(NULL), foaf_me_chromium_test_cert);

  CertPrincipal foaf_issuer;
  foaf_issuer.common_name = "FOAF.ME";
  foaf_issuer.locality_name = "Wimbledon";
  foaf_issuer.state_or_province_name = "LONDON";
  foaf_issuer.country_name = "GB";
  foaf_issuer.organization_names.push_back("FOAF.ME");

  std::vector<CertPrincipal> foaf_issuers(1, foaf_issuer);
  EXPECT_TRUE(foaf_me_chromium_test_cert->IsIssuedBy(foaf_issuers));

  // And test some combinations and mismatches.
  std::vector<CertPrincipal> both_issuers;
  both_issuers.push_back(mit_issuer);
  both_issuers.push_back(foaf_issuer);
  EXPECT_TRUE(foaf_me_chromium_test_cert->IsIssuedBy(both_issuers));
  EXPECT_TRUE(mit_davidben_cert->IsIssuedBy(both_issuers));
  EXPECT_FALSE(foaf_me_chromium_test_cert->IsIssuedBy(mit_issuers));
  EXPECT_FALSE(mit_davidben_cert->IsIssuedBy(foaf_issuers));
}
#endif  // defined(OS_MACOSX)
#endif  // !defined(OS_IOS)

#if !defined(OS_IOS)  // TODO(ios): Unable to create certificates.
#if defined(USE_NSS) || defined(OS_WIN) || defined(OS_MACOSX)
// This test creates a self-signed cert from a private key and then verify the
// content of the certificate.
TEST(X509CertificateTest, CreateSelfSigned) {
  scoped_ptr<crypto::RSAPrivateKey> private_key(
      crypto::RSAPrivateKey::Create(1024));
  scoped_refptr<X509Certificate> cert =
      X509Certificate::CreateSelfSigned(
          private_key.get(), "CN=subject", 1, base::TimeDelta::FromDays(1));

  EXPECT_EQ("subject", cert->subject().GetDisplayName());
  EXPECT_FALSE(cert->HasExpired());

  const uint8 private_key_info[] = {
    0x30, 0x82, 0x02, 0x78, 0x02, 0x01, 0x00, 0x30,
    0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
    0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x82,
    0x02, 0x62, 0x30, 0x82, 0x02, 0x5e, 0x02, 0x01,
    0x00, 0x02, 0x81, 0x81, 0x00, 0xb8, 0x7f, 0x2b,
    0x20, 0xdc, 0x7c, 0x9b, 0x0c, 0xdc, 0x51, 0x61,
    0x99, 0x0d, 0x36, 0x0f, 0xd4, 0x66, 0x88, 0x08,
    0x55, 0x84, 0xd5, 0x3a, 0xbf, 0x2b, 0xa4, 0x64,
    0x85, 0x7b, 0x0c, 0x04, 0x13, 0x3f, 0x8d, 0xf4,
    0xbc, 0x38, 0x0d, 0x49, 0xfe, 0x6b, 0xc4, 0x5a,
    0xb0, 0x40, 0x53, 0x3a, 0xd7, 0x66, 0x09, 0x0f,
    0x9e, 0x36, 0x74, 0x30, 0xda, 0x8a, 0x31, 0x4f,
    0x1f, 0x14, 0x50, 0xd7, 0xc7, 0x20, 0x94, 0x17,
    0xde, 0x4e, 0xb9, 0x57, 0x5e, 0x7e, 0x0a, 0xe5,
    0xb2, 0x65, 0x7a, 0x89, 0x4e, 0xb6, 0x47, 0xff,
    0x1c, 0xbd, 0xb7, 0x38, 0x13, 0xaf, 0x47, 0x85,
    0x84, 0x32, 0x33, 0xf3, 0x17, 0x49, 0xbf, 0xe9,
    0x96, 0xd0, 0xd6, 0x14, 0x6f, 0x13, 0x8d, 0xc5,
    0xfc, 0x2c, 0x72, 0xba, 0xac, 0xea, 0x7e, 0x18,
    0x53, 0x56, 0xa6, 0x83, 0xa2, 0xce, 0x93, 0x93,
    0xe7, 0x1f, 0x0f, 0xe6, 0x0f, 0x02, 0x03, 0x01,
    0x00, 0x01, 0x02, 0x81, 0x80, 0x03, 0x61, 0x89,
    0x37, 0xcb, 0xf2, 0x98, 0xa0, 0xce, 0xb4, 0xcb,
    0x16, 0x13, 0xf0, 0xe6, 0xaf, 0x5c, 0xc5, 0xa7,
    0x69, 0x71, 0xca, 0xba, 0x8d, 0xe0, 0x4d, 0xdd,
    0xed, 0xb8, 0x48, 0x8b, 0x16, 0x93, 0x36, 0x95,
    0xc2, 0x91, 0x40, 0x65, 0x17, 0xbd, 0x7f, 0xd6,
    0xad, 0x9e, 0x30, 0x28, 0x46, 0xe4, 0x3e, 0xcc,
    0x43, 0x78, 0xf9, 0xfe, 0x1f, 0x33, 0x23, 0x1e,
    0x31, 0x12, 0x9d, 0x3c, 0xa7, 0x08, 0x82, 0x7b,
    0x7d, 0x25, 0x4e, 0x5e, 0x19, 0xa8, 0x9b, 0xed,
    0x86, 0xb2, 0xcb, 0x3c, 0xfe, 0x4e, 0xa1, 0xfa,
    0x62, 0x87, 0x3a, 0x17, 0xf7, 0x60, 0xec, 0x38,
    0x29, 0xe8, 0x4f, 0x34, 0x9f, 0x76, 0x9d, 0xee,
    0xa3, 0xf6, 0x85, 0x6b, 0x84, 0x43, 0xc9, 0x1e,
    0x01, 0xff, 0xfd, 0xd0, 0x29, 0x4c, 0xfa, 0x8e,
    0x57, 0x0c, 0xc0, 0x71, 0xa5, 0xbb, 0x88, 0x46,
    0x29, 0x5c, 0xc0, 0x4f, 0x01, 0x02, 0x41, 0x00,
    0xf5, 0x83, 0xa4, 0x64, 0x4a, 0xf2, 0xdd, 0x8c,
    0x2c, 0xed, 0xa8, 0xd5, 0x60, 0x5a, 0xe4, 0xc7,
    0xcc, 0x61, 0xcd, 0x38, 0x42, 0x20, 0xd3, 0x82,
    0x18, 0xf2, 0x35, 0x00, 0x72, 0x2d, 0xf7, 0x89,
    0x80, 0x67, 0xb5, 0x93, 0x05, 0x5f, 0xdd, 0x42,
    0xba, 0x16, 0x1a, 0xea, 0x15, 0xc6, 0xf0, 0xb8,
    0x8c, 0xbc, 0xbf, 0x54, 0x9e, 0xf1, 0xc1, 0xb2,
    0xb3, 0x8b, 0xb6, 0x26, 0x02, 0x30, 0xc4, 0x81,
    0x02, 0x41, 0x00, 0xc0, 0x60, 0x62, 0x80, 0xe1,
    0x22, 0x78, 0xf6, 0x9d, 0x83, 0x18, 0xeb, 0x72,
    0x45, 0xd7, 0xc8, 0x01, 0x7f, 0xa9, 0xca, 0x8f,
    0x7d, 0xd6, 0xb8, 0x31, 0x2b, 0x84, 0x7f, 0x62,
    0xd9, 0xa9, 0x22, 0x17, 0x7d, 0x06, 0x35, 0x6c,
    0xf3, 0xc1, 0x94, 0x17, 0x85, 0x5a, 0xaf, 0x9c,
    0x5c, 0x09, 0x3c, 0xcf, 0x2f, 0x44, 0x9d, 0xb6,
    0x52, 0x68, 0x5f, 0xf9, 0x59, 0xc8, 0x84, 0x2b,
    0x39, 0x22, 0x8f, 0x02, 0x41, 0x00, 0xb2, 0x04,
    0xe2, 0x0e, 0x56, 0xca, 0x03, 0x1a, 0xc0, 0xf9,
    0x12, 0x92, 0xa5, 0x6b, 0x42, 0xb8, 0x1c, 0xda,
    0x4d, 0x93, 0x9d, 0x5f, 0x6f, 0xfd, 0xc5, 0x58,
    0xda, 0x55, 0x98, 0x74, 0xfc, 0x28, 0x17, 0x93,
    0x1b, 0x75, 0x9f, 0x50, 0x03, 0x7f, 0x7e, 0xae,
    0xc8, 0x95, 0x33, 0x75, 0x2c, 0xd6, 0xa4, 0x35,
    0xb8, 0x06, 0x03, 0xba, 0x08, 0x59, 0x2b, 0x17,
    0x02, 0xdc, 0x4c, 0x7a, 0x50, 0x01, 0x02, 0x41,
    0x00, 0x9d, 0xdb, 0x39, 0x59, 0x09, 0xe4, 0x30,
    0xa0, 0x24, 0xf5, 0xdb, 0x2f, 0xf0, 0x2f, 0xf1,
    0x75, 0x74, 0x0d, 0x5e, 0xb5, 0x11, 0x73, 0xb0,
    0x0a, 0xaa, 0x86, 0x4c, 0x0d, 0xff, 0x7e, 0x1d,
    0xb4, 0x14, 0xd4, 0x09, 0x91, 0x33, 0x5a, 0xfd,
    0xa0, 0x58, 0x80, 0x9b, 0xbe, 0x78, 0x2e, 0x69,
    0x82, 0x15, 0x7c, 0x72, 0xf0, 0x7b, 0x18, 0x39,
    0xff, 0x6e, 0xeb, 0xc6, 0x86, 0xf5, 0xb4, 0xc7,
    0x6f, 0x02, 0x41, 0x00, 0x8d, 0x1a, 0x37, 0x0f,
    0x76, 0xc4, 0x82, 0xfa, 0x5c, 0xc3, 0x79, 0x35,
    0x3e, 0x70, 0x8a, 0xbf, 0x27, 0x49, 0xb0, 0x99,
    0x63, 0xcb, 0x77, 0x5f, 0xa8, 0x82, 0x65, 0xf6,
    0x03, 0x52, 0x51, 0xf1, 0xae, 0x2e, 0x05, 0xb3,
    0xc6, 0xa4, 0x92, 0xd1, 0xce, 0x6c, 0x72, 0xfb,
    0x21, 0xb3, 0x02, 0x87, 0xe4, 0xfd, 0x61, 0xca,
    0x00, 0x42, 0x19, 0xf0, 0xda, 0x5a, 0x53, 0xe3,
    0xb1, 0xc5, 0x15, 0xf3
  };

  std::vector<uint8> input;
  input.resize(sizeof(private_key_info));
  memcpy(&input.front(), private_key_info, sizeof(private_key_info));

  private_key.reset(crypto::RSAPrivateKey::CreateFromPrivateKeyInfo(input));
  ASSERT_TRUE(private_key.get());

  cert = X509Certificate::CreateSelfSigned(
      private_key.get(), "CN=subject", 1, base::TimeDelta::FromDays(1));

  EXPECT_EQ("subject", cert->subject().GetDisplayName());
  EXPECT_FALSE(cert->HasExpired());
}

TEST(X509CertificateTest, GetDEREncoded) {
  scoped_ptr<crypto::RSAPrivateKey> private_key(
      crypto::RSAPrivateKey::Create(1024));
  scoped_refptr<X509Certificate> cert =
      X509Certificate::CreateSelfSigned(
          private_key.get(), "CN=subject", 0, base::TimeDelta::FromDays(1));

  std::string der_cert;
  EXPECT_TRUE(X509Certificate::GetDEREncoded(cert->os_cert_handle(),
                                             &der_cert));
  EXPECT_FALSE(der_cert.empty());
}
#endif
#endif  // !defined(OS_IOS)

class X509CertificateParseTest
    : public testing::TestWithParam<CertificateFormatTestData> {
 public:
  virtual ~X509CertificateParseTest() {}
  virtual void SetUp() {
    test_data_ = GetParam();
  }
  virtual void TearDown() {}

 protected:
  CertificateFormatTestData test_data_;
};

TEST_P(X509CertificateParseTest, CanParseFormat) {
  FilePath certs_dir = GetTestCertsDirectory();
  CertificateList certs = CreateCertificateListFromFile(
      certs_dir, test_data_.file_name, test_data_.format);
  ASSERT_FALSE(certs.empty());
  ASSERT_LE(certs.size(), arraysize(test_data_.chain_fingerprints));
  CheckGoogleCert(certs.front(), google_parse_fingerprint,
                  kGoogleParseValidFrom, kGoogleParseValidTo);

  size_t i;
  for (i = 0; i < arraysize(test_data_.chain_fingerprints); ++i) {
    if (test_data_.chain_fingerprints[i] == NULL) {
      // No more test certificates expected - make sure no more were
      // returned before marking this test a success.
      EXPECT_EQ(i, certs.size());
      break;
    }

    // A cert is expected - make sure that one was parsed.
    ASSERT_LT(i, certs.size());

    // Compare the parsed certificate with the expected certificate, by
    // comparing fingerprints.
    const X509Certificate* cert = certs[i];
    const SHA1HashValue& actual_fingerprint = cert->fingerprint();
    uint8* expected_fingerprint = test_data_.chain_fingerprints[i];

    for (size_t j = 0; j < 20; ++j)
      EXPECT_EQ(expected_fingerprint[j], actual_fingerprint.data[j]);
  }
}

INSTANTIATE_TEST_CASE_P(, X509CertificateParseTest,
                        testing::ValuesIn(FormatTestData));

struct CertificateNameVerifyTestData {
  // true iff we expect hostname to match an entry in cert_names.
  bool expected;
  // The hostname to match.
  const char* hostname;
  // Common name, may be used if |dns_names| or |ip_addrs| are empty.
  const char* common_name;
  // Comma separated list of certificate names to match against. Any occurrence
  // of '#' will be replaced with a null character before processing.
  const char* dns_names;
  // Comma separated list of certificate IP Addresses to match against. Each
  // address is x prefixed 16 byte hex code for v6 or dotted-decimals for v4.
  const char* ip_addrs;
};

// GTest 'magic' pretty-printer, so that if/when a test fails, it knows how
// to output the parameter that was passed. Without this, it will simply
// attempt to print out the first twenty bytes of the object, which depending
// on platform and alignment, may result in an invalid read.
void PrintTo(const CertificateNameVerifyTestData& data, std::ostream* os) {
  ASSERT_TRUE(data.hostname && data.common_name);
  // Using StringPiece to allow for optional fields being NULL.
  *os << " expected: " << data.expected
      << "; hostname: " << data.hostname
      << "; common_name: " << data.common_name
      << "; dns_names: " << base::StringPiece(data.dns_names)
      << "; ip_addrs: " << base::StringPiece(data.ip_addrs);
}

const CertificateNameVerifyTestData kNameVerifyTestData[] = {
    { true, "foo.com", "foo.com" },
    { true, "f", "f" },
    { false, "h", "i" },
    { true, "bar.foo.com", "*.foo.com" },
    { true, "www.test.fr", "common.name",
        "*.test.com,*.test.co.uk,*.test.de,*.test.fr" },
    { true, "wwW.tESt.fr",  "common.name",
        ",*.*,*.test.de,*.test.FR,www" },
    { false, "f.uk", ".uk" },
    { false, "w.bar.foo.com", "?.bar.foo.com" },
    { false, "www.foo.com", "(www|ftp).foo.com" },
    { false, "www.foo.com", "www.foo.com#" },  // # = null char.
    { false, "www.foo.com", "", "www.foo.com#*.foo.com,#,#" },
    { false, "www.house.example", "ww.house.example" },
    { false, "test.org", "", "www.test.org,*.test.org,*.org" },
    { false, "w.bar.foo.com", "w*.bar.foo.com" },
    { false, "www.bar.foo.com", "ww*ww.bar.foo.com" },
    { false, "wwww.bar.foo.com", "ww*ww.bar.foo.com" },
    { true, "wwww.bar.foo.com", "w*w.bar.foo.com" },
    { false, "wwww.bar.foo.com", "w*w.bar.foo.c0m" },
    { true, "WALLY.bar.foo.com", "wa*.bar.foo.com" },
    { true, "wally.bar.foo.com", "*Ly.bar.foo.com" },
    { true, "ww%57.foo.com", "", "www.foo.com" },
    { true, "www&.foo.com", "www%26.foo.com" },
    // Common name must not be used if subject alternative name was provided.
    { false, "www.test.co.jp",  "www.test.co.jp",
        "*.test.de,*.jp,www.test.co.uk,www.*.co.jp" },
    { false, "www.bar.foo.com", "www.bar.foo.com",
      "*.foo.com,*.*.foo.com,*.*.bar.foo.com,*..bar.foo.com," },
    { false, "www.bath.org", "www.bath.org", "", "20.30.40.50" },
    { false, "66.77.88.99", "www.bath.org", "www.bath.org" },
    // IDN tests
    { true, "xn--poema-9qae5a.com.br", "xn--poema-9qae5a.com.br" },
    { true, "www.xn--poema-9qae5a.com.br", "*.xn--poema-9qae5a.com.br" },
    { false, "xn--poema-9qae5a.com.br", "", "*.xn--poema-9qae5a.com.br,"
                                            "xn--poema-*.com.br,"
                                            "xn--*-9qae5a.com.br,"
                                            "*--poema-9qae5a.com.br" },
    { true, "xn--poema-9qae5a.com.br", "*.com.br" },
    // The following are adapted from the  examples quoted from
    // http://tools.ietf.org/html/rfc6125#section-6.4.3
    //  (e.g., *.example.com would match foo.example.com but
    //   not bar.foo.example.com or example.com).
    { true, "foo.example.com", "*.example.com" },
    { false, "bar.foo.example.com", "*.example.com" },
    { false, "example.com", "*.example.com" },
    //   (e.g., baz*.example.net and *baz.example.net and b*z.example.net would
    //   be taken to match baz1.example.net and foobaz.example.net and
    //   buzz.example.net, respectively
    { true, "baz1.example.net", "baz*.example.net" },
    { true, "foobaz.example.net", "*baz.example.net" },
    { true, "buzz.example.net", "b*z.example.net" },
    // Wildcards should not be valid unless there are at least three name
    // components.
    { true,  "h.co.uk", "*.co.uk" },
    { false, "foo.com", "*.com" },
    { false, "foo.us", "*.us" },
    { false, "foo", "*" },
    // Multiple wildcards are not valid.
    { false, "foo.example.com", "*.*.com" },
    { false, "foo.bar.example.com", "*.bar.*.com" },
    // Absolute vs relative DNS name tests. Although not explicitly specified
    // in RFC 6125, absolute reference names (those ending in a .) should
    // match either absolute or relative presented names.
    { true, "foo.com", "foo.com." },
    { true, "foo.com.", "foo.com" },
    { true, "foo.com.", "foo.com." },
    { true, "f", "f." },
    { true, "f.", "f" },
    { true, "f.", "f." },
    { true, "www-3.bar.foo.com", "*.bar.foo.com." },
    { true, "www-3.bar.foo.com.", "*.bar.foo.com" },
    { true, "www-3.bar.foo.com.", "*.bar.foo.com." },
    { false, ".", "." },
    { false, "example.com", "*.com." },
    { false, "example.com.", "*.com" },
    { false, "example.com.", "*.com." },
    { false, "foo.", "*." },
    // IP addresses in common name; IPv4 only.
    { true, "127.0.0.1", "127.0.0.1" },
    { true, "192.168.1.1", "192.168.1.1" },
    { true,  "676768", "0.10.83.160" },
    { true,  "1.2.3", "1.2.0.3" },
    { false, "192.169.1.1", "192.168.1.1" },
    { false, "12.19.1.1", "12.19.1.1/255.255.255.0" },
    { false, "FEDC:ba98:7654:3210:FEDC:BA98:7654:3210",
      "FEDC:BA98:7654:3210:FEDC:ba98:7654:3210" },
    { false, "1111:2222:3333:4444:5555:6666:7777:8888",
      "1111:2222:3333:4444:5555:6666:7777:8888" },
    { false, "::192.9.5.5", "[::192.9.5.5]" },
    // No wildcard matching in valid IP addresses
    { false, "::192.9.5.5", "*.9.5.5" },
    { false, "2010:836B:4179::836B:4179", "*:836B:4179::836B:4179" },
    { false, "192.168.1.11", "*.168.1.11" },
    { false, "FEDC:BA98:7654:3210:FEDC:BA98:7654:3210", "*.]" },
    // IP addresses in subject alternative name (common name ignored)
    { true, "10.1.2.3", "", "", "10.1.2.3" },
    { true,  "14.15", "", "", "14.0.0.15" },
    { false, "10.1.2.7", "10.1.2.7", "", "10.1.2.6,10.1.2.8" },
    { false, "10.1.2.8", "10.20.2.8", "foo" },
    { true, "::4.5.6.7", "", "", "x00000000000000000000000004050607" },
    { false, "::6.7.8.9", "::6.7.8.9", "::6.7.8.9",
        "x00000000000000000000000006070808,x0000000000000000000000000607080a,"
        "xff000000000000000000000006070809,6.7.8.9" },
    { true, "FE80::200:f8ff:fe21:67cf", "no.common.name", "",
        "x00000000000000000000000006070808,xfe800000000000000200f8fffe2167cf,"
        "xff0000000000000000000000060708ff,10.0.0.1" },
    // Numeric only hostnames (none of these are considered valid IP addresses).
    { false,  "12345.6", "12345.6" },
    { false, "121.2.3.512", "", "1*1.2.3.512,*1.2.3.512,1*.2.3.512,*.2.3.512",
        "121.2.3.0"},
    { false, "1.2.3.4.5.6", "*.2.3.4.5.6" },
    { true, "1.2.3.4.5", "", "1.2.3.4.5" },
    // Invalid host names.
    { false, "junk)(£)$*!@~#", "junk)(£)$*!@~#" },
    { false, "www.*.com", "www.*.com" },
    { false, "w$w.f.com", "w$w.f.com" },
    { false, "nocolonallowed:example", "", "nocolonallowed:example" },
    { false, "www-1.[::FFFF:129.144.52.38]", "*.[::FFFF:129.144.52.38]" },
    { false, "[::4.5.6.9]", "", "", "x00000000000000000000000004050609" },
};

class X509CertificateNameVerifyTest
    : public testing::TestWithParam<CertificateNameVerifyTestData> {
};

TEST_P(X509CertificateNameVerifyTest, VerifyHostname) {
  CertificateNameVerifyTestData test_data = GetParam();

  std::string common_name(test_data.common_name);
  ASSERT_EQ(std::string::npos, common_name.find(','));
  std::replace(common_name.begin(), common_name.end(), '#', '\0');

  std::vector<std::string> dns_names, ip_addressses;
  if (test_data.dns_names) {
    // Build up the certificate DNS names list.
    std::string dns_name_line(test_data.dns_names);
    std::replace(dns_name_line.begin(), dns_name_line.end(), '#', '\0');
    base::SplitString(dns_name_line, ',', &dns_names);
  }

  if (test_data.ip_addrs) {
    // Build up the certificate IP address list.
    std::string ip_addrs_line(test_data.ip_addrs);
    std::vector<std::string> ip_addressses_ascii;
    base::SplitString(ip_addrs_line, ',', &ip_addressses_ascii);
    for (size_t i = 0; i < ip_addressses_ascii.size(); ++i) {
      std::string& addr_ascii = ip_addressses_ascii[i];
      ASSERT_NE(0U, addr_ascii.length());
      if (addr_ascii[0] == 'x') {  // Hex encoded address
        addr_ascii.erase(0, 1);
        std::vector<uint8> bytes;
        EXPECT_TRUE(base::HexStringToBytes(addr_ascii, &bytes))
            << "Could not parse hex address " << addr_ascii << " i = " << i;
        ip_addressses.push_back(std::string(reinterpret_cast<char*>(&bytes[0]),
                                            bytes.size()));
        ASSERT_EQ(16U, ip_addressses.back().size()) << i;
      } else {  // Decimal groups
        std::vector<std::string> decimals_ascii;
        base::SplitString(addr_ascii, '.', &decimals_ascii);
        EXPECT_EQ(4U, decimals_ascii.size()) << i;
        std::string addr_bytes;
        for (size_t j = 0; j < decimals_ascii.size(); ++j) {
          int decimal_value;
          EXPECT_TRUE(base::StringToInt(decimals_ascii[j], &decimal_value));
          EXPECT_GE(decimal_value, 0);
          EXPECT_LE(decimal_value, 255);
          addr_bytes.push_back(static_cast<char>(decimal_value));
        }
        ip_addressses.push_back(addr_bytes);
        ASSERT_EQ(4U, ip_addressses.back().size()) << i;
      }
    }
  }

  EXPECT_EQ(test_data.expected, X509Certificate::VerifyHostname(
      test_data.hostname, common_name, dns_names, ip_addressses));
}

INSTANTIATE_TEST_CASE_P(, X509CertificateNameVerifyTest,
                        testing::ValuesIn(kNameVerifyTestData));

}  // namespace net
