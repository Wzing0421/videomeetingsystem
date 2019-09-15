# -*- coding: utf-8 -*-
from flask import Flask, request, render_template, jsonify

# import
import json
import os
import time
from socket import *
app = Flask(__name__)


@app.route('/')
def index():
    return render_template('json2.html')


def killpro():
    print('kill UDPRecv、UDPVideo、ch_encoder mychdet')
    os.system('killall myrecv myvideo ch_encoder mychdet')


def startpro():
    print('start UDPRecv、UDPVideo  ch_encoder')
    os.system('./../ChannelDetection/mychdet > /dev/null &')
    time.sleep(0.1)
    os.system('./../UDPRecv/myrecv > /dev/null &')
    time.sleep(0.1)
    os.system('./../UDPVideo/myvideo > /dev/null &')
    time.sleep(0.1)
    os.system('./../check_encoder/ch_encoder')


@app.route('/sendAjax2', methods=['POST', 'GET'])
def sendAjax2():
    killpro()
    print('config.....')
    data = json.loads(request.form.get('data'))
    data['encode_id'] = (int)(data['encode_id'])
    # print(data)
    with open("config.json", "w") as f:
        json.dump(data, f)
        # print(type(data))
    outside_ip = data['IP']
    setip_str = '#!/bin/bash' + '\n' + 'sudo ifconfig eth0:1' + ' ' + outside_ip + \
        '  ' + 'netmask 255.255.255.0' + '\n'
    commd = 'sudo ifconfig eth0:1' + '  ' + outside_ip + \
        '  ' + 'netmask 255.255.255.0'
    setip = open('setip', 'w+')
    setip.write(setip_str)
    setip.close()
    print(commd)
    os.system(commd)
    # print(data['IP'])
    # print(type(data['encode_id']))
    with open("id2ip_de.json", "r") as f:
        id2ip = json.load(f)
        ch2ip = {

            '1': id2ip[data['decode1_id']]['decode_ip'],
            '2': id2ip[data['decode2_id']]['decode_ip'],
            '3': id2ip[data['decode3_id']]['decode_ip'],
            '4': id2ip[data['decode4_id']]['decode_ip'],
            '5': id2ip[data['decode5_id']]['decode_ip'],
            '6': id2ip[data['decode6_id']]['decode_ip'],
            '7': id2ip[data['decode7_id']]['decode_ip'],
            '8': id2ip[data['decode8_id']]['decode_ip'],
            '9': id2ip[data['decode9_id']]['decode_ip'],
            '10': id2ip[data['decode10_id']]['decode_ip']
        }
        with open("ch2ip.json", "w") as f1:
            json.dump(ch2ip, f1)
            print(ch2ip)
            ch2ip_str = json.dumps(ch2ip)
            print(ch2ip_str)
            HOST = '127.0.0.1'
            PORT = 9212
            ADDR = (HOST, PORT)
            udp_signal_sock = socket(AF_INET, SOCK_DGRAM)
            udp_signal_sock.sendto(ch2ip_str.encode("utf-8"), ADDR)
            print("send sucessful")
            udp_signal_sock.close()
    print("config successful!")
    # startpro()
    return jsonify(data)


@app.route('/start', methods=['POST', 'GET'])
def start():
    #print("start video")
    startpro()
    return('successful')


@app.route('/sendjson', methods=['POST','GET'])
def sendjson():
    with open("config.json", "r") as f:
        data = json.load(f)
        # print(data)
        data = json.dumps(data)
        # print(data)
        print("sendjson sucessful")
        return data


def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,
        struct.pack('256s', ifname[:15].encode('utf-8'))
    )[20:24])


if __name__ == '__main__':

    # app.debug=True

    app.run(host='0.0.0.0', port=8000)
