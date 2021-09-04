import serial
from flask import Flask, Response, render_template

app = Flask(__name__)
arduino_serial = serial.Serial('/dev/ttyACM0', 115200)
arduino_serial.flush()

def event_stream():
    while True:
        if arduino_serial.in_waiting > 0:
            line = arduino_serial.readline().decode('utf-8').rstrip()
            yield "data: " + line + "\n\n"

@app.route('/stream')
def stream():
    return Response(event_stream(), mimetype="text/event-stream")

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    app.run(debug = True)
