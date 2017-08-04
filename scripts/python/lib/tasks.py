#! /usr/bin/env python
# -*- coding: utf-8 -*-
import os
import shelve
import signal
import subprocess
import time
from datetime import datetime
import threading

class task(object):
    def __init__(self, name, setup):
        self.status = 'Pending'
        self.owner = name
        self.setup = setup
        self.spend = 0.00
        self.timeout = 5.00
        self.proc = None
        self.log = None
        self.start_time = 0.00

    def run(self):
        timestr = datetime.now().strftime('_%f_%S_%H_%M_%d_%m_%Y.log')
        self.log = open('../var/' + self.setup + '_' + self.owner + timestr, 'w', 1)
        self.proc = subprocess.Popen(['python', '../bin/hello.py', self.owner], stdout=self.log, stderr=subprocess.PIPE)
        self.status = 'Running'
        self.start_time = time.time()
        print "%s  %s start_time:%s" % (self.setup, self.owner, self.start_time)
        return True

    def update(self):
        if self.status == 'Running':
            self.log.flush()
            self.spend = time.time() - self.start_time
            if self.proc.poll() is not None or self.timeout < self.spend:
                self.log.close()
                self.status = "Done"
        return (self.owner, self.status, self.spend)

    def get_status(self):
        if self.status == 'Running':
            self.update()
        return self.status

class tasks_queue(object):
    def __init__(self, setup):
        self.num = 0
        self.setup = setup
        self.queue = []
        self.running = None

    def push(self, name):
        self.num += 1
        self.queue.append(task(name, self.setup))
        return True

    def pop(self, index):
        if index <= self.num and self.num > 0:
            self.queue.pop(index)
            self.num -= 1
        return True

    def insert(self, name, index):
        if index <= 0 and self.num > 0:
            return False
        self.num += 1
        self.queue.insert(index, task(name))
        return True

    def refresh(self):
        if self.queue[0].get_status() == 'Pending':
            self.queue[0].run()
        elif self.queue[0].get_status() == 'Done':
            self.pop(0)
        return True

    def get_status(self):
        print "----------------------------%s--------------------------------"%self.setup
        for job in self.queue:
            owner, status, spend = job.update()
            print "%5d  %20s %20s %20s" % (self.queue.index(job), owner, status, spend)
        print "-----------------------------------------------------------------"
        return True

class task_db(object):
    def __init__(self):
        self.db = shelve.open("../var/tasks.db", 'c', writeback = True)
        self.lock = threading.Lock()

    def get_status(self):
        for each_setup in self.db.keys():
            self.db[each_setup].get_status()
        return True

    def push(self, setup, name):
        try:
            self.lock.acquire()
            if setup not in self.db.keys():
                self.db[setup] = tasks_queue(setup)
            self.db[setup].push(name)   
            self.db.sync
            self.lock.release()
            return True
        except Exception:
            return False

    def pop(self, setup, index):
        try:
            self.lock.acquire()
            if setup in self.db.keys():
                self.db[setup].pop(index)   
                self.db.sync
            self.lock.release()
            return True
        except Exception:
            return False

    def insert(self, setup, name, index):
        try:
            self.lock.acquire()
            self.db[setup].insert(name, index)   
            self.db.sync
            self.lock.release()
            return True
        except Exception:
            return False

    def refresh(self, setup):
        try:
            self.lock.acquire()
            if setup in self.db.keys() and len(self.db[setup].queue):
                self.db[setup].refresh()
                self.db.sync
            self.lock.release()
            return True
        except Exception:
            return False

    def refresh_all(self):
        for each_setup in self.db.keys():
            self.refresh(each_setup) 
        return True

task_db_global = task_db()
def get_task_db():
    return task_db_global

if __name__ == '__main__':
	db = get_task_db()
        db.push('P2', 'a11111111111')
        db.refresh('P2')
        db.push('P2', '222222222222')
        db.push('P2', '333333333333')
        db.push('P2', '444444444444')
        db.push('P2', '555555555555')
        db.push('P2', '666666666666')
        db.push('P2', '777777777777')
        db.refresh('P2')
        db.refresh('P2')
        db.refresh('P2')
        db.refresh('P2')
        db.refresh('P2')
        db.refresh('P2')
        db.push('P1', 'aa')
        db.refresh('P1')
        db.push('P1', 'aa')
        db.push('P1', 'aa')
        db.push('P1', 'aa')
        db.push('P1', 'aa')
        db.push('P1', 'aa')
        db.refresh('P1')
        db.refresh_all()
        time.sleep(2)
        db.get_status()
        db.refresh_all()
        time.sleep(2)
        db.get_status()
        db.refresh_all()
        time.sleep(2)
        db.refresh_all()
        db.get_status()
        db.refresh_all()
        time.sleep(2)
        db.get_status()
        db.refresh_all()
        time.sleep(2)
        db.get_status()
        time.sleep(2)
        db.get_status()
        db.refresh_all()
        db.get_status()
