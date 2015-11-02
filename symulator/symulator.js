// Generate Poisson distributed random numbers
function rpoisson(lambda) {
  if (lambda === undefined) {
    lambda = 1;
  }
  var l = Math.exp(-lambda),
    k = 0,
    p = 1.0;
  do {
    k++;
    p *= Math.random();
  } while (p > l);

  return k - 1;
}

function getRandomInt(min, max) {
  return Math.floor(Math.random() * (max - min + 1) + min);
}

function Symulator(options) {
    this.basicMessages = [
        new BasicMessage({ id: 'p1', apperanceTime: 1, length: 300 }),
        new BasicMessage({ id: 'p2', apperanceTime: 7, length: 310 }),
        new BasicMessage({ id: 'p3', apperanceTime: 9, length: 250 }),
        new BasicMessage({ id: 'p4', apperanceTime: 13, length: 500 })
    ];
    this.hiddenMessages = [
        new HiddenMessage({ id: 'u1', apperanceTime: 5, segments: [ 128, 230 ]}),
        new HiddenMessage({ id: 'u2', apperanceTime: 6, segments: [ 221 ]})
    ];


    this.allBasicMessages = this.basicMessages.slice(0);
    this.allHiddenMessages = this.hiddenMessages.slice(0);

    this.timeForGeneratingPackets = options.timeForGeneratingPackets;
    this.hiddenChannel = new HiddenChannel();
    this.channel = new Channel(1000);
    this.currentTime = 0;
}

Symulator.prototype.generateSymulationData = function() {
    var hiddenDataAppearance = 2;
    var hiddenDataContent = 255;
    var basicDataAppearance = 1;
    var basicDataLength = 300;
    var hiddenPacketNumber = 0;
    var basicPacketNumber = 0;

    this.basicMessages = [];
    this.hiddenMessages = [];

    for (var t = 0; t < this.timeForGeneratingPackets; t++) {
        var numberOfPackets = rpoisson(hiddenDataAppearance);
        for (var i = 0; i < numberOfPackets; i++) {
            var packetContent = getRandomInt(1, hiddenDataContent);
            var data = {
                id: 'u' + hiddenPacketNumber,
                apperanceTime: t,
                segments: [packetContent]
            };
            console.log(JSON.stringify(data));
            this.hiddenMessages.push(new HiddenMessage(data));
            hiddenPacketNumber++;
        }
    }

    for (var t = 0; t < this.timeForGeneratingPackets + 10; t++) {
        var numberOfPackets = rpoisson(basicDataAppearance);
        for (var i = 0; i < numberOfPackets; i++) {
            var packetLength = getRandomInt(1, basicDataLength);
            var data = {
                id: 'p' + basicPacketNumber,
                apperanceTime: t,
                length: packetLength
            }
            console.log(JSON.stringify(data));
            this.basicMessages.push(new BasicMessage(data));
            basicPacketNumber++;
        }
    }

    this.allBasicMessages = this.basicMessages.slice(0);
    this.allHiddenMessages = this.hiddenMessages.slice(0);
}

Symulator.prototype.step = function() {
    var currentTime = this.currentTime;

    function hasMessageAppeared(message) { return message.get('apperanceTime') <= currentTime; }

    var hiddenMessagesArrived = _.filter(this.hiddenMessages, hasMessageAppeared);
    var basicMessagesArrived = _.filter(this.basicMessages, hasMessageAppeared);

    this.hiddenMessages = _.reject(this.hiddenMessages, hasMessageAppeared);
    this.basicMessages = _.reject(this.basicMessages, hasMessageAppeared);

    _.each(hiddenMessagesArrived, function(message) {
        this.hiddenChannel.addHiddenMessage(message);
    }, this);
    _.each(basicMessagesArrived, function(packet) {
        this.hiddenChannel.addBasicMessage(packet);
    }, this);

    var packetsGenerated = this.hiddenChannel.execute();
    _.each(packetsGenerated, _.bind(this.channel.addPacket, this.channel));
    var packetsThatReachedTarget = this.channel.getPacketsReachTarget();

    this.calculateDelayForPackets(packetsThatReachedTarget);

    this.currentTime += 1;

    return {
        arrivedBasicMessages: basicMessagesArrived,
        arrivedHiddenMessages: hiddenMessagesArrived,
        packetsGenerated: packetsGenerated,
        packetsThatReachedTarget: packetsThatReachedTarget
    };
}

Symulator.prototype.calculateDelayForPackets = function(packets) {
    var currentTime = this.currentTime;

    _.each(packets, function(packet) {
        packet.calculateDelay(currentTime);
    });
}


var HEADER_SIZE = 30;
var MIN_DATA_PACKET_SIZE = 256;

var PacketWithHiddenData = Backbone.Model.extend({
    defaults: {
        hasHiddenData: true
    },

    initialize: function(options) {
    },

    getFullSize: function () {
        return this.get('segment') + HEADER_SIZE;
    },

    getRealDataSize: function() {
        return this.get('segment');
    },

    calculateDelay: function (receiveTime) {
        _.each(this.get('dataSources'), function(dataSource) {
            if (!dataSource.hasMoreFragments)
                dataSource.packet.basicMessage.calculateDelay(receiveTime);
        });

        if (!this.get('hasMoreFragments'))
            this.get('hiddenPacket').calculateDelay(receiveTime);
    },

    getSizeDescription: function () {
        return '[U] Dane: ' + this.get('segment') + ' naglowek: ' + HEADER_SIZE;
    },

});


var PacketWithoutHiddenData = Backbone.Model.extend({
    defaults: {
        hasHiddenData: false
    },

    initialize: function(options) {
        this.set('dataSources', options.dataSources);
        this.set('dataLength', this.getRealDataSize());
        this.set('padding', Math.max(0, MIN_DATA_PACKET_SIZE - this.get('dataLength')));
    },

    getFullSize: function () {
        return HEADER_SIZE + this.get('dataLength') + this.get('padding');
    },

    getRealDataSize: function() {
        function sumDataSourcesLenght(acc, dataSource) {
            return acc + dataSource.length;
        }

        return _.reduce(this.get('dataSources'), sumDataSourcesLenght, 0);
    },

    calculateDelay: function (receiveTime) {
        _.each(this.get('dataSources'), function(dataSource) {
            if (!dataSource.hasMoreFragments)
                dataSource.packet.basicMessage.calculateDelay(receiveTime);
        });
    },

    getSizeDescription: function () {
        return '[P] Dane: ' + this.get('dataLength') + ' naglowek: ' + HEADER_SIZE + ' dopelnienie: ' + this.get('padding');
    },
});


function HiddenChannel() {
    this.hiddenMessagesQueue = new HiddenMessagesQueue();
    this.basicMessagesQueue = new BasicMessagesQueue();
}

HiddenChannel.prototype.addBasicMessage = function(packet) {
    this.basicMessagesQueue.add(new QueuedBasicPacked(packet));
}

HiddenChannel.prototype.addHiddenMessage = function(packet) {
    this.hiddenMessagesQueue.add(packet);
}

HiddenChannel.prototype.execute = function() {
    var sentPackets = [];

    // sending hidden packets
    while (this.canSendHiddenPacket()) {
        var packetInfo = this.hiddenMessagesQueue.getSegmentToSend();
        var dataSources = this.basicMessagesQueue.getData(packetInfo.segment);

        sentPackets.push(new PacketWithHiddenData({
            hiddenPacket: packetInfo.hiddenPacket,
            hasMoreFragments: packetInfo.hasMoreFragments,
            segment: packetInfo.segment,
            dataSources: dataSources
        }));
    }

    while (!this.basicMessagesQueue.isEmpty()) {
        var dataAmount = this.basicMessagesQueue.at(0).getReamingDataToSend();
        var dataSources = this.basicMessagesQueue.getData(dataAmount);

        sentPackets.push(new PacketWithoutHiddenData({dataSources: dataSources}));
    }

    return sentPackets;
}

HiddenChannel.prototype.canSendHiddenPacket = function() {
    return !this.hiddenMessagesQueue.isEmpty() &&
        this.basicMessagesQueue.getAviableDataLength() >= this.hiddenMessagesQueue.peekSegmentToSend();
}


function Channel(bandwith) {
    this.bandwith = bandwith;
    this.packetsQueue = [];
}

Channel.prototype.addPacket = function(packet) {
    this.packetsQueue.push(packet);
}

Channel.prototype.getPacketsReachTarget = function() {
    var sendBytes = 0;
    var receivedPackets = [];

    while (sendBytes <= this.bandwith && this.packetsQueue.length > 0) {
        var packet = this.packetsQueue.shift();

        sendBytes += packet.getFullSize();
        receivedPackets.push(packet);
    }

    return receivedPackets;
}
