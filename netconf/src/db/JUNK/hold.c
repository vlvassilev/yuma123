
/********************************************************************
* FUNCTION db_getServerSettings
*
*  mySQL access -- read the server_settings table to get static config
*  for this program
*********************************************************************/
status_t  db_getServerSettings (db_server_settings_t  *pset)
{
    MYSQL_ROW      row;
    MYSQL_RES   *pq;

    if (!db_init) {
        set_error(__FILE__, __LINE__, ERR_INTERNAL_INIT_SEQ, 0);
	return ERR_INTERNAL_INIT_SEQ;
    }
    memset(pset, 0x0, sizeof(db_server_settings_t));
    sprintf(qbuff, "SELECT server_name, bk_color, \
            bk_image FROM server_settings");
    if (mysql_query(db_socket, qbuff) != MYSQL_OK) {
        set_error(__FILE__, __LINE__, ERR_DB_QUERY_FAILED, 1);	
	return ERR_DB_QUERY_FAILED;
    }
    pq = mysql_store_result(db_socket);
    mysql_data_seek(pq, 0);
    row = mysql_fetch_row(pq);
    if (!row) {
        mysql_free_result(pq);
        set_error(__FILE__, __LINE__, ERR_DB_QUERY_FAILED, 1);	
	return ERR_DB_QUERY_FAILED;
    }

    /* fill in the settings struct */        
    strcpy(pset->server_name, row[0]);
    strcpy(pset->bk_color, row[1]);
    strcpy(pset->bk_image, row[2]);

    mysql_free_result(pq);
    return NO_ERR;    
}   /* db_getServerSettings */


/********************************************************************
* FUNCTION db_copy_str
* INPUTS:
*    pval == pointer to value string retrieved from a 
*            QUERY_STRING search (may be NULL)
*    ploc == pointer to destination buffer
*    bufflen == size of dest. buffer in bytes
*********************************************************************/
void db_copy_str (const char *pval, char *ploc, int bufflen)
{
    int   len;

    if (pval) {
        /* value set -- copy string even if already set */
        len = strlen(pval);
	if (len < bufflen) {
	    strcpy(ploc, pval);
	} else {
	    strncpy(ploc, pval, bufflen-1);

#ifdef DB_DEBUG
	   printf("\nError: string truncated '%s' to '%s'\n", pval, ploc);
#endif
	}
    }
    /* else DO NOT ZERO OUT OLD VALUE */
}  /* db_copy_str */



/********************************************************************
* FUNCTION db_getFirstOrder
*
* INPUTS:
*     cur_state == desired state of orders
*         ORD_ST_ANY == all open orders
*         ORD_ST_COMPLETE == all closed orders
*         ORD_ST_<other> == select only orders in that state
*     pret_order == order buffer to use
*     phandle == db_handle_t to use for subsequent 'nextEntry' calls 
* OUTPUTS:
*    *pret_order is filled in if return value is NO_ERR
*    *phandle is filled in the return value is NO_ERR
* RETURNS
*    status: 0 if no error
*********************************************************************/
status_t  db_getFirstOrder (int cur_state, db_order_t *pret_order,
			db_handle_t   *phandle)
{
    MYSQL_ROW      row;
    MYSQL_RES   *pq;
    status_t    res;
    int        num_rows, htyp;

    if (!db_init) {
        set_error(__FILE__, __LINE__, ERR_INTERNAL_INIT_SEQ, 0);	
	return ERR_INTERNAL_INIT_SEQ;
    }

    /* clear return handle */
    (void)memset(phandle, 0x0, sizeof(db_handle_t));

    /* determine the proper SQL query to send */
    if (cur_state==ORD_ST_ANY) {
	sprintf(qbuff, "SELECT ord_id FROM open_orders"
		" ORDER BY ord_state");
	htyp = DB_HTYP_OPEN;
    } else if (cur_state==ORD_ST_COMPLETE) {
	sprintf(qbuff, "SELECT comp_inv_num FROM comp_orders"
		" ORDER BY comp_inv_num");
	htyp = DB_HTYP_CLOSED;
    } else {
	sprintf(qbuff, "SELECT ord_id FROM open_orders"
		" WHERE ord_state=%d ORDER BY ord_create_tvsec",
		cur_state);
	htyp = DB_HTYP_OPEN;
    } 

    if (mysql_query(db_socket, qbuff) != MYSQL_OK)  {
        set_error(__FILE__, __LINE__, ERR_DB_QUERY_FAILED, 1);      
	return ERR_DB_QUERY_FAILED;
    }
    pq = mysql_store_result(db_socket);
    if (!pq) {
	return ERR_DB_NOT_FOUND;
    }
    num_rows = mysql_num_rows(pq);
    if (!num_rows) {
	mysql_free_result(pq);
	return ERR_DB_NOT_FOUND;
    }
    mysql_data_seek(pq, 0);
    row = mysql_fetch_row(pq);
    if (!row) {
	mysql_free_result(pq);	
        set_error(__FILE__, __LINE__, ERR_DB_READ_FAILED, 1);
	return ERR_DB_READ_FAILED;
    }

    /* row[0] == open order ID or completed order invoice number */
    if (htyp==DB_HTYP_OPEN) {
	res = db_getOrder(row[0], pret_order);
    } else {
	res = db_getCompletedOrder(atoi(row[0]), pret_order);
    }
    if (res != NO_ERR) {
	mysql_free_result(pq);
	return res;
    }

    /* else retrieved the first open or closed order */
    /* don't free MYSQL_RES, return it as a handle for next call */
    phandle->htyp = htyp;
    phandle->presult = pq;
    phandle->num_rows = num_rows;
    phandle->pos = 1;   /* returning row zero now */
    return NO_ERR;

} /* db_getFirstOrder */


/********************************************************************
* FUNCTION db_getNextOrder
*
* RETURNS
*    status: 0 if no error
*********************************************************************/
status_t db_getNextOrder (db_handle_t *phandle, 
			   int         cur_state,
			   db_order_t *pret_order)
{
    MYSQL_ROW      row;

    if (!db_init) {
        set_error(__FILE__, __LINE__, ERR_INTERNAL_INIT_SEQ, 0);
	return ERR_INTERNAL_INIT_SEQ;
    }

    /* the db_getFirstOrder function set the 'pos' field to 1,
     * so the user should be calling this function with 'pos'
     * pointing to the next available row or == to the num_rows
     */
    if (phandle->pos < phandle->num_rows) {
	mysql_data_seek(phandle->presult, phandle->pos++);
	row = mysql_fetch_row(phandle->presult);
	if (row) {
	    if (phandle->htyp==DB_HTYP_OPEN) {
		return db_getOrder(row[0], pret_order);
	    } else {
		return db_getCompletedOrder(atoi(row[0]), pret_order);
	    }
	} else {
	    set_error(__FILE__, __LINE__, ERR_DB_READ_FAILED, 1);
	    return ERR_DB_READ_FAILED;
	}
    } else {
	return ERR_DB_NOT_FOUND;
    }
} /* db_getNextOrder */


/********************************************************************
* FUNCTION db_releaseHandle
*
* RETURNS
*    status: 0 if no error
*********************************************************************/
void db_releaseHandle(db_handle_t *phandle)
{
    if (phandle) {
	if (phandle->presult) {
	    mysql_free_result((MYSQL_RES *)phandle->presult);
	}
	(void)memset(phandle, 0x0, sizeof(db_handle_t));
    }
} /* db_releaseHandle */

