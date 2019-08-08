var mqtt = require('mqtt');

var options = {
    port: 13190,
    host: 'mmqtt://m15.cloudmqtt.com',
    clientId: 'AC326' + Math.random().toString(16).substr(2, 8),
    username: 'wkeuusha',
    password: 'OEPpT3Jp1AG3',
    keepalive: 60,
    reconnectPeriod: 1000,
    protocolId: 'MQIsdp',
    protocolVersion: 3,
    clean: true,
    encoding: 'utf8'
};

var client = mqtt.connect('mqtt://m15.cloudmqtt.com', options);
client.on('connect', function () {
    console.log('connected');
    client.subscribe('temperatura', function () {
        client.on('message', function (topic, message, packet) {
            console.log("Received '" + message + "' on '" + topic + "'");
            temperatura = message;
        });
    });
});

var buscarTemperatura = function (req, res) {
    res.send(temperatura);
}

var configVentoinha = function (req, res) {
    console.log(req.body);
    client.publish('ventoinha', JSON.stringify(req.body), function(resultado) {
        console.log("Message is published");
        res.status(201).json(resultado);
    });
}

exports.buscarTemperatura = buscarTemperatura;
exports.configVentoinha = configVentoinha;