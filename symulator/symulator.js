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
    this.basicPackets = [
        new BasicPacket({ id: 'p1', apperanceTime: 1, length: 300 }),
        new BasicPacket({ id: 'p2', apperanceTime: 7, length: 310 }),
        new BasicPacket({ id: 'p3', apperanceTime: 13, length: 500 })
    ];
    this.hiddenPackets = [
        new HiddenPacket({ id: 'u1', apperanceTime: 5, segments: [ 128 ]}),
        new HiddenPacket({ id: 'u2', apperanceTime: 6, segments: [ 221 ]})
    ];

    this.allBasicPackets = this.basicPackets.slice(0);
    this.allHiddenPackets = this.hiddenPackets.slice(0);

    this.symulationTime = options.symulationTime;
    this.hiddenChannel = new HiddenChannel();
    this.channel = new Channel(1000);
    this.currentTime = 0;
}

Symulator.prototype.step = function() {
    var currentTime = this.currentTime;

    if (currentTime < this.symulationTime) {
        function hasPacketAppeared(packet) { return packet.get('apperanceTime') <= currentTime; }

        var hiddenPacketsArrived = _.filter(this.hiddenPackets, hasPacketAppeared);
        var basicPacketsArrived = _.filter(this.basicPackets, hasPacketAppeared);

        this.hiddenPackets= _.reject(this.hiddenPackets, hasPacketAppeared);
        this.basicPackets = _.reject(this.basicPackets, hasPacketAppeared);

        _.each(hiddenPacketsArrived, function(packet) {
            this.hiddenChannel.addHiddenPacket(packet);
        }, this);
        _.each(basicPacketsArrived, function(packet) {
            this.hiddenChannel.addBasicPacket(packet);
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
        this.set('basicPacket', options.basicPacket.basicPacket);
        this.set('padding', Math.max(0, MIN_DATA_PACKET_SIZE - this.get('dataLength')));
    },

    getFullSize: function () {
        return HEADER_SIZE + this.get('dataLength') + this.get('padding');
    },

    calculateDelay: function (receiveTime) {
        this.get('basicPacket').calculateDelay(receiveTime);
    },
});


function HiddenChannel() {
    this.hiddenPacketsQueue = new HiddenPacketsQueue();
    this.basicPacketsQueue = new BasicPacketsQueue();
}

HiddenChannel.prototype.addBasicPacket = function(packet) {
    this.basicPacketsQueue.add(new QueuedBasicPacked(packet));
}

HiddenChannel.prototype.addHiddenPacket = function(packet) {
    this.hiddenPacketsQueue.add(packet);
}

HiddenChannel.prototype.execute = function() {
    var sentPackets = [];

    // sending hidden packets
    while (this.canSendHiddenPacket()) {
        var packetToSend = this.hiddenPacketsQueue.shift();
        var neededData = packetToSend.get('segments')[0];
        var basicPacketUsed = this.basicPacketsQueue.shift();

        basicPacketUsed.getData(neededData);

        sentPackets.push(new PacketWithHiddenData({
            hiddenPacket: packetToSend,
            dataSources: [{packet: basicPacketUsed, length: neededData, hasMoreFragments: true}]
        }));
        sentPackets.push(new PacketWithoutHiddenData({basicPacket: basicPacketUsed}));
    }

    while (!this.basicPacketsQueue.isEmpty()) {
        var packetToSend = this.basicPacketsQueue.shift();

        sentPackets.push(new PacketWithoutHiddenData({basicPacket: packetToSend}));
    }

    return sentPackets;
}

HiddenChannel.prototype.canSendHiddenPacket = function() {
    return !this.hiddenPacketsQueue.isEmpty() &&
        this.basicPacketsQueue.getAviableDataLength() >= this.hiddenPacketsQueue.at(0).get('segments')[0];
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
