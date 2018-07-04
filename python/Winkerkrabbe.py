import Queue
import serial
import time
import threading
from   flask import Flask
from   flask import render_template
from   flask import request

app       = Flask(__name__)
sendQueue = Queue.Queue()

class Status:

    def __init__(self):
        self.servoPosition  = [300] * 8
        self.telescopeState = [  3] * 3 


@app.route('/', methods=['GET', 'POST'])
def index():

    if request.method == 'POST':
        command = request.form.get('command').split("-")
        c =  "6,%s,%s;" % (command[0], command[1])
        sendQueue.put(str(c))
        print("post")
        return render_template('index.html', reload=True, servoPosition=status.servoPosition, telescopeState=status.telescopeState)     

    print("normal")
    return render_template('index.html', reload=False, servoPosition=status.servoPosition, telescopeState=status.telescopeState)


@app.route('/move', methods=['POST'])
def move():

    if request.method == "POST":
        print("POST")
        position = request.form.get('position')
        plate    = request.form.get('plate')
        command  = request.form.get('command')

        if command == "move":
            sendQueue.put(str("3,%s,%s;" % (plate, position)))
        
        if command == "open":
            sendQueue.put(str("4,%s,1;" % (plate)))

        if command == "close":
            sendQueue.put(str("4,%s,0;" % (plate)))

    return render_template('index.html', reload=False, servoPosition=status.servoPosition, telescopeState=status.telescopeState)

class ReceiveThread(threading.Thread):

    def __init__(self, ser, status):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self._ser      = ser
        self._status   = status

    def run(self):
        line = self._ser.readline() # To clear buffer
        while True:
            line    = self._ser.readline()
            clean   = line.split(";")[0]
            element = clean.split(',')
            command = int(element[0])
            if command == 0:
                print "Ack Comment:%s" % line
            if command == 1:
                print "Error:%s" % line
            if command == 2:
                pass
                print "Debug:%s" % element[1:]
            if command == 5:
                telescope = int(element[1])
                state     = int(element[2])
                status.telescopeState[telescope] = state


class SendThread(threading.Thread):

    def __init__(self, queue, ser, status):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self._queue  = queue
        self._ser    = ser
        self._status = status

    def run(self):
        while True:
            line = self._queue.get()
            #print ">>>>>>>>>>>" + line
            self._ser.write(line + "\n")
            self._ser.flush()
            self._queue.task_done()

if __name__ == "__main__":
    print "Start"
    status = Status()
    ser         =  serial.Serial("COM3", 9600)
    servoIsOpen = [True] * 8
    receive     = ReceiveThread(ser, status)
    send        = SendThread(sendQueue, ser, status)
    receive.start()
    send.start()
    sendQueue.put("2,0,0;") # to sync communication
    app.run(host='0.0.0.0', port=8080)
