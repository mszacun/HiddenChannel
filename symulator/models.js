var Message = Backbone.Model.extend({
    initialize: function(options) {

    },

    calculateDelay: function (receiveTime) {
        this.receiveTime = receiveTime;
        this.delay = receiveTime - this.apperanceTime;
    },

    isReceived: function () {
        return this.delay !== undefined;
    },
});
var BasicMessage = Message.extend({
    initialize: function (options) {
        this.apperanceTime = options.apperanceTime;
        this.length = options.length;
    },
});


var QueuedBasicPacked = Backbone.Model.extend({
    defaults: {
        alreadyTakenDataLength: 0
    },

    initialize: function (basicMessage) {
        this.basicMessage = basicMessage;
    },

    getReamingDataToSend: function () {
        return this.basicMessage.length - this.get('alreadyTakenDataLength');
    },

    getData: function (amount) {
        this.set('alreadyTakenDataLength', this.get('alreadyTakenDataLength') + amount);
    },
});


var BasicMessagesQueue = Backbone.Collection.extend({
    getAviableDataLength: function () {
        function sumMessagesReamingData(acc, packet) {
            return acc + packet.getReamingDataToSend();
        }

        return this.reduce(sumMessagesReamingData, 0);
    },

    getData: function (amount) {
        var dataSources = [];

        while (amount > 0) {
            var basicPacketUsed = this.at(0);
            var aviableDataFromPacket = basicPacketUsed.getReamingDataToSend();

            if (amount >= aviableDataFromPacket) {
                this.shift();
                dataSources.push({
                    packet: basicPacketUsed,
                    length: aviableDataFromPacket,
                    hasMoreFragments: false
                });
                amount -= aviableDataFromPacket;
            }
            else {
                dataSources.push({
                    packet: basicPacketUsed,
                    length: amount,
                    hasMoreFragments: true
                });
                basicPacketUsed.getData(amount);
                amount = 0;
            }
        }
        return dataSources;
    },

});


var HiddenMessage = Message.extend({
    initialize: function (options) {
        this.apperanceTime = options.apperanceTime;
        this.segments = options.segments;
        this.sentSegments = 0;
    },

    getNextSegmentToSend: function () {
        return this.segments[this.sentSegments++];
    },

    hasMoreSegmentsToSend: function () {
        return this.sentSegments < this.segments.length;
    },

    peekNextSegmentToSend: function () {
        return this.segments[this.sentSegments];
    },
});


var HiddenMessagesQueue = Backbone.Collection.extend({
    getSegmentToSend: function () {
        var packet = this.at(0);
        var returnedSegment = packet.getNextSegmentToSend();
        if (!packet.hasMoreSegmentsToSend())
            this.shift();

        return {
            hiddenPacket: packet,
            segment: returnedSegment,
            hasMoreFragments: packet.hasMoreSegmentsToSend()
        };
    },

    peekSegmentToSend: function () {
        return this.at(0).peekNextSegmentToSend();
    },
});
