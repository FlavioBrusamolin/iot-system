var express = require('express');
var cors = require('cors');
var app = express();
var bodyParser = require('body-parser');

app.use(bodyParser.json());
app.use(cors());

var mqttHomeCtrl = require('./mqtt/mqttHome');

app.get('/mqtt/home', mqttHomeCtrl.buscarTemperatura);
app.post('/mqtt/home/config', mqttHomeCtrl.configVentoinha);

app.listen(9000, function() {
    console.log('EC208 API');
});