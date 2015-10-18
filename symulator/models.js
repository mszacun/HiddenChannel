var Message = Backbone.Model.extend({
    initialize: function(options) {

    },

    calculateDelay: function (receiveTime) {
        this.receiveTime = receiveTime;
        this.delay = receiveTime - this.apperanceTime;
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

});


var HiddenMessage = Message.extend({
    initialize: function (options) {
        this.apperanceTime = options.apperanceTime;
        this.segments = options.segments;
    },
});


var HiddenMessagesQueue = Backbone.Collection.extend({
    initialize: function(options) {

    },


});
