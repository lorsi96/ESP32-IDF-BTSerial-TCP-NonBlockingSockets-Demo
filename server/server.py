'''MIT License

Copyright (c) 2021 Lucas Orsi (lorsi 96) 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''

import os
import re
import socket
import time
import sys
from random import randint
import fcntl
from builtins import input
from threading import Event, Thread

PORT = 3333
MENU_STR = '''
-------------------------------------------------------
Choose one of the following options and press [Enter]
(0) Query Blink Speed.
(1) Query Server status.
(2) Toggle Bluetooth Server on/off. 
(9) Exit.
-------------------------------------------------------

'''

CMD_SENT_MESSAGES = {
    '0': 'Asking ESP32 for its current blink speed...',
    '1': 'Asking ESP32 for its current BT server status...',
    '2': 'Asked ESP32 to toggle its BT service...'
}

BLINK_SPEED_DECODER = {
    '0': 'ESP32 is Blinking Slowly',
    '1': 'ESP32 is Blinking Fast',
    '2': 'ESP32 is Not Blinking'
}

SERVER_STATUS_DECODER = {
    '0': 'ESP32 is Listening to BT events',
    '1': 'ESP32 is not Listening to BT events'
}

SERVER_TOGGLE_DECODER = {
    '0': 'ESP32 turned Off its BT server',
    '1': 'ESP32 turned On its BT server'
}

DECODERS = {
    '0': BLINK_SPEED_DECODER,
    '1': SERVER_STATUS_DECODER,
    '2': SERVER_TOGGLE_DECODER
}


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
                    print(MENU_STR)
                    choice = input()
                    if choice == '9':
                        conn.close()
                        exit(0)
                    print(CMD_SENT_MESSAGES[choice])
                    conn.send(str(choice).encode())
                    data = conn.recv(1024, )
                    if data:
                        print(DECODERS[choice][str(data)[2]]) # Data comes in "b'n\0" format, where n is the interesting part.    
                    time.sleep(5)
                conn.close()
            except socket.error as e:
                print('Running server failed:{}'.format(e))
                raise
            if not self.persist:
                break


if __name__ == '__main__':
    family_addr =  socket.AF_INET
    with TcpServer(PORT, family_addr, persist=True) as s:
        while True:
            time.sleep(60) # Yup, after exit you'll have to Ctrl+C anyway!
        
