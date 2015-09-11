/**
 * Created by bruce on 15/9/6.
 */


var net = require('net');

//var HOST = '222.92.133.154';
var HOST = '127.0.0.1';
var PORT = 8081;

var client = new net.Socket();
client.connect(PORT, HOST, function() {
    console.log('Connect to: ' + HOST + ':' + PORT);

    var i = 0;
    // 33 bytes
    var buf = new Buffer(33);

    // header
    buf[i++] = 0x02;
    buf[i++] = 0x55;
    buf[i++] = 0xAA;
    buf[i++] = 0x00;
    buf[i++] = 0x21;

    // code type
    buf[i++] = 0x40;

    // imei 8 bytes
    buf[i++] = 0x35;
    buf[i++] = 0x21;
    buf[i++] = 0x51;
    buf[i++] = 0x02;
    buf[i++] = 0x20;
    buf[i++] = 0x01;
    buf[i++] = 0x85;
    buf[i++] = 0x80;

    // imsi 8 bytes
    buf[i++] = 0x46;
    buf[i++] = 0x69;
    buf[i++] = 0x23;
    buf[i++] = 0x30;
    buf[i++] = 0x33;
    buf[i++] = 0x11;
    buf[i++] = 0x60;
    buf[i++] = 0x10;

    // end
    buf[i++] = 0xAA;
    buf[i++] = 0x55;
    buf[i++] = 0x03;

    console.log("SEND>>>>>>>>>>>>>>>>>");
    console.log(buf);

    client.write(buf);
});

client.on('data', function(data) {
    console.log("RECV<<<<<<<<<<<<<<<<<");
    console.log(data);
});

client.on('error', function() {
    console.log('Connect error');
});

client.on('close', function() {
    console.log('Connection closed');
})
