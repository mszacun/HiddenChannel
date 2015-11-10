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
    this.timeForGeneratingHiddenMessages = options.timeForGeneratingHiddenMessages || 0;
    this.numberOfHiddenMessagesToGenerate = options.numberOfHiddenMessagesToGenerate || 0;
    this.timeForGeneratingBasicMessages = options.timeForGeneratingBasicMessages;
    this.hiddenChannel = new HiddenChannel(options.hiddenDataSegmentLength);
    this.channel = new Channel(options.channelBandwith);
    this.currentTime = 0;

    this.hiddenDataAppearance = options.hiddenDataAppearance;
    this.hiddenDataSegmentLength = options.hiddenDataSegmentLength;
    this.basicDataAppearance = options.basicDataAppearance;
    this.basicDataLength = options.basicDataLength;
    this.hiddenMessageLength = options.hiddenMessageLength;
}

Symulator.prototype.generateSymulationData = function() {
    var hiddenPacketNumber = 0;
    var basicPacketNumber = 0;

    this.basicMessages = [];
    this.allBasicMessages = [];
    this.hiddenMessages = [];
    this.allHiddenMessages = [];

    var t = 0;
    while (t < this.timeForGeneratingHiddenMessages || this.allHiddenMessages.length < this.numberOfHiddenMessagesToGenerate)
        this.generateHiddenMessages(t++);

    for (var t = 0; t < this.timeForGeneratingBasicMessages; t++)
        this.generateBasicMessages(t);
}

Symulator.prototype.generateHiddenMessages = function(apperanceTime) {
    var numberOfPackets = rpoisson(this.hiddenDataAppearance);
    for (var i = 0; i < numberOfPackets; i++) {
        var segments = [];
        var numberOfSegments = Math.max(1, rpoisson(this.hiddenMessageLength));
        for (var j = 0; j < numberOfSegments; j++) {
            segments.push(getRandomInt(1, Math.pow(2, this.hiddenDataSegmentLength) - 1));
        }
        var data = {
            id: 'u' + this.allHiddenMessages.length,
            apperanceTime: apperanceTime,
            segments: segments
        };

        var message = new HiddenMessage(data);
        this.hiddenMessages.push(message);
        this.allHiddenMessages.push(message);
    }
}

Symulator.prototype.generateBasicMessages = function(apperanceTime) {
    var numberOfPackets = rpoisson(this.basicDataAppearance);
    for (var i = 0; i < numberOfPackets; i++) {
        var packetLength = getRandomInt(1, this.basicDataLength);
        var data = {
            id: 'p' + this.allBasicMessages.length,
            apperanceTime: apperanceTime,
            length: packetLength
        };

        var message = new BasicMessage(data);
        this.basicMessages.push(message);
        this.allBasicMessages.push(message);
    }
}

Symulator.prototype.hasAllMessageBeenReceived = function(messages) {
    return _.every(messages, function(m) { return m.isReceived(); });
}

Symulator.prototype.hasAllMessagesAppeared = function(messages) {
    var currentTime = this.currentTime;
    return _.every(messages, function(m) {
        return m.get('apperanceTime') < currentTime;
    });
}

Symulator.prototype.step = function() {
    this.addBasicMessagesToSymulationIfNeeded();

    var messagesArrived = this.addMessagesThatArrived();

    var packetsGenerated = this.hiddenChannel.execute();
    _.each(packetsGenerated, _.bind(this.channel.addPacket, this.channel));
    var packetsThatReachedTarget = this.channel.getPacketsReachTarget();

    this.calculateDelayForPackets(packetsThatReachedTarget);

    this.currentTime += 1;

    return {
        arrivedBasicMessages: messagesArrived.basic,
        arrivedHiddenMessages: messagesArrived.hidden,
        packetsGenerated: packetsGenerated,
        packetsThatReachedTarget: packetsThatReachedTarget
    };
}

Symulator.prototype.hasSymulationEnd = function() {
    return this.hasAllMessageBeenReceived(this.allHiddenMessages) &&
        this.hasAllMessageBeenReceived(this.allBasicMessages);
}

Symulator.prototype.addMessagesThatArrived = function() {
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

    return {
        basic: basicMessagesArrived,
        hidden: hiddenMessagesArrived
    };
}

Symulator.prototype.addBasicMessagesToSymulationIfNeeded = function() {
    if (this.needMoreDataForHiddenMessages() || this.needMoreDataForBasicMessages())
        this.generateBasicMessages(this.currentTime);
}

Symulator.prototype.needMoreDataForHiddenMessages = function() {
    return !this.hasAllMessageBeenReceived(this.allHiddenMessages) &&
        this.hasAllMessagesAppeared(this.allBasicMessages) &&
        !this.hiddenChannel.hasEnoughBasicData();
}

Symulator.prototype.needMoreDataForBasicMessages = function() {
    return !this.hasAllMessageBeenReceived(this.allBasicMessages) &&
        this.hasAllMessagesAppeared(this.allBasicMessages) &&
        this.hiddenChannel.basicMessagesQueue.getAviableDataLength() < Math.pow(2, this.hiddenDataSegmentLength);
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


function HiddenChannel(hiddenDataSegmentLength) {
    this.hiddenMessagesQueue = new HiddenMessagesQueue();
    this.basicMessagesQueue = new BasicMessagesQueue();

    this.hiddenDataSegmentLength = hiddenDataSegmentLength;
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

    var dataAviable = this.basicMessagesQueue.getAviableDataLength();
    if (dataAviable >= Math.pow(2, this.hiddenDataSegmentLength)) {
        var dataSources = this.basicMessagesQueue.getData(dataAviable);
        sentPackets.push(new PacketWithoutHiddenData({dataSources: dataSources}));
    }

    return sentPackets;
}

HiddenChannel.prototype.canSendHiddenPacket = function() {
    return !this.hiddenMessagesQueue.isEmpty() &&
        this.basicMessagesQueue.getAviableDataLength() >= this.hiddenMessagesQueue.peekSegmentToSend();
}

HiddenChannel.prototype.hasEnoughBasicData = function() {
    return this.hiddenMessagesQueue.getDataAmountNeeded() <=
        this.basicMessagesQueue.getAviableDataLength();
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
