angular.module('ec208').factory('mqtt', function ($http) {

    var buscarTemperatura = function () {
        return $http.get('http://localhost:9000/mqtt/home');
    };

    var configVentoinha = function (ventoinha) {
        var configs = { ventoinha: ventoinha };
        return $http.post('http://localhost:9000/mqtt/home/config', configs);
    };

    return {
        buscarTemperatura: buscarTemperatura,
        configVentoinha: configVentoinha
    }

});