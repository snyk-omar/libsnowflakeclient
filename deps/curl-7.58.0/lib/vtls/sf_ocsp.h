/*
* Copyright (c) 2017-2018 Snowflake Computing
*/

#ifndef HEADER_CURL_SF_OCSP_H
#define HEADER_CURL_SF_OCSP_H

#include "urldata.h"
#include <openssl/ssl.h>

CURLcode checkCertOCSP(struct connectdata *conn,
                       STACK_OF(X509) *ch, X509_STORE *st);

#endif