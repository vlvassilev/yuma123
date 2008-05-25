/**
 * section: xmlReader
 * synopsis: Parse an XML file with an xmlReader
 * purpose: Demonstrate the use of xmlReaderForFile() to parse an XML file
 *          and dump the informations about the nodes found in the process.
 *          (Note that the XMLReader functions require libxml2 version later
 *          than 2.6.)
 * usage: reader1 <filename>
 * test: reader1 test2.xml > reader1.tmp ; diff reader1.tmp reader1.res ; rm reader1.tmp
 * author: Daniel Veillard
 * copy: see Copyright for the status of this software.
 */

#include <stdio.h>
#include <libxml/xmlreader.h>

#ifdef LIBXML_READER_ENABLED


/* get the node type according to the xmlElementType enum list
 * in /usr/include/libxml2/libxml/tree.h
 */
static const char * getNodeName (int nodeval)
{
    switch (nodeval) {
    case 1: return "XML_ELEMENT_NODE";
    case 2: return "XML_ATTRIBUTE_NODE";
    case 3: return "XML_TEXT_NODE";
    case 4: return "XML_CDATA_SECTION_NODE";
    case 5: return "XML_ENTITY_REF_NODE";
    case 6: return "XML_ENTITY_NODE";
    case 7: return "XML_PI_NODE";
    case 8: return "XML_COMMENT_NODE";
    case 9: return "XML_DOCUMENT_NODE";
    case 10: return "XML_DOCUMENT_TYPE_NODE";
    case 11: return "XML_DOCUMENT_FRAG_NODE";
    case 12: return "XML_NOTATION_NODE";
    case 13: return "XML_HTML_DOCUMENT_NODE";
    case 14: return "XML_DTD_NODE";
    case 15: return "XML_ELEMENT_DECL";
    case 16: return "XML_ATTRIBUTE_DECL";
    case 17: return "XML_ENTITY_DECL";
    case 18: return "XML_NAMESPACE_DECL";
    case 19: return "XML_XINCLUDE_START";
    case 20: return "XML_XINCLUDE_END";
    case 21: return "XML_DOCB_DOCUMENT_NODE";
    default: return "**unknown**";
    }

} /* getNodeName */


/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
static void
processNode(xmlTextReaderPtr reader) {
    xmlChar *str;
    int  cnt, i;

    str = xmlTextReaderName(reader);
    if (str == NULL) {
	str = (const xmlChar *)"--";
    }

    printf("\nL:%d Type:%s Name:'%s' IsEmpty:%d HasVal:%d ", 
	   xmlTextReaderDepth(reader),
	   getNodeName(xmlTextReaderNodeType(reader)),
	   str,
	   xmlTextReaderIsEmptyElement(reader),
	   xmlTextReaderHasValue(reader));

    str = xmlTextReaderValue(reader);
    if (str == NULL) {
	str = (const xmlChar *)"--";
    }
    if (xmlStrlen(str) > 40) {
        printf(" '%.40s...'", str);
    } else {
        printf(" '%s'", str);
    }

    str = xmlTextReaderPrefix(reader);
    if (!str) {
        str = (const xmlChar *)"--";
    }
    printf("\n  pfix: '%s'", str);

    str = xmlTextReaderNamespaceUri(reader);
    if (!str) {
        str = (const xmlChar *)"--";
    }
    printf("  NS: '%s'", str);

    str = xmlTextReaderReadOuterXml(reader);
    printf("\n  outer XML: '%s'", (str) ? (char *)str : "none");

    str = xmlTextReaderReadInnerXml(reader);
    printf("\n  inner XML: '%s'", (str) ? (char *)str : "none");

    cnt = xmlTextReaderAttributeCount(reader);
    printf("\n   AttrCnt: %d", cnt);
    for (i=0; i<cnt; i++) {
        if (xmlTextReaderMoveToAttributeNo(reader, i)) {
            str = xmlTextReaderName(reader);
            if (str == NULL) {
                str = (const xmlChar *)"--";
            }
            printf("\n    attr: '%s' ", str);

            str = xmlTextReaderValue(reader);
            if (str == NULL) {
                str = (const xmlChar *)"--";
            }

            printf(" val: '%s' ", str);
        }
    }
    (void)xmlTextReaderMoveToElement(reader);
        
}

/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse and print information about an XML file.
 */
static void
streamFile(const char *filename) {
    xmlTextReaderPtr reader;
    int ret, options;

    options = XML_PARSE_NOBLANKS + XML_PARSE_XINCLUDE;

    reader = xmlReaderForFile(filename, NULL, options);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            processNode(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            fprintf(stderr, "%s : failed to parse\n", filename);
        }
    } else {
        fprintf(stderr, "Unable to open %s\n", filename);
    }
}

int main(int argc, char **argv) {
    if (argc != 2)
        return(1);

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    streamFile(argv[1]);

    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
    return(0);
}

#else
int main(void) {
    fprintf(stderr, "XInclude support not compiled in\n");
    exit(1);
}
#endif
