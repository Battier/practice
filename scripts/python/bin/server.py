#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os

sys.path.append("../lib/rpyc")
sys.path.append("../lib")
import rpyc
from rpyc.utils.server import ThreadedServer
import log
import tasks

import threading
lock = threading.Lock()

ROOT_USERS = {
    "falu" : "1",
}
tokens = set()

class GuestToken(rpyc.Service):
    'Define guest services'
    def __init__(self, name):
        self.name = name
        self.stale = False
        self.role = "Guest"
        print "* Hello Guest %s *" % self.name
        tokens.add(self)

    def exposed_logout(self):
        if self.stale:
            return
        self.stale = True
        self.callback = None

    def exposed_put_task(self, priority):
        'Put a new task'
        db = tasks.get_task_db()
        db.push(priority, self.name)
        return True

    def exposed_refresh(self):
        'Put a new task'
        db = tasks.get_task_db()
        db.refresh()
        return True

    def exposed_get_sys_path(self):
        print sys.path
        return sys.path


    def exposed_echo(self, text):
        print text
        return text

class RootToken(GuestToken):
    'Define root services'
    def __init__(self, name):
        self.name = name
        self.stale = False
        self.callback = "Admin"
        print "* Hello Admin %s *" % self.name
        tokens.add(self)

    def exposed_remove_task(self, priority, task_index):
        'Remove a task'
        db = tasks.get_task_db()
        db.pop(priority, index)

class MyService(rpyc.Service):
    'Define all services for server'
    def __init__(self, logger=None):
        self.logger = logger or logging.getLogger(__name__)
        self.token = None

    def exposed_logout(self):
        if self.token:
            self.token = None

    def exposed_login(self, username):
        print username
        if self.token and not self.token.stale:
            raise ValueError("already logged in")
        if username in ROOT_USERS:
            self.token = RootToken(username)
        else:
            self.token = GuestToken(username)
        return self.token

def server():
    'Launch a server'
    try:
        server = ThreadedServer(MyService, port = 12345, protocol_config = {"allow_all_attrs": True}, auto_register = False)
        server.start()
    except Exception:
        raise
    return True


def test():
    'test server'
    log.setup_logging()
    server()

if __name__ == '__main__':
    test()
