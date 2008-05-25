
/********************************************************************
* FUNCTION wr_pdu
*
* Write a hello message to the specified buffer
*
* INPUTS:
*    buffer == buffer to fill with the <hello> PDU
*    caps == cap_list_t struct to print in the PDU
*    session_id == 0 if not printed, non-zero if added
* RETURNS:
*    status, NO_ERR if no param or buffer overflow errors
*********************************************************************/
static status_t wr_pdu (buf_buffer_t  *buffer,
			cap_list_t *caps,
			uint32 session_id)
{
    status_t  res;
    xml_attrs_t  attrs;
    xmlChar num[12];
   
    /* setup the hello attributes */
    xml_init_attrs(&attrs);
    res = xml_add_attr(&attrs, xmlns_nc_id(), 
                       (const xmlChar *) XMLNS, (const xmlChar *) NC_URN);
    if (res != NO_ERR) {
	xml_clean_attrs(&attrs);
	return res;
    }
    
    /* start the hello message */
    res = buf_write_tag(buffer, xmlns_nc_id(), 
	(const xmlChar *) NC_HELLO_STR, BUF_START_TAG, &attrs);
    xml_clean_attrs(&attrs);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the hello element contents */
    buf_inc_indent_level(buffer);

    /* write the capabilities string */
    res = cap_emit_caplist(buffer, caps);
    if (res != NO_ERR) {
	return res;
    }

    /* maybe print the session-id */
    if (session_id) {
	xmlStrPrintf(num, 12, (const xmlChar *) "%lu", session_id);
	res = buf_write_elem(buffer, xmlns_nc_id(), 
            (const xmlChar *) NC_SESSION_ID, num, NULL);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* undo the indent for the hello element contents */
    buf_dec_indent_level(buffer);

    /* write the end tag for the hello message */
    return buf_write_tag(buffer, xmlns_nc_id(), 
		(const xmlChar *) NC_HELLO_STR, BUF_END_TAG, NULL);

} /* wr_pdu */


/********************************************************************
* FUNCTION hello_wr_mgr
*
* Write a manager hello message to the specified buffer
*
* INPUTS:
*    buffer == buffer to fill with the <hello> PDU
* RETURNS:
*    status, NO_ERR if no param or buffer overflow errors
*********************************************************************/
status_t hello_wr_mgr (buf_buffer_t  *buffer)
{
    cap_list_t *mgr_caps;

    /* check params */
    if (!buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    mgr_caps = mgr_cap_get_caps();
    if (!mgr_caps) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }	

    return wr_pdu(buffer, mgr_caps, 0);

} /* hello_wr_mgr */
