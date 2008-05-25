#!/usr/bin/python
# -*- coding: utf-8 -*-
"""Start script for the ncorg TurboGears project.

This script is only needed during development for running from the project
directory. When the project is installed, easy_install will create a
proper start script.
"""

import sys
import cherrypy
import ncorg
from ncorg.commands import start, ConfigurationError
# from ncorg.session import sessionAdded, sessionDeleted

if __name__ == "__main__":

    update_map = {'global' :
                      {'sessionFilter.onCreateSession' : ncorg.session.sessionAdded,
                       'sessionFilter.onDeleteSession' : ncorg.session.sessionDeleted}}
    cherrypy.config.update(updateMap = update_map)

    try:
        start()
    except ConfigurationError, exc:
        sys.stderr.write(str(exc))
        sys.exit(1)

