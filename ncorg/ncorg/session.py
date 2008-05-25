import logging

log = logging.getLogger("ncorg.session")

def sessionAdded (data):
    log.debug("sessionAdded called")


def sessionDeleted (data):
    log.debug("sessionDeleted called")

