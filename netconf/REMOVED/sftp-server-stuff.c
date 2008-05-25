    /* sftp file transfer ... */
#ifdef NOT_YET
    pw = my_pwcopy(pw);

    logit("session opened for local user %s from [%s]",
	  pw->pw_name, client_addr);


    handle_init();

    in = dup(STDIN_FILENO);
    out = dup(STDOUT_FILENO);

#ifdef HAVE_CYGWIN
    setmode(in, O_BINARY);
    setmode(out, O_BINARY);
#endif

    max = 0;
    if (in > max)
	max = in;
    if (out > max)
	max = out;

    buffer_init(&iqueue);
    buffer_init(&oqueue);

    set_size = howmany(max + 1, NFDBITS) * sizeof(fd_mask);
    rset = (fd_set *)xmalloc(set_size);
    wset = (fd_set *)xmalloc(set_size);

    for (;;) {
	memset(rset, 0, set_size);
	memset(wset, 0, set_size);

	FD_SET(in, rset);
	olen = buffer_len(&oqueue);
	if (olen > 0)
	    FD_SET(out, wset);

	if (select(max+1, rset, wset, NULL, NULL) < 0) {
	    if (errno == EINTR)
		continue;
	    error("select: %s", strerror(errno));
	    cleanup_exit(2);
	}

	/* copy stdin to iqueue */
	if (FD_ISSET(in, rset)) {
	    char buf[4*4096];
	    len = read(in, buf, sizeof buf);
	    if (len == 0) {
		debug("read eof");
		cleanup_exit(0);
	    } else if (len < 0) {
		error("read: %s", strerror(errno));
		cleanup_exit(1);
	    } else {
		buffer_append(&iqueue, buf, len);
	    }
	}
	/* send oqueue to stdout */
	if (FD_ISSET(out, wset)) {
	    len = write(out, buffer_ptr(&oqueue), olen);
	    if (len < 0) {
		error("write: %s", strerror(errno));
		cleanup_exit(1);
	    } else {
		buffer_consume(&oqueue, len);
	    }
	}
	/* process requests from client */
	process();
    }
#endif
