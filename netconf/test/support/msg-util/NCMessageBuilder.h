#ifndef __YUMA_NC_MESSAGE_BUILDER__H
#define __YUMA_NC_MESSAGE_BUILDER__H

#include <string>
#include <cstdint>

namespace YumaTest
{

// ---------------------------------------------------------------------------!
/**
 * Utility class for building Netconf XML Messages.
 */
class NCMessageBuilder
{
public:
    /** Constructor */
    explicit NCMessageBuilder();
 
    /** Destructor */
    virtual ~NCMessageBuilder();
 
    /**
     * Build aNetconf Message. This function simlpy adds the XML 
     * start tag and NC_SSH_END indication to the supplied message.
     *
     * \param message the message to build.
     * \return a valid Netconf Message.
     */
    std::string buildNCMessage( const std::string& message ) const;
 
    /**
     * Build a Netconf 'hello' message.
     *
     * \return a Netconf 'hello' Message.
     */
    std::string buildHelloMessage() const;
 
    /**
     * Build a Netconf 'rpc' message.
     *
     * \param queryStr the RPC body
     * \param messageId the id of the message
     * \return a Netconf 'rpc' Message.
     */
    std::string buildRPCMessage( const std::string& queryStr,
                                 uint16_t messageId ) const;

    /**
     * Build a commit message.
     *
     * \param messageId the id of the message
     * \return a Netconf 'rpc-commit' Message.
     */
    std::string buildCommitMessage( const uint16_t messageId );

    /**
     * Build a Netconf 'load' message.
     *
     * \param moduleName the name of the module to load.
     * \param messageId the id of the message
     * \return a Netconf 'load' Message.
     */
    std::string buildLoadMessage( const std::string& moduleName,
                                  uint16_t messageId ) const;
 
    /** 
     * Build a Netconf 'get-config' message
     *
     * \param filter the filter to apply in the format:
     *      \<filter type="XXXX' select="YYYYY" /\>.
     * \param target the target database name
     * \param messageId the id of the message
     * \return a Netconf 'get-config' Message.
     */
     std::string buildGetConfigMessage( const std::string& filter,
                                        const std::string& target,
                                        uint16_t messageId ) const;
 
    /**
     * Build a Netconf 'get-config' message with an XPath filter.
     *
     * \param xPathStr the XPath filter to apply
     * \param target the target database name
     * \param messageId the id of the message
     * \return a Netconf 'get-config' Message.
     *
     */
     std::string buildGetConfigMessageXPath( const std::string& xPathStr,
                                             const std::string& target,
                                             uint16_t messageId ) const;

    /**
     * Build a Netconf 'lock' message.
     *
     * \param messageId the id of the message
     * \param target the target database name
     * \return a Netconf 'lock' Message.
     */
    std::string buildLockMessage( uint16_t messageId, 
                                  const std::string& target ) const;

    /**
     * Build a Netconf 'unlock' message.
     *
     * \param messageId the id of the message
     * \param target the target database name
     * \return a Netconf 'unlock' Message.
     */
    std::string buildUnlockMessage( uint16_t messageId, 
                                    const std::string& target ) const;

    /**
     * Build a Netconf 'set-log-level' message.
     * The following Log Levels are supported:
     * <ul>
     *   <li>off</li>
     *   <li>error</li>
     *   <li>warn</li>
     *   <li>info</li>
     *   <li>debug</li>
     *   <li>debug2</li>
     *   <li>debug3</li>
     *   <li>debug4</li>
     * </ul>
     *
     * \param logLevelStr the log level to set to lock
     * \param messageId the id of the message
     * \return a Netconf 'set-log-level' Message.
     */
    std::string buildSetLogLevelMessage( const std::string& logLevelStr,
                                         uint16_t messageId ) const;

    /**
     * Build a Netconf 'edit-config' message.
     *
     * \param configChange the configuration change
     * \param target the target database name
     * \param messageId the id of the message
     * \param defaultOperation the default Netconf operation
     * \return a Netconf 'edit-config' message.
     */
    std::string buildEditConfigMessage( const std::string& configChange,
                    const std::string& target,
                    uint16_t messageId, 
                    const std::string& defaultOperation = "merge" ) const;

    /**
     * Utility function for generating text for 'edit-config' operations.
     *
     * \param moduleName the name of the module
     * \param moduleNs the namespace of the module
     * \param operation the operation to perform 
     * \return a std::string containing the create text
     */
    std::string genTopLevelContainerText( const std::string& moduleName, 
                                          const std::string& moduleNs, 
                                          const std::string& operation ) const;

    /**
     * Generate XML text for an operation on a node.
     * This function generates XML in the following form:
     *
     * \<nodeName xmlns="NS" nc_operation="operation"\>
     *   nodeVal
     * \</nodeName\>
     *
     * \param nodeName the name of the node 
     * \param nodeVal the new value of the node 
     * \param operation the operation to perform 
     * \return a string containing the xml formatted query
     */
    std::string genOperationText( const std::string& nodeName,
                                  const std::string& nodeVal,
                                  const std::string& operation ) const;

    /**
     * Generate XML text for an operation on a node.
     * This function generates XML in the following form:
     *
     * \<nodeName xmlns="NS" nc_operation="operation"\>
     *   \<keyName\>keyVal\</keyName\>
     * \</nodeName\>
     *
     * \param nodeName the name of the node 
     * \param keyName the name of the key for the node 
     * \param keyVal the new value of the key 
     * \param operation the operation to perform 
     * \return a string containing the xml formatted query
     */
    std::string genKeyOperationText( const std::string& nodeName,
                                     const std::string& keyName,
                                     const std::string& keyVal,
                                     const std::string& operation ) const;

    /**
     * Utility function for formatting module operation text.
     * This formats the supplied text as follows:
     * \<moduleName xmlns="moduleNs" \> queryText \</moduleName\>
     *
     * \param moduleName the name of the module
     * \param moduleNs the namespace of the module
     * \param queryText the query to format
     * \return a string containing the xml formatted query
     */
    std::string genModuleOperationText( const std::string& moduleName,
                                        const std::string& moduleNs,
                                        const std::string& queryText ) const;
  
    
    /**
     * Utility function for formatting a 'path' around the supplied 
     * operation text.
     * This formats the supplied text as follows:
     * \<nodeName\>
     *   \<keyName\>keyVal\</keyName\>
     *   queryText 
     * \</nodeName\>
     *
     * \param nodeName the name of the parent node
     * \param keyName the name of the key for the node 
     * \param keyVal the new value of the key 
     * \param queryText the query to format
     * \return a string containing the xml formatted query
     */
    std::string genKeyParentPathText( const std::string& nodeName,
                                      const std::string& keyName,
                                      const std::string& keyVal,
                                      const std::string& queryText ) const;

protected:
    /**
     * Utility function for generating xmlns text.
     *
     * \param xmlnsArg the argument string (e.g. xmlns or xmlns:nc )
     * \param ns the namespace string 
     */
    const std::string genXmlNsText( const std::string& xmlnsArg, 
                               const std::string& ns ) const;

    /**
     * Utility function for generating xmlns text.
     *
     * \param ns the namespace string 
     */
    const std::string genXmlNsText( const std::string& ns ) const;

    /**
     * Utility function for generating xmlns:nc text.
     *
     * \param ns the namespace string 
     */
    const std::string genXmlNsNcText( const std::string& ns ) const;
};

} // namespace YumaTest

#endif // __YUMA_NC_MESSAGE_BUILDER__H
