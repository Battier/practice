#! /usr/bin/env python
# -*- coding: utf-8 -*-
import os
import shelve
import signal
import subprocess
import time
import threading
lock = threading.Lock()


class task(object):
    def __init__(self, name):
        self.status = None
        self.owner = name
        self.spent = None
        self.proc = None
        self.start_time = None

    def run(self):
        self.proc = subprocess.Popen(['python', 'hello.py', self.owner])
        self.status = "Running"
        self.start_time = time.time()
        return True

    def update(self):
        self.proc.poll()
        if self.proc.returncode:
            self.status = "Done"
        self.spend = time.time() - self.start_time

    def status(self):
        update()
        print "%s:%s..... %s seconds" % self.owner, self.status, self.spend

class task_queue(object):
    def __init__(self, priority):
        self.num = 0
        self.priority = priority
        self.queue = []

    def push(self, name):
        self.num += 1
        self.queue.append(task(name))
        return True

    def pop(self, index):
        if index <= self.num and self.num > 0:
            self.queue.pop(index)
            self.num -= 1
        return True

    def insert(self, name, index):
        self.num += 1
        self.queue.insert(index, task(name))
        return True

class task_db(object):
    def __init__(self):
        self.db = shelve.open("../var/tasks.db", 'c', writeback = True)

    def get_db(self):
        return self.db

    def push(self, priority, name):
        print "Adding a new task in %s" % priority
        try:
            lock.acquire()
            if priority not in self.db.keys():
                print "Adding priority %s" % priority
                self.db[priority] = task_queue(priority)
            self.db[priority].push(name)   
            self.db.sync
            lock.release()
            return True
        except Exception:
            return False

    def pop(self, priority, index):
        try:
            lock.acquire()
            if priority in self.db.keys():
                self.db[priority].pop(index)   
                self.db.sync
            lock.release()
            return True
        except Exception:
            return False

    def insert(self, priority, name, index):
        try:
            lock.acquire()
            self.db[priority].insert(name, index)   
            self.db.sync
            lock.release()
            return True
        except Exception:
            return False

    def refresh(self):
        for each_priority in self.db.keys():
            if len(self.db[each_priority].queue):
                self.db[each_priority].queue[0].run()
                self.db[each_priority].pop(0)
        return True

task_db_global = task_db()
def get_task_db():
    return task_db_global

if __name__ == '__main__':
	db = get_task_db()
	db['1'].status = False
	print db['1'].status
