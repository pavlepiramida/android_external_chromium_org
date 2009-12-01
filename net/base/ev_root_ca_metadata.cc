// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/base/ev_root_ca_metadata.h"

#if defined(OS_LINUX)
#include <cert.h>
#include <pkcs11n.h>
#include <secerr.h>
#include <secoid.h>
#endif

#include "base/logging.h"
#include "base/singleton.h"

namespace net {

// Raw metadata.
struct EVMetadata {
  // The SHA-1 fingerprint of the root CA certificate, used as a unique
  // identifier for a root CA certificate.
  X509Certificate::Fingerprint fingerprint;

  // The EV policy OID of the root CA.
  // Note: a root CA may have multiple EV policies.  When that actually
  // happens, we'll need to support that.
  const char* policy_oid;
};

static const EVMetadata ev_root_ca_metadata[] = {
  // AddTrust External CA Root
  // https://addtrustexternalcaroot-ev.comodoca.com
  { { { 0x02, 0xfa, 0xf3, 0xe2, 0x91, 0x43, 0x54, 0x68, 0x60, 0x78,
        0x57, 0x69, 0x4d, 0xf5, 0xe4, 0x5b, 0x68, 0x85, 0x18, 0x68 } },
    "1.3.6.1.4.1.6449.1.2.1.5.1"
  },
  // COMODO Certification Authority
  // https://secure.comodo.com/
  { { { 0x66, 0x31, 0xbf, 0x9e, 0xf7, 0x4f, 0x9e, 0xb6, 0xc9, 0xd5,
        0xa6, 0x0c, 0xba, 0x6a, 0xbe, 0xd1, 0xf7, 0xbd, 0xef, 0x7b } },
    "1.3.6.1.4.1.6449.1.2.1.5.1"
  },
  // COMODO ECC Certification Authority
  // https://comodoecccertificationauthority-ev.comodoca.com/
  { { { 0x9f, 0x74, 0x4e, 0x9f, 0x2b, 0x4d, 0xba, 0xec, 0x0f, 0x31,
        0x2c, 0x50, 0xb6, 0x56, 0x3b, 0x8e, 0x2d, 0x93, 0xc3, 0x11 } },
    "1.3.6.1.4.1.6449.1.2.1.5.1"
  },
  // Cybertrust Global Root
  // https://evup.cybertrust.ne.jp/ctj-ev-upgrader/evseal.gif
  { { { 0x5f, 0x43, 0xe5, 0xb1, 0xbf, 0xf8, 0x78, 0x8c, 0xac, 0x1c,
        0xc7, 0xca, 0x4a, 0x9a, 0xc6, 0x22, 0x2b, 0xcc, 0x34, 0xc6 } },
    "1.3.6.1.4.1.6334.1.100.1"
  },
  // DigiCert High Assurance EV Root CA
  // https://www.digicert.com
  { { { 0x5f, 0xb7, 0xee, 0x06, 0x33, 0xe2, 0x59, 0xdb, 0xad, 0x0c,
        0x4c, 0x9a, 0xe6, 0xd3, 0x8f, 0x1a, 0x61, 0xc7, 0xdc, 0x25 } },
    "2.16.840.1.114412.2.1"
  },
  // DigiNotar Root CA
  // https://www.evssl.nl
  // https://www.polisdirect.nl
  { { { 0xc0, 0x60, 0xed, 0x44, 0xcb, 0xd8, 0x81, 0xbd, 0x0e, 0xf8,
        0x6c, 0x0b, 0xa2, 0x87, 0xdd, 0xcf, 0x81, 0x67, 0x47, 0x8c } },
    "2.16.528.1.1001.1.1.1.12.6.1.1.1"
  },
  // Entrust.net Secure Server Certification Authority
  // https://www.entrust.net/
  { { { 0x99, 0xa6, 0x9b, 0xe6, 0x1a, 0xfe, 0x88, 0x6b, 0x4d, 0x2b,
        0x82, 0x00, 0x7c, 0xb8, 0x54, 0xfc, 0x31, 0x7e, 0x15, 0x39 } },
    "2.16.840.1.114028.10.1.2"
  },
  // Entrust Root Certification Authority
  // https://www.entrust.net/
  { { { 0xb3, 0x1e, 0xb1, 0xb7, 0x40, 0xe3, 0x6c, 0x84, 0x02, 0xda,
        0xdc, 0x37, 0xd4, 0x4d, 0xf5, 0xd4, 0x67, 0x49, 0x52, 0xf9 } },
    "2.16.840.1.114028.10.1.2"
  },
  // Equifax Secure Certificate Authority (GeoTrust)
  // https://www.geotrust.com/
  { { { 0xd2, 0x32, 0x09, 0xad, 0x23, 0xd3, 0x14, 0x23, 0x21, 0x74,
        0xe4, 0x0d, 0x7f, 0x9d, 0x62, 0x13, 0x97, 0x86, 0x63, 0x3a } },
    "1.3.6.1.4.1.14370.1.6"
  },
  // GeoTrust Primary Certification Authority
  // https://www.geotrust.com/
  { { { 0x32, 0x3c, 0x11, 0x8e, 0x1b, 0xf7, 0xb8, 0xb6, 0x52, 0x54,
        0xe2, 0xe2, 0x10, 0x0d, 0xd6, 0x02, 0x90, 0x37, 0xf0, 0x96 } },
    "1.3.6.1.4.1.14370.1.6"
  },
  // GlobalSign
  // https://www.globalsign.com/
  { { { 0x75, 0xe0, 0xab, 0xb6, 0x13, 0x85, 0x12, 0x27, 0x1c, 0x04,
        0xf8, 0x5f, 0xdd, 0xde, 0x38, 0xe4, 0xb7, 0x24, 0x2e, 0xfe } },
    "1.3.6.1.4.1.4146.1.1"
  },
  // GlobalSign Root CA
  { { { 0xb1, 0xbc, 0x96, 0x8b, 0xd4, 0xf4, 0x9d, 0x62, 0x2a, 0xa8,
        0x9a, 0x81, 0xf2, 0x15, 0x01, 0x52, 0xa4, 0x1d, 0x82, 0x9c } },
    "1.3.6.1.4.1.4146.1.1"
  },
  // Go Daddy Class 2 Certification Authority
  // https://www.godaddy.com/
  { { { 0x27, 0x96, 0xba, 0xe6, 0x3f, 0x18, 0x01, 0xe2, 0x77, 0x26,
        0x1b, 0xa0, 0xd7, 0x77, 0x70, 0x02, 0x8f, 0x20, 0xee, 0xe4 } },
    "2.16.840.1.114413.1.7.23.3"
  },
  // GTE CyberTrust Global Root
  // https://www.cybertrust.ne.jp/
  { { { 0x97, 0x81, 0x79, 0x50, 0xd8, 0x1c, 0x96, 0x70, 0xcc, 0x34,
        0xd8, 0x09, 0xcf, 0x79, 0x44, 0x31, 0x36, 0x7e, 0xf4, 0x74 } },
    "1.3.6.1.4.1.6334.1.100.1"
  },
  //  Network Solutions Certificate Authority
  //  https://www.networksolutions.com/website-packages/index.jsp
  { { { 0x74, 0xf8, 0xa3, 0xc3, 0xef, 0xe7, 0xb3, 0x90, 0x06, 0x4b,
        0x83, 0x90, 0x3c, 0x21, 0x64, 0x60, 0x20, 0xe5, 0xdf, 0xce } },
    "1.3.6.1.4.1.782.1.2.1.8.1"
  },
  // QuoVadis Root CA 2
  // https://www.quovadis.bm/
  { { { 0xca, 0x3a, 0xfb, 0xcf, 0x12, 0x40, 0x36, 0x4b, 0x44, 0xb2,
        0x16, 0x20, 0x88, 0x80, 0x48, 0x39, 0x19, 0x93, 0x7c, 0xf7 } },
    "1.3.6.1.4.1.8024.0.2.100.1.2"
  },
  // SecureTrust CA, SecureTrust Corporation
  // https://www.securetrust.com
  // https://www.trustwave.com/
  { { { 0x87, 0x82, 0xc6, 0xc3, 0x04, 0x35, 0x3b, 0xcf, 0xd2, 0x96,
        0x92, 0xd2, 0x59, 0x3e, 0x7d, 0x44, 0xd9, 0x34, 0xff, 0x11 } },
    "2.16.840.1.114404.1.1.2.4.1"
  },
  // Secure Global CA, SecureTrust Corporation
  { { { 0x3a, 0x44, 0x73, 0x5a, 0xe5, 0x81, 0x90, 0x1f, 0x24, 0x86,
        0x61, 0x46, 0x1e, 0x3b, 0x9c, 0xc4, 0x5f, 0xf5, 0x3a, 0x1b } },
    "2.16.840.1.114404.1.1.2.4.1"
  },
  // Security Communication RootCA1
  // https://www.secomtrust.net/contact/form.html
  { { { 0x36, 0xb1, 0x2b, 0x49, 0xf9, 0x81, 0x9e, 0xd7, 0x4c, 0x9e,
        0xbc, 0x38, 0x0f, 0xc6, 0x56, 0x8f, 0x5d, 0xac, 0xb2, 0xf7 } },
    "1.2.392.200091.100.721.1"
  },
  // Security Communication EV RootCA1
  // https://www.secomtrust.net/contact/form.html
  { { { 0xfe, 0xb8, 0xc4, 0x32, 0xdc, 0xf9, 0x76, 0x9a, 0xce, 0xae,
        0x3d, 0xd8, 0x90, 0x8f, 0xfd, 0x28, 0x86, 0x65, 0x64, 0x7d } },
    "1.2.392.200091.100.721.1"
  },
  // Starfield Class 2 Certification Authority
  // https://www.starfieldtech.com/
  { { { 0xad, 0x7e, 0x1c, 0x28, 0xb0, 0x64, 0xef, 0x8f, 0x60, 0x03,
        0x40, 0x20, 0x14, 0xc3, 0xd0, 0xe3, 0x37, 0x0e, 0xb5, 0x8a } },
    "2.16.840.1.114414.1.7.23.3"
  },
  // SwissSign Gold CA - G2
  // https://testevg2.swisssign.net/
  { { { 0xd8, 0xc5, 0x38, 0x8a, 0xb7, 0x30, 0x1b, 0x1b, 0x6e, 0xd4,
        0x7a, 0xe6, 0x45, 0x25, 0x3a, 0x6f, 0x9f, 0x1a, 0x27, 0x61 } },
    "2.16.756.1.89.1.2.1.1"
  },
  // Thawte Premium Server CA
  // https://www.thawte.com/
  { { { 0x62, 0x7f, 0x8d, 0x78, 0x27, 0x65, 0x63, 0x99, 0xd2, 0x7d,
        0x7f, 0x90, 0x44, 0xc9, 0xfe, 0xb3, 0xf3, 0x3e, 0xfa, 0x9a } },
    "2.16.840.1.113733.1.7.48.1"
  },
  // thawte Primary Root CA
  // https://www.thawte.com/
  { { { 0x91, 0xc6, 0xd6, 0xee, 0x3e, 0x8a, 0xc8, 0x63, 0x84, 0xe5,
        0x48, 0xc2, 0x99, 0x29, 0x5c, 0x75, 0x6c, 0x81, 0x7b, 0x81 } },
    "2.16.840.1.113733.1.7.48.1"
  },
  // UTN - DATACorp SGC
  { { { 0x58, 0x11, 0x9f, 0x0e, 0x12, 0x82, 0x87, 0xea, 0x50, 0xfd,
        0xd9, 0x87, 0x45, 0x6f, 0x4f, 0x78, 0xdc, 0xfa, 0xd6, 0xd4 } },
    "1.3.6.1.4.1.6449.1.2.1.5.1"
  },
  // UTN-USERFirst-Hardware
  { { { 0x04, 0x83, 0xed, 0x33, 0x99, 0xac, 0x36, 0x08, 0x05, 0x87,
        0x22, 0xed, 0xbc, 0x5e, 0x46, 0x00, 0xe3, 0xbe, 0xf9, 0xd7 } },
    "1.3.6.1.4.1.6449.1.2.1.5.1"
  },
  // ValiCert Class 2 Policy Validation Authority
  // TODO(wtc): bug 1165107: this CA has another policy OID
  // "2.16.840.1.114414.1.7.23.3".
  { { { 0x31, 0x7a, 0x2a, 0xd0, 0x7f, 0x2b, 0x33, 0x5e, 0xf5, 0xa1,
        0xc3, 0x4e, 0x4b, 0x57, 0xe8, 0xb7, 0xd8, 0xf1, 0xfc, 0xa6 } },
    "2.16.840.1.114413.1.7.23.3"
  },
  // VeriSign Class 3 Public Primary Certification Authority
  // https://www.verisign.com/
  { { { 0x74, 0x2c, 0x31, 0x92, 0xe6, 0x07, 0xe4, 0x24, 0xeb, 0x45,
        0x49, 0x54, 0x2b, 0xe1, 0xbb, 0xc5, 0x3e, 0x61, 0x74, 0xe2 } },
    "2.16.840.1.113733.1.7.23.6"
  },
  // VeriSign Class 3 Public Primary Certification Authority - G5
  // https://www.verisign.com/
  { { { 0x4e, 0xb6, 0xd5, 0x78, 0x49, 0x9b, 0x1c, 0xcf, 0x5f, 0x58,
        0x1e, 0xad, 0x56, 0xbe, 0x3d, 0x9b, 0x67, 0x44, 0xa5, 0xe5 } },
    "2.16.840.1.113733.1.7.23.6"
  },
  // Wells Fargo WellsSecure Public Root Certificate Authority
  // https://nerys.wellsfargo.com/test.html
  { { { 0xe7, 0xb4, 0xf6, 0x9d, 0x61, 0xec, 0x90, 0x69, 0xdb, 0x7e,
        0x90, 0xa7, 0x40, 0x1a, 0x3c, 0xf4, 0x7d, 0x4f, 0xe8, 0xee } },
    "2.16.840.1.114171.500.9"
  },
  // XRamp Global Certification Authority
  { { { 0xb8, 0x01, 0x86, 0xd1, 0xeb, 0x9c, 0x86, 0xa5, 0x41, 0x04,
        0xcf, 0x30, 0x54, 0xf3, 0x4c, 0x52, 0xb7, 0xe5, 0x58, 0xc6 } },
    "2.16.840.1.114404.1.1.2.4.1"
  }
};

// static
EVRootCAMetadata* EVRootCAMetadata::GetInstance() {
  return Singleton<EVRootCAMetadata>::get();
}

bool EVRootCAMetadata::GetPolicyOID(
    const X509Certificate::Fingerprint& fingerprint,
    PolicyOID* policy_oid) const {
  PolicyOidMap::const_iterator iter = ev_policy_.find(fingerprint);
  if (iter == ev_policy_.end())
    return false;
  *policy_oid = iter->second;
  return true;
}

EVRootCAMetadata::EVRootCAMetadata() {
  // Constructs the object from the raw metadata in ev_root_ca_metadata.
#if defined(OS_LINUX)
  for (size_t i = 0; i < arraysize(ev_root_ca_metadata); i++) {
    const EVMetadata& metadata = ev_root_ca_metadata[i];
    PRUint8 buf[1024];
    SECItem oid_item;
    oid_item.data = buf;
    oid_item.len = sizeof(buf);
    SECStatus status = SEC_StringToOID(NULL, &oid_item, metadata.policy_oid, 0);
    if (status != SECSuccess) {
      LOG(ERROR) << "Failed to convert to OID: " << metadata.policy_oid;
      continue;
    }
    // Register the OID.
    SECOidData od;
    od.oid.len = oid_item.len;
    od.oid.data = oid_item.data;
    od.offset = SEC_OID_UNKNOWN;
    od.desc = metadata.policy_oid;
    od.mechanism = CKM_INVALID_MECHANISM;
    od.supportedExtension = INVALID_CERT_EXTENSION;
    SECOidTag policy = SECOID_AddEntry(&od);
    DCHECK(policy != SEC_OID_UNKNOWN);
    ev_policy_[metadata.fingerprint] = policy;
    policy_oids_.push_back(policy);
  }
#else
  for (size_t i = 0; i < arraysize(ev_root_ca_metadata); i++) {
    const EVMetadata& metadata = ev_root_ca_metadata[i];
    ev_policy_[metadata.fingerprint] = metadata.policy_oid;
    // Multiple root CA certs may use the same EV policy OID.  Having
    // duplicates in the policy_oids_ array does no harm, so we don't
    // bother detecting duplicates.
    policy_oids_.push_back(metadata.policy_oid);
  }
#endif
}

}  // namespace net
