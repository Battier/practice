#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
sys.path.append("../lib/rpyc")
import rpyc

if __name__ == "__main__":
    c = rpyc.connect("localhost", 12345)
    token = c.root.login("falu")
    token.put_task('1')
    token.refresh()
    token.put_task('1')
    token.refresh()
    token.put_task("1")
    token.refresh()
    token.put_task("2")
    token.refresh()
    token.put_task("2")
    token.refresh()
    str = token.echo("aaddcc")
    print str
