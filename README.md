# NextionEsp8266Mqtt
Control your Nextion display using a esp8266 and mqtt

This sketch uses a esp8266 and mqtt to control a Nextion Display.

The sketch publishers every event that is fired from the display to a mqtt channel
which is defined under the root_topicOut

It allows for setting text and sending commands to the display through the root_topicIn
subscribed mqtt channel.

The two topics that have been exposed for control are "setComponentText" and "sendCommand".
eg: to set text for a txt control on the display just send the following payload
  {
"topic":"setComponentText",
"payload":{
  "component":"page0.t2",
   "text":"hello"
 }
}

to Set a background picture send
 {
"topic":sendCommand,
"payload":{
  "command":"page0.b0.picc=1"
 }
}

You can also use Node-red to set different texts and commands.
example:  

[{"id":"a909d4ea.be9478","type":"mqtt in","z":"a1a84f55.f6dbe","name":"","topic":"Nextion/Out","qos":"2","broker":"d4be06bc.5dd008","x":153,"y":204,"wires":[["6aa28447.9e3eec","51353c01.ca0d54"]]},{"id":"b8f0d5ef.fc4768","type":"function","z":"a1a84f55.f6dbe","name":"","func":"var settemp = flow.get(\"settemp\");\nnode.warn(settemp);\nsettemp = settemp + 0.5;\nif (settemp >= 30)\n{\n    settemp=30;\n}\nflow.set(\"settemp\",settemp);\n\nmsg.payload=settemp.toFixed(1) ;\nmsg.topic=\"setComponentText\"\nreturn msg;","outputs":1,"noerr":0,"x":543,"y":177,"wires":[["a09e256d.815e18","e403a4cb.763e18"]]},{"id":"51353c01.ca0d54","type":"switch","z":"a1a84f55.f6dbe","name":"","property":"payload","propertyType":"msg","rules":[{"t":"eq","v":"65110ff","vt":"str"},{"t":"eq","v":"65120ff","vt":"str"}],"checkall":"true","outputs":2,"x":348,"y":202,"wires":[["b8f0d5ef.fc4768"],["b00ca382.40cb4"]]},{"id":"e403a4cb.763e18","type":"template","z":"a1a84f55.f6dbe","name":"Command Template","field":"payload","fieldType":"msg","format":"handlebars","syntax":"mustache","template":"{\n\"topic\":{{topic}},\n\"payload\":{\n  \"component\":\"Thermostat.t2\",\n   \"text\":\"{{payload}}\"\n }\n}","x":837,"y":205,"wires":[["4ec90d5f.6956a4"]]},{"id":"4ec90d5f.6956a4","type":"mqtt out","z":"a1a84f55.f6dbe","name":"","topic":"Nextion/In","qos":"","retain":"","broker":"d4be06bc.5dd008","x":1165,"y":155,"wires":[]},{"id":"b00ca382.40cb4","type":"function","z":"a1a84f55.f6dbe","name":"","func":"var settemp = flow.get(\"settemp\");\nnode.warn(settemp);\nsettemp = settemp - 0.5;\nif (settemp <= 10)\n{\n    settemp=10;\n}\nflow.set(\"settemp\",settemp);\n\nmsg.payload=settemp.toFixed(1) ;\nmsg.topic=\"setComponentText\"\nreturn msg;","outputs":1,"noerr":0,"x":538,"y":242,"wires":[["e403a4cb.763e18"]]},{"id":"d4be06bc.5dd008","type":"mqtt-broker","z":"","broker":"192.168.2.231","port":"1883","clientid":"","usetls":false,"compatmode":true,"keepalive":"60","cleansession":true,"willTopic":"","willQos":"0","willPayload":"","birthTopic":"","birthQos":"0","birthPayload":""}]
