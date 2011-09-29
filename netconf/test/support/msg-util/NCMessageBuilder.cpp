#include "test/support/msg-util/NCMessageBuilder.h"

//#include <iostream>
#include <sstream>
#include <string>

#include "src/ncx/xml_util.h"

// ---------------------------------------------------------------------------!
using namespace std;

// ---------------------------------------------------------------------------!
// Anonymous Namespace
// ---------------------------------------------------------------------------!
namespace
{

// ---------------------------------------------------------------------------!
const char* IETF_BASE = "urn:ietf:params:netconf:base:1.0";
const char* IETF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0";
const char* YUMA_NS = "http://netconfcentral.org/ns/yuma-system";

} // Anonymous namespace

namespace YumaTest
{

// ---------------------------------------------------------------------------!
NCMessageBuilder::NCMessageBuilder() 
{
}

// ---------------------------------------------------------------------------!
NCMessageBuilder::~NCMessageBuilder()
{
}

// ---------------------------------------------------------------------------!
const string NCMessageBuilder::genXmlNsText( const string& xmlnsArg, 
                                             const string& ns ) const
{
    stringstream query;

    query << xmlnsArg << "=\"" << ns << "\"";

    return query.str();
}

// ---------------------------------------------------------------------------!
const string NCMessageBuilder::genXmlNsText( const string& ns ) const
{
    return genXmlNsText( "xmlns", ns );
}

// ---------------------------------------------------------------------------!
const string NCMessageBuilder::genXmlNsNcText( const string& ns ) const
{
    return genXmlNsText( "xmlns:nc", ns );
}

// ---------------------------------------------------------------------------!
string NCMessageBuilder::buildNCMessage( const string& message ) const
{
    stringstream query;

    query << XML_START_MSG << "\n" << message << "\n"; // << NC_SSH_END;
   
    return query.str();
}

// ---------------------------------------------------------------------------!
string NCMessageBuilder::buildHelloMessage() const
{
    stringstream query;

    query << "<hello " << genXmlNsNcText( IETF_NS )
          << "       " << genXmlNsText( IETF_NS ) << ">"
          << "  <capabilities>"
          << "    <capability>" << IETF_BASE << "</capability>"
          << "  </capabilities>\n"
          << "</hello>";

   return buildNCMessage( query.str() );
}

// ---------------------------------------------------------------------------!
string NCMessageBuilder::buildRPCMessage( const std::string& queryStr,
                                          const uint16_t messageId ) const
{
    stringstream query;
    
    query << "<rpc message-id=\"" << messageId << "\"\n"
          << "     " << genXmlNsNcText( IETF_NS ) << "\n"
          << "     " << genXmlNsText( IETF_NS ) << ">\n"
          << queryStr << "\n"
          << "</rpc>";

    return buildNCMessage( query.str() );
}

// ---------------------------------------------------------------------------!
string NCMessageBuilder::buildLoadMessage( const std::string& moduleName,
                                           const uint16_t messageId ) const
{
    stringstream query;
    query << "  <load " << genXmlNsText( YUMA_NS ) << ">\n"
          << "    <module> " << moduleName << " </module>\n"
          << "  </load>";

    return buildRPCMessage( query.str(), messageId );
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::buildGetConfigMessage( const string& filter,
                                                const string& target,
                                                const uint16_t messageId )  const
{
    stringstream query;
    query << "  <get-config " << genXmlNsText( IETF_NS ) << ">\n"
          << "    <source> " << "<" << target << "/> " << "</source>\n"
          << "    " << filter << "\n"
          << "  </get-config>";
 
    return buildRPCMessage( query.str(), messageId );
}

// ------------------------------------------------------------------------|
string NCMessageBuilder:: buildGetConfigMessageXPath( 
        const string& xPathStr,
        const string& target,
        const uint16_t messageId )  const
{
    stringstream query;
    query << "<filter type=\"xpath\" " 
          << "select=\"" << xPathStr << "\"/>" ;
 
    return buildGetConfigMessage( query.str(), target, messageId );
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::buildLockMessage( const uint16_t messageId,
                                           const string& target ) const
{
    stringstream query;
    
    query << "  <lock><target><" << target << "/></target></lock>";
    return buildRPCMessage( query.str(), messageId );
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::buildUnlockMessage( const uint16_t messageId,
                                             const string& target ) const
{
    stringstream query;
    
    query << "  <unlock><target><" << target << "/></target></unlock>";
    return buildRPCMessage( query.str(), messageId );
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::buildSetLogLevelMessage( 
      const std::string& logLevelStr,
      const uint16_t messageId ) const
{
    stringstream query;
    
    query << "  <set-log-level " << genXmlNsText( YUMA_NS ) << ">\n"
          << "     <log-level>" << logLevelStr << "</log-level>\n"
          << "  </set-log-level>\n";
    return buildRPCMessage( query.str(), messageId );
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::buildEditConfigMessage( const string& configChange, 
        const string& target,
        uint16_t messageId,
        const string& defaultOperation ) const
{
    stringstream query;
    query << "  <edit-config " << genXmlNsText( IETF_NS ) << ">\n"
          << "    <target> " << "<" << target << "/> " << "</target>\n"
          << "    <default-operation>" << defaultOperation 
              << "</default-operation>\n"
          << "    <config>\n"
          << "      " << configChange << "\n"
          << "    </config>\n"
          << "  </edit-config>";
    return buildRPCMessage( query.str(), messageId );
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::genTopLevelContainerText( 
        const string& moduleName,
        const string& moduleNs,
        const string& operation ) const
{
    stringstream query;
 
    query << "<" << moduleName << " " << genXmlNsNcText( IETF_NS )
          << " nc:operation=\"" << operation << "\""
          << " " << genXmlNsText( moduleNs ) << "/>";
        
    return query.str();
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::genOperationText( 
        const string& nodeName,
        const string& nodeVal,
        const string& operation ) const
{
    stringstream query;
 
    query << "<" << nodeName << " " 
                 << genXmlNsNcText( IETF_NS ) << " " 
                 << " nc:operation=\"" << operation 
          << "\">\n"
          << nodeVal 
          << "</" << nodeName << ">";
    return query.str();
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::genKeyOperationText( 
        const string& nodeName,
        const string& keyName,
        const string& keyVal,
        const string& operation ) const
{
    stringstream query;
 
    query << "<" << nodeName << " " 
                 << genXmlNsNcText( IETF_NS ) << " " 
                 << " nc:operation=\"" << operation 
          << "\">\n"
          << "<" << keyName << ">" << keyVal << "</" << keyName << ">"
          << "</" << nodeName << ">";
    return query.str();
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::genKeyParentPathText( 
        const string& nodeName,
        const string& keyName,
        const string& keyVal,
        const string& queryText ) const
{
    stringstream query;

    query << "<" << nodeName << ">\n"
          << "<" << keyName << ">" << keyVal << "</" << keyName << ">\n"
          << queryText << "\n"
         << "</" << nodeName << ">"; 
 
    return query.str();
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::genModuleOperationText( 
        const string& moduleName,
        const string& moduleNs,
        const string& queryText ) const
{
    stringstream query;
 
    query << "<" << moduleName << " "
          << genXmlNsText( moduleNs ) << ">\n"
          << queryText
          << "</" << moduleName << ">";
        
    return query.str();
}

// ------------------------------------------------------------------------|
string NCMessageBuilder::buildCommitMessage( const uint16_t messageId )
{
    return buildRPCMessage( "<commit/>", messageId );
} 

} // namespace YumaTest
