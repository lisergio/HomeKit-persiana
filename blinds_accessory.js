var Blind1Position=0;
var Blind1Target=0;;
var Dato=0;
// MQTT Setup
var mqtt = require('mqtt');
console.log("Connecting to MQTT broker...");
var mqtt = require('mqtt');
var options = {
port: 1883,
host: '192.168.0.56',
clientId: 'blind1'
};
var client = mqtt.connect(options);
console.log("Blind 1 Connected to MQTT broker");
client.subscribe('blind1posicion');
client.on('message', function(topic, message) {
    console.log(parseInt(message));
    Dato=parseInt(message);

if (Dato>=200){
Blind1Target=(Dato-200);
updateTarget(Blind1Target);
}
else{
   Blind1Position=Dato;
   updateBlind(Blind1Position);
}

});

var Accessory = require('../').Accessory;
var Service = require('../').Service;
var Characteristic = require('../').Characteristic;
var uuid = require('../').uuid;
var exec = require('child_process').exec;
var ServiceName="Persiana1";

var sensorUUID = uuid.generate('hap-nodejs:accessories:move1');
var blind1 = exports.accessory = new Accessory('Persiana 1', sensorUUID);
blind1.username = "69:34:CC:5A:13:2B";
blind1.pincode = "031-45-154";

blind1.getService(Service.AccessoryInformation)
.setCharacteristic(Characteristic.Manufacturer, "Lisergio")
.setCharacteristic(Characteristic.Model, "Blinds")
.setCharacteristic(Characteristic.SerialNumber, "00001");

blind1.on('identify', function(paired, callback) {
callback();
});

blind1.addService(Service.WindowCovering,ServiceName)
    .getCharacteristic(Characteristic.CurrentPosition)
    .on('get', function(callback) {
        //iOS will want to know whats the current Blind1Position when it opens the HomeKit App 
        //So here we send it to iOS
        callback(null, Blind1Position)
    })

var updateBlind = function(toPosition) {
    //Update the value to the iOS when the client tells us it's changing
    blind1.getService(Service.WindowCovering)
        .getCharacteristic(Characteristic.CurrentPosition)
        .updateValue(toPosition);
}

var updateTarget = function(toPosition) {
    //Update the value to the iOS when the client tells us it's changing
    blind1.getService(Service.WindowCovering)
        .getCharacteristic(Characteristic.TargetPosition)
        .updateValue(toPosition);
}


blind1.getService(Service.WindowCovering)
    .getCharacteristic(Characteristic.TargetPosition)
    .on('get', function(callback) {
        //Tell the iOS whats the current target initaly when you open the HomeKit App
        callback(null, Blind1Target);
    })
    .on('set',function(val, callback){
        //iOS tells the client that target needs to be changed
        client.publish('blind1',String(val));
        Blind1Target=val;
        callback(null);
    });


