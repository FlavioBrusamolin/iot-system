angular.module('ec208').controller('homeCtrl', function ($scope, mqtt) {

    var temperatura = function () {
        var promise = mqtt.buscarTemperatura();
        promise.then(function (response) {
            console.log(response.data);
            $scope.home = response.data;
            if(response.data > 40) {
                if(ativacao == 0)
                    window.setTimeout(function () {
                        alert('A temperatura do ambiente est\u00e1 muito elevada! Recomenda-se \u00e0 ativa\u00e7\u00e3o do sistema de ventila\u00e7\u00e3o.');
                    }, 1500);
                ativacao ++;
            }
        });
        promise.catch(function (err) {
            alert('Nao foi possivel buscar a temperatura.');
        });
    }
    
    $scope.$watch('power', function (value) {
        if (value == true) {
            $scope.statusVent = 'ON';
            var promise = mqtt.configVentoinha(1);
            promise.then(function (response) {
                console.log('Ventoinha ligada com sucesso');
            });
            promise.catch(function (err) {
                alert('Nao foi possivel ligar a ventoinha.');
            });
        }
        else if (value == false) {
            $scope.statusVent = 'OFF';
            var promise = mqtt.configVentoinha(0);
            promise.then(function (response) {
                console.log('Ventoinha desligada com sucesso');
                ativacao = 0;
            });
            promise.catch(function (err) {
                alert('Nao foi possivel desligar a ventoinha.');
            });
        }
    });

    $scope.statusVent = 'OFF';
    var ativacao = 0;
    temperatura();
    chamaTemp = setInterval(temperatura, 500);

});