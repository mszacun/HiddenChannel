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

function Symulator(options) {
    this.basicMessages = [
        new BasicMessage({ id: 'p1', apperanceTime: 1, length: 300 }),
        new BasicMessage({ id: 'p2', apperanceTime: 7, length: 310 }),
        new BasicMessage({ id: 'p3', apperanceTime: 13, length: 500 })
    ];
    this.hiddenMessages = [
        new HiddenMessage({ id: 'u1', apperanceTime: 5, segments: [ 128 ]}),
        new HiddenMessage({ id: 'u2', apperanceTime: 6, segments: [ 221 ]})
    ];

    this.allBasicMessages = this.basicMessages.slice(0);
    this.allHiddenMessages = this.hiddenMessages.slice(0);

    this.symulationTime = options.symulationTime;
    this.hiddenChannel = new HiddenChannel();
    this.channel = new Channel(1000);
    this.currentTime = 0;
}

Symulator.prototype.step = function() {
    var currentTime = this.currentTime;

    if (currentTime < this.symulationTime) {
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
            packetsGenerated: packetsGenerated,
            packetsThatReachedTarget: packetsThatReachedTarget
        };
    }
    else
        return [];
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
    initialize: function(options) {
    },

    getFullSize: function () {
        return this.get('hiddenPacket').get('segments')[0] + HEADER_SIZE;
    },

    calculateDelay: function (receiveTime) {
        _.each(this.get('dataSources'), function(dataSource) {
            if (!dataSource.hasMoreFragments)
                dataSource.packet.calculateDelay(receiveTime);
        });

        this.get('hiddenPacket').calculateDelay(receiveTime);
    },

});


var PacketWithoutHiddenData = Backbone.Model.extend({
    initialize: function(options) {
        this.set('dataLength', options.basicPacket.getReamingDataToSend());
        this.set('basicMessage', options.basicPacket.basicMessage);
        this.set('padding', Math.max(0, MIN_DATA_PACKET_SIZE - this.get('dataLength')));
    },

    getFullSize: function () {
        return HEADER_SIZE + this.get('dataLength') + this.get('padding');
    },

    calculateDelay: function (receiveTime) {
        this.get('basicMessage').calculateDelay(receiveTime);
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
        var packetToSend = this.hiddenMessagesQueue.shift();
        var neededData = packetToSend.get('segments')[0];
        var basicPacketUsed = this.basicMessagesQueue.shift();

        basicPacketUsed.getData(neededData);

        sentPackets.push(new PacketWithHiddenData({
            hiddenPacket: packetToSend,
            dataSources: [{packet: basicPacketUsed, length: neededData, hasMoreFragments: true}]
        }));
        sentPackets.push(new PacketWithoutHiddenData({basicPacket: basicPacketUsed}));
    }

    while (!this.basicMessagesQueue.isEmpty()) {
        var packetToSend = this.basicMessagesQueue.shift();

        sentPackets.push(new PacketWithoutHiddenData({basicPacket: packetToSend}));
    }

    return sentPackets;
}

HiddenChannel.prototype.canSendHiddenPacket = function() {
    return !this.hiddenMessagesQueue.isEmpty() &&
        this.basicMessagesQueue.getAviableDataLength() >= this.hiddenMessagesQueue.at(0).get('segments')[0];
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
