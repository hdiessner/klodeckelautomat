import Queue
import serial
import time
import threading
from   flask import Flask
from   flask import render_template

app       = Flask(__name__)
sendQueue = Queue.Queue()

@app.route('/', methods=['GET', 'POST'])
def index():
    return render_template('index.html', servos=servoIsOpen)


@app.route('/move/<servo>/<position>')
def move(servo, position):

    if servo is not None and int(servo) > 0 and int(servo) < 5 and position in ['open', 'close']:

        if position == 'open':
            if int(servo) == 1:
                servoIsOpen[int(servo)-1] = True
        else:
            if int(servo) == 1:
                sendQueue.put("3,%s;" % config.get("SERVO1", "min"))
                status.servoIsOpen[int(servo) - 1] = False
        return render_template('index.html', servos=servoIsOpen)

    else:
        return "Error, wrong parameters. Try /move/1/open"


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
            element = line.split(',')
            command = int(element[0])
            if command == 0:
                print "Ack Comment:%s" % line
            if command == 1:
                print "Error:%s" % line
            if command == 2:
                print "Command 2:%s" % line
            if command == 3:
                print "Command 3:%s" % line
            if command == 4:
                print "Command 4:%s" % line
            if command == 5:
                print "Command 5:%s" % line
            if command == 6:
                print "Command 6:%s" % line
            if command == 7:
                print "Command 7:%s" % line
            if command == 8:
                print "Command 8:%s" % line


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
            self._queue.task_done()
            time.sleep(0.5) # Give him time to react

if __name__ == "__main__":
    print "Start"

    ser         =  serial.Serial("COM3", 9600)
    servoIsOpen = [True] * 8
    receive     = ReceiveThread(ser, status)
    send        = SendThread(sendQueue, ser, status)
    receive.start()
    send.start()

    time.sleep(2)
    sendQueue.put('3,%s;' % config.get("SERVO1", "min"))
    time.sleep(2)
    sendQueue.put('3,%s;' % config.get("SERVO1", "max"))
    time.sleep(2)
    sendQueue.put('4,%s;' % config.get("SERVO2", "min"))
    time.sleep(2)
    sendQueue.put('4,%s;' % config.get("SERVO2", "max"))
    time.sleep(2)
    sendQueue.put('5,%s;' % config.get("SERVO3", "min"))
    time.sleep(2)
    sendQueue.put('5,%s;' % config.get("SERVO3", "max"))
    time.sleep(2)
    sendQueue.put('6,%s;' % config.get("SERVO4", "min"))
    time.sleep(2)
    sendQueue.put('6,%s;' % config.get("SERVO4", "max"))
    time.sleep(2)
    app.run(host='0.0.0.0', port=8080)
