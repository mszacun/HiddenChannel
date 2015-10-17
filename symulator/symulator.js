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
        new BasicPacket({ apperanceTime: 1, length: 300 }),
        new BasicPacket({ apperanceTime: 7, length: 310 })
    ];
    this.hiddenPackets = [
        new HiddenPacket({ apperanceTime: 5, segments: [ 128 ]})
    ];

    this.symulationTime = options.symulationTime;
    this.hiddenChannel = new HiddenChannel();
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

        this.currentTime += 1;

        return this.hiddenChannel.execute();
    }
    else
        return [];
}


function PacketWithHiddenData(hiddenPacket, dataSources) {
    this.hiddenPacket = hiddenPacket;
    this.dataSources = dataSources;
}


var MIN_DATA_PACKET_SIZE = 256;

function PacketWithoutHiddenData(basicPacket) {
    this.basicPacket = basicPacket;
    this.dataLength = basicPacket.getReamingDataToSend();
    this.padding = Math.max(0, MIN_DATA_PACKET_SIZE - this.dataLength);
}


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
    var happenedEvents = [];

    // sending hidden packets
    while (this.canSendHiddenPacket()) {
        var packetToSend = this.hiddenPacketsQueue.pop();
        var neededData = packetToSend.get('segments')[0];

        var dataSources = this.basicPacketsQueue.getData(neededData);

        happenedEvents.push(new PacketWithHiddenData(packetToSend,dataSources));

    }

    while (!this.basicPacketsQueue.isEmpty()) {
        var packetToSend = this.basicPacketsQueue.pop();

        happenedEvents.push(new PacketWithoutHiddenData(packetToSend));
    }

    return happenedEvents;
}

HiddenChannel.prototype.canSendHiddenPacket = function() {
    if (!this.hiddenPacketsQueue.isEmpty())

    return !this.hiddenPacketsQueue.isEmpty() &&
        this.basicPacketsQueue.getAviableDataLength() >= this.hiddenPacketsQueue.at(0).get('segments')[0];
}
