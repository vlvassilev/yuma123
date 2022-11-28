import litenc
import lxml
from lxml import etree

def strip_namespaces(tree):
	xslt='''<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" indent="no"/>

    <xsl:template match="/|comment()|processing-instruction()">
        <xsl:copy>
          <xsl:apply-templates/>
        </xsl:copy>
    </xsl:template>

    <xsl:template match="*">
        <xsl:element name="{local-name()}">
          <xsl:apply-templates select="@*|node()"/>
        </xsl:element>
    </xsl:template>

    <xsl:template match="@*">
        <xsl:attribute name="{local-name()}">
          <xsl:value-of select="."/>
        </xsl:attribute>
    </xsl:template>
    </xsl:stylesheet>
    '''
	xslt_doc = lxml.etree.fromstring(xslt)
	transform = lxml.etree.XSLT(xslt_doc)
	tree = transform(tree)
	return tree

class litenc_lxml():
	def __init__(self, litenc, strip_namespaces=False):
		self.litenc=litenc
		self.strip_namespaces=strip_namespaces

	def receive(self):
		(ret,reply_xml)=self.litenc.receive()
		if(ret!=0):
			return None
		reply_lxml = lxml.etree.fromstring(reply_xml.encode('ascii'))
		if(self.strip_namespaces):
			reply_lxml=strip_namespaces(reply_lxml)
		return reply_lxml

	def rpc(self, xml, message_id=1):
		ret=self.litenc.send('''<rpc xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="'''+str(message_id)+'''">'''+xml+"</rpc>")
		if(ret!=0):
			return None
		reply_lxml=self.receive()
		if(self.strip_namespaces):
			reply_lxml=strip_namespaces(reply_lxml)
		return reply_lxml

	def send(self, xml):
		return self.litenc.send(xml)
