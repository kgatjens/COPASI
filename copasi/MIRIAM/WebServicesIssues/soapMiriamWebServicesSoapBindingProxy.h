// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/MIRIAM/WebServicesIssues/soapMiriamWebServicesSoapBindingProxy.h,v $
//   $Revision: 1.3 $
//   $Name:  $
//   $Author: aruff $
//   $Date: 2008/09/17 17:41:51 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

/* soapMiriamWebServicesSoapBindingProxy.h
   Generated by gSOAP 2.7.11 from miriam.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
 */

#ifndef soapMiriamWebServicesSoapBindingProxy_H
#define soapMiriamWebServicesSoapBindingProxy_H
#include "soapH.h"

class SOAP_CMAC MiriamWebServicesSoapBindingProxy : public soap
  {
  public:
    /// Endpoint URL of service 'MiriamWebServicesSoapBindingProxy' (change as needed)
    const char *soap_endpoint;
    /// Constructor
    MiriamWebServicesSoapBindingProxy();
    /// Constructor with copy of another engine state
    MiriamWebServicesSoapBindingProxy(const struct soap&);
    /// Constructor with engine input+output mode control
    MiriamWebServicesSoapBindingProxy(soap_mode iomode);
    /// Constructor with engine input and output mode control
    MiriamWebServicesSoapBindingProxy(soap_mode imode, soap_mode omode);
    /// Destructor frees deserialized data
    virtual ~MiriamWebServicesSoapBindingProxy();
    /// Initializer used by constructor
    virtual void MiriamWebServicesSoapBindingProxy_init(soap_mode imode, soap_mode omode);
    /// Disables and removes SOAP Header from message
    virtual void soap_noheader();
    /// Get SOAP Fault structure (NULL when absent)
    virtual const SOAP_ENV__Fault *soap_fault();
    /// Get SOAP Fault string (NULL when absent)
    virtual const char *soap_fault_string();
    /// Get SOAP Fault detail as string (NULL when absent)
    virtual const char *soap_fault_detail();
    /// Force close connection (normally automatic, except for send_X ops)
    virtual int soap_close_socket();
    /// Print fault
    virtual void soap_print_fault(FILE*);
#ifndef WITH_LEAN
    /// Print fault to stream
    virtual void soap_stream_fault(std::ostream&);
    /// Put fault into buffer
    virtual char *soap_sprint_fault(char *buf, size_t len);
#endif

    /// Web service operation 'getName' (returns error code or SOAP_OK)
    virtual int getName(char *_uri, char *&_getNameReturn);

    /// Web service operation 'getLocation' (returns error code or SOAP_OK)
    virtual int getLocation(char *_uri, char *_resource, char *&_getLocationReturn);

    /// Web service operation 'getURL' (returns error code or SOAP_OK)
    virtual int getURL(char *_name, char *_id, char *&_getURLReturn);

    /// Web service operation 'getURI' (returns error code or SOAP_OK)
    virtual int getURI(char *_name, char *_id, char *_type, char *&_getURIReturn);

    /// Web service operation 'getURI' (returns error code or SOAP_OK)
    virtual int getURI_(char *_name, char *_id, char *_type, char *&_getURIReturn);

    /// Web service operation 'getServicesInfo' (returns error code or SOAP_OK)
    virtual int getServicesInfo(char *&getServicesInfoReturn);

    /// Web service operation 'getServicesVersion' (returns error code or SOAP_OK)
    virtual int getServicesVersion(char *&getServicesVersionReturn);

    /// Web service operation 'getJavaLibraryVersion' (returns error code or SOAP_OK)
    virtual int getJavaLibraryVersion(char *&getJavaLibraryVersionReturn);

    /// Web service operation 'getDataTypeURN' (returns error code or SOAP_OK)
    virtual int getDataTypeURN(char *_name, char *&_getDataTypeURNReturn);

    /// Web service operation 'getDataTypeURNs' (returns error code or SOAP_OK)
    virtual int getDataTypeURNs(char *_name, struct ns2__getDataTypeURNsResponse &_param_1);

    /// Web service operation 'getDataTypeURL' (returns error code or SOAP_OK)
    virtual int getDataTypeURL(char *_name, char *&_getDataTypeURLReturn);

    /// Web service operation 'getDataTypeURLs' (returns error code or SOAP_OK)
    virtual int getDataTypeURLs(char *_name, struct ns2__getDataTypeURLsResponse &_param_2);

    /// Web service operation 'getDataTypeURI' (returns error code or SOAP_OK)
    virtual int getDataTypeURI(char *_name, char *_type, char *&_getDataTypeURIReturn);

    /// Web service operation 'getDataTypeURI' (returns error code or SOAP_OK)
    virtual int getDataTypeURI_(char *_name, char *_type, char *&_getDataTypeURIReturn);

    /// Web service operation 'getDataTypeURIs' (returns error code or SOAP_OK)
    virtual int getDataTypeURIs(char *_name, char *_type, struct ns2__getDataTypeURIsResponse &_param_3);

    /// Web service operation 'getDataTypeURIs' (returns error code or SOAP_OK)
    virtual int getDataTypeURIs_(char *_name, char *_type, struct ns2__getDataTypeURIsResponse_ &_param_4);

    /// Web service operation 'getDataTypeAllURIs' (returns error code or SOAP_OK)
    virtual int getDataTypeAllURIs(char *_name, struct ns2__getDataTypeAllURIsResponse &_param_5);

    /// Web service operation 'getURN' (returns error code or SOAP_OK)
    virtual int getURN(char *_name, char *_id, char *&_getURNReturn);

    /// Web service operation 'getDataTypeDef' (returns error code or SOAP_OK)
    virtual int getDataTypeDef(char *_nickname, char *&_getDataTypeDefReturn);

    /// Web service operation 'getDataEntries' (returns error code or SOAP_OK)
    virtual int getDataEntries(char *_nickname, char *_id, struct ns2__getDataEntriesResponse &_param_6);

    /// Web service operation 'getDataEntries' (returns error code or SOAP_OK)
    virtual int getDataEntries_(char *_nickname, char *_id, struct ns2__getDataEntriesResponse_ &_param_7);

    /// Web service operation 'getLocations' (returns error code or SOAP_OK)
    virtual int getLocations(char *_nickname, char *_id, struct ns2__getLocationsResponse &_param_8);

    /// Web service operation 'getLocations' (returns error code or SOAP_OK)
    virtual int getLocations_(char *_nickname, char *_id, struct ns2__getLocationsResponse_ &_param_9);

    /// Web service operation 'getDataEntry' (returns error code or SOAP_OK)
    virtual int getDataEntry(char *_uri, char *_resource, char *&_getDataEntryReturn);

    /// Web service operation 'getDataResources' (returns error code or SOAP_OK)
    virtual int getDataResources(char *_nickname, struct ns2__getDataResourcesResponse &_param_10);

    /// Web service operation 'isDeprecated' (returns error code or SOAP_OK)
    virtual int isDeprecated(char *_uri, char *&_isDeprecatedReturn);

    /// Web service operation 'getOfficialURI' (returns error code or SOAP_OK)
    virtual int getOfficialURI(char *_uri, char *&_getOfficialURIReturn);

    /// Web service operation 'getOfficialDataTypeURI' (returns error code or SOAP_OK)
    virtual int getOfficialDataTypeURI(char *_uri, char *&_getOfficialDataTypeURIReturn);

    /// Web service operation 'getMiriamURI' (returns error code or SOAP_OK)
    virtual int getMiriamURI(char *_uri, char *&_getMiriamURIReturn);

    /// Web service operation 'getDataTypePattern' (returns error code or SOAP_OK)
    virtual int getDataTypePattern(char *_nickname, char *&_getDataTypePatternReturn);

    /// Web service operation 'getResourceInfo' (returns error code or SOAP_OK)
    virtual int getResourceInfo(char *_id, char *&_getResourceInfoReturn);

    /// Web service operation 'getResourceInstitution' (returns error code or SOAP_OK)
    virtual int getResourceInstitution(char *_id, char *&_getResourceInstitutionReturn);

    /// Web service operation 'getResourceLocation' (returns error code or SOAP_OK)
    virtual int getResourceLocation(char *_id, char *&_getResourceLocationReturn);

    /// Web service operation 'getDataTypeSynonyms' (returns error code or SOAP_OK)
    virtual int getDataTypeSynonyms(char *_name, struct ns2__getDataTypeSynonymsResponse &_param_11);

    /// Web service operation 'getNames' (returns error code or SOAP_OK)
    virtual int getNames(char *_uri, struct ns2__getNamesResponse &_param_12);

    /// Web service operation 'getDataTypesName' (returns error code or SOAP_OK)
    virtual int getDataTypesName(struct ns2__getDataTypesNameResponse &_param_13);

    /// Web service operation 'getDataTypesId' (returns error code or SOAP_OK)
    virtual int getDataTypesId(struct ns2__getDataTypesIdResponse &_param_14);

    /// Web service operation 'checkRegExp' (returns error code or SOAP_OK)
    virtual int checkRegExp(char *_identifier, char *_datatype, char *&_checkRegExpReturn);
  };
#endif