
var BasicPacket = Backbone.Model.extend({
    initialize: function (options) {
        this.apperanceTime = options.apperanceTime;
        this.length = options.length;
    },
});


var QueuedBasicPacked = Backbone.Model.extend({
    defaults: {
        alreadyTakenDataLength: 0
    },

    initialize: function (basicPacket) {
        this.basicPacket = basicPacket;
    },

    getReamingDataToSend: function () {
        return this.basicPacket.length - this.get('alreadyTakenDataLength');
    },

    getData: function (amount) {
        this.set('alreadyTakenDataLength', this.get('alreadyTakenDataLength') + amount);
    },
});


var BasicPacketsQueue = Backbone.Collection.extend({
    getAviableDataLength: function () {
        function sumPacketsReamingData(acc, packet) {
            return acc + packet.getReamingDataToSend();
        }

        return this.reduce(sumPacketsReamingData, 0);
    },

});


var HiddenPacket = Backbone.Model.extend({
    initialie: function (options) {
        this.apperanceTime = options.apperanceTime;
        this.segments = options.segments;
    },
});


var HiddenPacketsQueue = Backbone.Collection.extend({
    initialize: function(options) {

    },


});
