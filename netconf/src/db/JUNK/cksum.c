
/********************************************************************
* FUNCTION calc_cksum
*
*
*********************************************************************/
static ulong calc_cksum (db_order_t *porder)
{
    int     i, len;
    ulong   sum;
    ulong   *plong;
    
    len =  (sizeof(db_order_t) - sizeof(ulong)) / 4;
    plong = (ulong *)porder;
    sum = 0;
    for (i=0; i<len; i++) {
	sum += *plong++;
    }
    return (sum - 0x55555555);
}



/********************************************************************
* FUNCTION set_cksum
*
*
*********************************************************************/
static inline void  set_cksum (db_order_t  *porder)
{
    porder->od_checksum = calc_cksum(porder);
}

/********************************************************************
* FUNCTION cksum_ok
*
*
*********************************************************************/
static inline boolean cksum_ok(db_order_t  *porder)
{
    ulong   csum;

    csum = calc_cksum(porder);
    return (boolean)(csum == porder->od_checksum);
}
