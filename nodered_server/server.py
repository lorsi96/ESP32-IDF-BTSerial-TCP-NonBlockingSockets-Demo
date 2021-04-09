# This example code is in the Public Domain (or CC0 licensed, at your option.)

# Unless required by applicable law or agreed to in writing, this
# software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, either express or implied.

#Author: lorsi

import os
import re
import socket
import time
import sys
from random import randint
import fcntl
from builtins import input
from threading import Event, Thread

# import ttfw_idf

# -----------  Config  ----------
PORT = 3333
# -------------------------------

class TcpServer:

    def __init__(self, port, family_addr, persist=False):
        self.port = port
        self.socket = socket.socket(family_addr, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.settimeout(60.0)
        fcntl.fcntl(self.socket, fcntl.F_SETFL, os.O_NONBLOCK)
        self.shutdown = Event()
        self.persist = persist
        self.family_addr = family_addr

    def __enter__(self):
        try:
            self.socket.bind(('', self.port))
        except socket.error as e:
            print('Bind failed:{}'.format(e))
            raise
        self.socket.listen(1)

        print('Starting server on port={} family_addr={}'.format(self.port, self.family_addr))
        self.server_thread = Thread(target=self.run_server)
        self.server_thread.start()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.persist:
            sock = socket.socket(self.family_addr, socket.SOCK_STREAM)
            sock.connect(('localhost', self.port))
            sock.sendall(b'Stop', )
            sock.close()
            self.shutdown.set()
        self.shutdown.set()
        self.server_thread.join()
        self.socket.close()

    def run_server(self):
        while not self.shutdown.is_set():
            try:
                conn, address = self.socket.accept()  # accept new connection
                print('Connection from: {}'.format(address))
                while 1:
                    mode = randint(0, 2)
                    print('Sending: ' + str(mode))
                    conn.send(str(mode).encode())
                    data = conn.recv(1024, )
                    if data:
                        print('Received' + str(data))
                    time.sleep(5)
                conn.close()
            except socket.error as e:
                print('Running server failed:{}'.format(e))
                raise
            if not self.persist:
                break


if __name__ == '__main__':
    if sys.argv[1:] and sys.argv[1].startswith('IPv'):     # if additional arguments provided:
        # Usage: example_test.py <IPv4|IPv6>
        family_addr = socket.AF_INET6 if sys.argv[1] == 'IPv6' else socket.AF_INET
        with TcpServer(PORT, family_addr, persist=True) as s:
            print(input('Press Enter stop the server...'))
