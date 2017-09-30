#! /usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import re
import time
import logging

# semi-standard modules
try:
    import pexpect
except ImportError:
    sys.stderr.write("Module pexpect is not available. It can be downloaded from http://pexpect.sourceforge.net/\n")
    raise

class telnet(object):
    def __init__(self, ip, port, timeout=60):
        self.host = ip
        self.port = port
        self.opts = ["Escape character is",
            "Connection refused",
            "Username:",
            "Password:",
            "Selection:",
            "loader>",
            "root@sup1:~#",
            pexpect.EOF,
            pexpect.TIMEOUT]
        self.timeout = timeout

    def login(self, port=""):
        self.tn = pexpect.spawn('telnet %s %s' % (self.host, port))
        while True:
            index = self.tn.expect(self.opts, self.timeout)
            if index == 0:
                self.tn.sendline("")
            elif index == 1:
                self.clear_line():
                self.login(port)
            elif index == 2:
                self.tn.sendline(self.port_username)
            elif index == 3:
                self.tn.sendline(self.port_password)
            elif index == 4:
                self.tn.sendline("x")
            elif index == 5:
                return True
            else:
                return False

    def login2port(self):
        if self.login(self.host, self.port):
            return True
        else:
            return False

    def clear_line(self):
        if self.login(self.host):
            self.send('enable', 'Password', 6)
            self.send(self.server_password, '[0-9a-zA-z\-]*[>#]', 6)
            line = 'clear line %s' % (self.port - 2000)
            self.send(line, '\[confirm\]', 6)
            self.send('', '[0-9a-zA-z\-]*[>#]', 6)
            self.send(line, '\[confirm\]', 6)
            self.send('', '[0-9a-zA-z\-]*[>#]', 6)
            self.tn.close()
            return True
        else:
            print "Failed to clear line."
            return False

    def send(self, command, pattern, timeout):
        self.tn.sendline(command)
        index = self.tn.expect([pexpect.TIMEOUT, pattern], timeout)
        if index == 0:
            print "Failed send command: %s"%command
            return True
        return False

    def login_out(self):
        self.tn.close()
        return

    def interact(self):
        self.tn.interact()
        return

class ssh(object):
    def __init__(self, ip, username, password, timeout=60):
        self.host = ip
        self.user = username
        self.pw = password
        self.opts = ['[0-9a-zA-z\-]*[>#]',
            pexpect.EOF,
            pexpect.TIMEOUT]
        self.timeout = timeout

    def login(self):
        self.tn = pexpect.spawn('ssh %s@%s' % (self.user, self.host))
        while True:
            index = self.tn.expect(self.opts, self.timeout)
            if index == 0:
                return True
            else:
                return False

    def send(self, command, pattern, timeout=60):
        self.tn.sendline(command)
        index = self.tn.expect([pexpect.TIMEOUT, pattern], timeout)
        if index == 0:
            print "Failed send command: %s"%command
            return True
        return False

    def login_out(self):
        self.tn.close()
        return

    def interact(self):
        self.tn.interact()
        return

class setup(object):
    def __init__(self, platform):
        self.power_ip = 10.74.126.5
        self.power_port = 2008
        self.console_ip = 10.74.126.5
        self.console_port = 2008
        self.ssh_ip = 10.124.11.108
        self.ssh_user = 'diag'
        self.ssh_pw = ''

    def power_cycle(self):
        self.power_cycle_terminal = telnet(self.power_ip, self.power_port)
        self.power_cycle_terminal.clear_line()
        self.power_cycle_terminal.login2port()

    def console(self):
        self.tty = telnet(self.console_ip, self.console_port)
        self.tty.login2port()

    def ssh(self):
        self.ssh = ssh(self.ssh_ip, self.ssh_user, self.ssh_pw)
        self.ssh.login()

    def send_by_ssh(self, command):
        self.ssh.send(command, '[0-9a-zA-z\-]*[>#]')

    def send_by_console(self, command):
        self.ssh.send(command, '[0-9a-zA-z\-]*[>#]')

    def interact(self):
        self.ssh.interact()

def saveLog () :
    log_file = time.strftime("/auto/crdc_dcbu_diag_users/falu/log/%a_%d_%b_%Y_%H:%M:%S.txt", time.localtime())
    fout = file(log_file,'w')
    return fout

if __name__=="__main__":
    p = setup('Yushan' )
    p.ssh()
    p.send_by_ssh()
    p.interact()
